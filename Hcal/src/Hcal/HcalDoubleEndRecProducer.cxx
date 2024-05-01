#include "Hcal/HcalDoubleEndRecProducer.h"

namespace hcal {

void HcalDoubleEndRecProducer::configure(framework::config::Parameters& p) {
  pass_name_ = p.getParameter("pass_name", pass_name_);
  coll_name_ = p.getParameter("coll_name", coll_name_);

  rec_pass_name_ = p.getParameter("rec_pass_name", pass_name_);
  rec_coll_name_ = p.getParameter("rec_coll_name", coll_name_);

  pe_per_mip_ = p.getParameter<double>("pe_per_mip");
  mip_energy_ = p.getParameter<double>("mip_energy");
  clock_cycle_ = p.getParameter<double>("clock_cycle");
}

void HcalDoubleEndRecProducer::produce(framework::Event& event) {
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  const auto& conditions{
      getCondition<HcalReconConditions>(HcalReconConditions::CONDITIONS_NAME)};

  auto hcalRecHits = event.getCollection<ldmx::HcalHit>(coll_name_, pass_name_);

  std::vector<ldmx::HcalHit> doubleHcalRecHits;

  // group hcal rechits by the same HcalID
  std::map<ldmx::HcalID, std::vector<ldmx::HcalHit>> hitsByID;
  for (auto const& hit : hcalRecHits) {
    ldmx::HcalID id(hit.getSection(), hit.getLayer(), hit.getStrip());

    auto idh = hitsByID.find(id);
    if (idh == hitsByID.end()) {
      hitsByID[id] = std::vector<ldmx::HcalHit>(1, hit);
    } else {
      idh->second.push_back(hit);
    }
  }

  // make pairs of hcal rechits indices that belong to the same pulse
  // @TODO: for now we just take the first two indices that have opposite-ends
  //        we do not cover the case where two hits come separated in time
  std::map<ldmx::HcalID, std::pair<int, int>> indicesByID;
  for (auto const& hcalBar : hitsByID) {
    auto id = hcalBar.first;

    std::pair<int, int> indices(-1, -1);
    int iHit = 0;
    while (iHit < hcalBar.second.size()) {
      auto hit = hcalBar.second.at(iHit);

      ldmx::HcalDigiID digi_id(hit.getSection(), hit.getLayer(), hit.getStrip(),
                               hit.getEnd());
      if (digi_id.isNegativeEnd() && indices.second == -1) {
        indices.second = iHit;
      }
      if (!digi_id.isNegativeEnd() && indices.first == -1) {
        indices.first = iHit;
      }
      iHit++;
    }
    indicesByID[id] = indices;
  }

  // reconstruct double-ended hits
  for (auto const& hcalBar : hitsByID) {
    auto id = hcalBar.first;

    // get bar position from geometry
    auto position = hcalGeometry.getStripCenterPosition(id);
    const auto orientation{hcalGeometry.getScintillatorOrientation(id)};

    // skip non-double-ended layers
    if (id.section() != ldmx::HcalID::HcalSection::BACK) continue;

    // get two hits to reconstruct
    auto hitPosEnd = hcalBar.second.at(indicesByID[id].first);
    auto hitNegEnd = hcalBar.second.at(indicesByID[id].second);

    // update TOA hit with negative end with mean shift
    ldmx::HcalDigiID digi_id_pos(hitPosEnd.getSection(), hitPosEnd.getLayer(),
                                 hitPosEnd.getStrip(), hitPosEnd.getEnd());
    ldmx::HcalDigiID digi_id_neg(hitNegEnd.getSection(), hitNegEnd.getLayer(),
                                 hitNegEnd.getStrip(), hitNegEnd.getEnd());
    double mean_shift = conditions.toaCalib(digi_id_neg.raw(), 1);

    double pos_time = hitPosEnd.getTime();
    double neg_time = hitNegEnd.getTime();
    if (pos_time != 0 || neg_time != 0) {
      neg_time = neg_time - mean_shift;
    }

    // update position in strip according to time measurement
    double v =
        299.792 / 1.6;  // velocity of light in polystyrene, n = 1.6 = c/v
    double hitTimeDiff = pos_time - neg_time;

    // std::cout << "\n new hit " << std::endl;
    // std::cout << "strip " << id.strip() << " layer " << id.layer() << "
    // center position " << position.X() << " " << position.Y() << " " <<
    // position.Z() << std::endl; std::cout << "hittime pos " << pos_time << "
    // neg " << neg_time << " bar sign " << " diff " << hitTimeDiff <<
    // std::endl;

    int position_bar_sign = hitTimeDiff > 0 ? 1 : -1;
    double position_unchanged = 0;
    int isX = 0;
    double position_bar = position_bar_sign * fabs(hitTimeDiff) * v / 2;
    if (orientation ==
        ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
      position_unchanged = position.X();
      isX = 1;
      position.SetX(position_bar);
    } else {
      position_unchanged = position.Y();
      isX = 0;
      position.SetY(position_bar);
    }
    // std::cout << "position unchanged " << position_unchanged << " isx " <<
    // isX << std::endl; std::cout << "newposition " << position.X() << " " <<
    // position.Y() << " " << position.Z() << std::endl;

    // TODO: switch unique hit time for this pulse
    [[maybe_unused]] double hitTime =
        (hitPosEnd.getTime() + hitNegEnd.getTime());

    // amplitude and PEs
    double num_mips_equivalent =
        (hitPosEnd.getAmplitude() + hitNegEnd.getAmplitude());
    double PEs = (hitPosEnd.getPE() + hitNegEnd.getPE());
    double reconstructed_energy =
        num_mips_equivalent * pe_per_mip_ * mip_energy_;

    // reconstructed Hit
    ldmx::HcalHit recHit;
    recHit.setID(id.raw());
    recHit.setXPos(position.X());
    recHit.setYPos(position.Y());
    recHit.setZPos(position.Z());
    recHit.setSection(id.section());
    recHit.setStrip(id.strip());
    recHit.setLayer(id.layer());
    recHit.setPE(PEs);
    recHit.setMinPE(std::min(hitPosEnd.getPE(), hitNegEnd.getPE()));
    recHit.setAmplitude(num_mips_equivalent);
    recHit.setAmplitudePos(hitPosEnd.getAmplitude());
    recHit.setAmplitudeNeg(hitNegEnd.getAmplitude());
    recHit.setToaPos(hitPosEnd.getTime());
    recHit.setToaNeg(hitNegEnd.getTime());
    recHit.setEnergy(reconstructed_energy);
    recHit.setTime(hitTimeDiff);
    recHit.setTimeDiff(hitPosEnd.getTime() - hitNegEnd.getTime());
    recHit.setPositionUnchanged(position_unchanged, isX);
    doubleHcalRecHits.push_back(recHit);
  }

  // add collection to event bus
  event.add(rec_coll_name_, doubleHcalRecHits);
}

}  // namespace hcal
DECLARE_PRODUCER_NS(hcal, HcalDoubleEndRecProducer);
