#include "Hcal/HcalDoubleEndRecProducer.h"

namespace hcal {

void HcalDoubleEndRecProducer::configure(framework::config::Parameters& p) {
  pass_name_ = p.getParameter("pass_name", pass_name_);
  coll_name_ = p.getParameter("coll_name", coll_name_);

  rec_pass_name_ = p.getParameter("rec_pass_name", pass_name_);
  rec_coll_name_ = p.getParameter("rec_coll_name", coll_name_);

  pe_per_mip_ = p.get<double>("pe_per_mip");
  mip_energy_ = p.get<double>("mip_energy");
  clock_cycle_ = p.get<double>("clock_cycle");
}

void HcalDoubleEndRecProducer::produce(framework::Event& event) {
  const auto& hcal_geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  const auto& conditions{
      getCondition<HcalReconConditions>(HcalReconConditions::CONDITIONS_NAME)};

  auto hcal_rec_hits =
      event.getCollection<ldmx::HcalHit>(coll_name_, pass_name_);

  std::vector<ldmx::HcalHit> double_hcal_rec_hits;

  // group hcal rechits by the same HcalID
  std::map<ldmx::HcalID, std::vector<ldmx::HcalHit>> hits_by_id;
  for (auto const& hit : hcal_rec_hits) {
    ldmx::HcalID id(hit.getSection(), hit.getLayer(), hit.getStrip());

    auto idh = hits_by_id.find(id);
    if (idh == hits_by_id.end()) {
      hits_by_id[id] = std::vector<ldmx::HcalHit>(1, hit);
    } else {
      idh->second.push_back(hit);
    }
  }

  // make pairs of hcal rechits indices that belong to the same pulse
  // @TODO: for now we just take the first two indices that have opposite-ends
  //        we do not cover the case where two hits_ come separated in time
  std::map<ldmx::HcalID, std::pair<int, int>> indices_by_id;
  for (auto const& hcal_bar : hits_by_id) {
    auto id = hcal_bar.first;

    std::pair<int, int> indices(-1, -1);
    int i_hit = 0;
    while (i_hit < hcal_bar.second.size()) {
      auto hit = hcal_bar.second.at(i_hit);

      ldmx::HcalDigiID digi_id(hit.getSection(), hit.getLayer(), hit.getStrip(),
                               hit.getEnd());
      if (digi_id.isNegativeEnd() && indices.second == -1) {
        indices.second = i_hit;
      }
      if (!digi_id.isNegativeEnd() && indices.first == -1) {
        indices.first = i_hit;
      }
      i_hit++;
    }
    indices_by_id[id] = indices;
  }

  // reconstruct double-ended hits_
  for (auto const& hcal_bar : hits_by_id) {
    auto id = hcal_bar.first;

    // get bar position from geometry
    auto position = hcal_geometry.getStripCenterPosition(id);
    const auto orientation{hcal_geometry.getScintillatorOrientation(id)};
    int orientation_int = static_cast<int>(orientation);

    // skip non-double-ended layers
    if (id.section() != ldmx::HcalID::HcalSection::BACK) continue;

    // skip if we couldn't find both ends of the bar
    auto indices = indices_by_id[id];
    if (indices.first == -1 || indices.second == -1) {
      ldmx_log(warn) << "Couldn't find both ends of the bar "
                     << " for HcalID section " << static_cast<int>(id.section())
                     << " layer " << id.layer() << " strip " << id.strip();
      continue;
    }

    // get two hits_ to reconstruct
    auto hit_pos_end = hcal_bar.second.at(indices.first);
    auto hit_neg_end = hcal_bar.second.at(indices.second);

    // update TOA hit with negative end with mean shift
    ldmx::HcalDigiID digi_id_pos(hit_pos_end.getSection(),
                                 hit_pos_end.getLayer(), hit_pos_end.getStrip(),
                                 hit_pos_end.getEnd());
    ldmx::HcalDigiID digi_id_neg(hit_neg_end.getSection(),
                                 hit_neg_end.getLayer(), hit_neg_end.getStrip(),
                                 hit_neg_end.getEnd());
    double mean_shift = conditions.toaCalib(digi_id_neg.raw(), 1);

    double pos_time = hit_pos_end.getTime();
    double neg_time = hit_neg_end.getTime();
    if (pos_time != 0 && neg_time != 0) {
      neg_time = neg_time - mean_shift;
    }

    // update position in strip according to time measurement
    // velocity of light in polystyrene, n = 1.6 = c/v
    double v = 299.792 / 1.6;
    double hit_time_diff = pos_time - neg_time;

    ldmx_log(trace) << "\n new hit ";
    ldmx_log(trace) << "strip " << id.strip() << " layer_ " << id.layer()
                    << "center position X = " << position.X()
                    << " Y =" << position.Y() << " Z = " << position.Z();
    ldmx_log(trace) << "hittime pos_ " << pos_time << "neg " << neg_time
                    << " bar sign " << " diff " << hit_time_diff;

    int position_bar_sign = hit_time_diff > 0 ? 1 : -1;
    double position_unchanged = 0;
    double position_bar = position_bar_sign * fabs(hit_time_diff) * v / 2;
    if (orientation ==
        ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
      position_unchanged = position.X();
      position.SetX(position_bar);
    } else {
      position_unchanged = position.Y();
      position.SetY(position_bar);
    }
    ldmx_log(trace) << "position unchanged " << position_unchanged
                    << " orientation = " << orientation_int;
    ldmx_log(trace) << "newposition X = " << position.X()
                    << " Y = " << position.Y() << " Z = " << position.Z();

    // TODO: switch unique hit time for this pulse
    [[maybe_unused]] double hit_time =
        (hit_pos_end.getTime() + hit_neg_end.getTime());

    // amplitude and PEs
    double num_mips_equivalent =
        (hit_pos_end.getAmplitude() + hit_neg_end.getAmplitude());
    double p_es = (hit_pos_end.getPE() + hit_neg_end.getPE());
    double reconstructed_energy =
        num_mips_equivalent * pe_per_mip_ * mip_energy_;

    // reconstructed Hit
    ldmx::HcalHit rec_hit;
    rec_hit.setID(id.raw());
    rec_hit.setXPos(position.X());
    rec_hit.setYPos(position.Y());
    rec_hit.setZPos(position.Z());
    rec_hit.setSection(id.section());
    rec_hit.setStrip(id.strip());
    rec_hit.setLayer(id.layer());
    rec_hit.setPE(p_es);
    rec_hit.setMinPE(std::min(hit_pos_end.getPE(), hit_neg_end.getPE()));
    rec_hit.setAmplitude(num_mips_equivalent);
    rec_hit.setAmplitudePos(hit_pos_end.getAmplitude());
    rec_hit.setAmplitudeNeg(hit_neg_end.getAmplitude());
    rec_hit.setToaPos(hit_pos_end.getTime());
    rec_hit.setToaNeg(hit_neg_end.getTime());
    rec_hit.setEnergy(reconstructed_energy);
    rec_hit.setTime(hit_time_diff);
    rec_hit.setTimeDiff(hit_pos_end.getTime() - hit_neg_end.getTime());
    rec_hit.setPositionUnchanged(position_unchanged, orientation_int);
    double_hcal_rec_hits.push_back(rec_hit);
  }

  // add collection to event bus
  event.add(rec_coll_name_, double_hcal_rec_hits);
}

}  // namespace hcal
DECLARE_PRODUCER(hcal::HcalDoubleEndRecProducer);
