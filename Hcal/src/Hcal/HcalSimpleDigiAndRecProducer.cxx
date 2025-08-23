#include "Hcal/HcalSimpleDigiAndRecProducer.h"

#include "Hcal/Event/HcalHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace hcal {

void HcalSimpleDigiAndRecProducer::configure(
    framework::config::Parameters& ps) {
  input_coll_name_ = ps.get<std::string>("input_coll_name");
  input_pass_name_ = ps.get<std::string>("input_pass_name");
  output_coll_name_ = ps.get<std::string>("output_coll_name");
  mev_per_mip_ = ps.get<double>("mev_per_mip");
  pe_per_mip_ = ps.get<double>("pe_per_mip");
  attenuation_length_ = ps.get<double>("attenuation_length");
  readout_threshold_ = ps.get<int>("readout_threshold");
  mean_noise_ = ps.get<double>("mean_noise");
  position_resolution_smear_ =
      std::make_unique<std::normal_distribution<double>>(
          0.0, ps.get<double>("position_resolution"));
}

void HcalSimpleDigiAndRecProducer::onNewRun(const ldmx::RunHeader&) {
  noise_generator_ = std::make_unique<ldmx::NoiseGenerator>(mean_noise_, false);
  // hard-code this number, create noise hits_ for non-zero PEs!
  noise_generator_->setNoiseThreshold(1);
  const framework::RandomNumberSeedService& rseed =
      getCondition<framework::RandomNumberSeedService>(
          framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  noise_generator_->seedGenerator(
      rseed.getSeed("HcalSimpleDigiAndRecProducer::NoiseGenerator"));
  rng_.seed(rseed.getSeed("HcalSimpleDigiAndRecProducer"));
}

void HcalSimpleDigiAndRecProducer::produce(framework::Event& event) {
  const auto& hcal_geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  std::vector<ldmx::HcalHit> hcal_rec_hits;

  auto sim_hits{event.getCollection<ldmx::SimCalorimeterHit>(input_coll_name_,
                                                             input_pass_name_)};
  std::unordered_map<unsigned int, std::vector<const ldmx::SimCalorimeterHit*>>
      hits_by_id{};
  // Important, has to be a reference so that we don't take the address of a
  // variable that goes out of scope!
  for (const auto& hit : sim_hits) {
    auto id{hit.getID()};
    auto found{hits_by_id.find(id)};
    if (found == hits_by_id.end()) {
      hits_by_id[id] = std::vector<const ldmx::SimCalorimeterHit*>{&hit};
    } else {
      hits_by_id[id].push_back(&hit);
    }
  }
  for (const auto& [barID, simhits_in_bar] : hits_by_id) {
    ldmx::HcalHit& rec_hit = hcal_rec_hits.emplace_back();
    double edep{};
    double time{};
    std::vector<double> pos{0, 0, 0};
    for (auto hit : simhits_in_bar) {
      edep += hit->getEdep();
      double edep_hit = hit->getEdep();
      time += hit->getTime() * edep_hit;
      auto hit_pos{hit->getPosition()};
      pos[0] += hit_pos[0] * edep_hit;
      pos[1] += hit_pos[1] * edep_hit;
      pos[2] += hit_pos[2] * edep_hit;
    }
    ldmx::HcalID hit_id{barID};

    // Position smearing
    double mean_pe{(edep / mev_per_mip_) * pe_per_mip_};
    double xpos{pos[0] / edep};
    double ypos{pos[1] / edep};
    double zpos{pos[2] / edep};
    time /= edep;

    auto orientation{hcal_geometry.getScintillatorOrientation(barID)};
    double half_total_width{
        hcal_geometry.getHalfTotalWidth(hit_id.section(), hit_id.layer())};
    double scint_bar_length{hcal_geometry.getScintillatorLength(hit_id)};

    auto strip_center{hcal_geometry.getStripCenterPosition(hit_id)};
    if (hit_id.section() == ldmx::HcalID::HcalSection::BACK) {
      double distance_along_bar =
          (orientation ==
           ldmx::HcalGeometry::ScintillatorOrientation::horizontal)
              ? xpos
              : ypos;
      if (orientation ==
          ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
        ypos = strip_center.y();
        xpos += (*position_resolution_smear_)(rng_);
      } else {
        xpos = strip_center.x();
        ypos += (*position_resolution_smear_)(rng_);
      }
      zpos = strip_center.z();
      // Attenuation
      mean_pe *= exp(1. / attenuation_length_);
      double mean_pe_close =
          mean_pe * exp(-1. *
                        ((half_total_width - distance_along_bar) /
                         (scint_bar_length * 0.5)) /
                        attenuation_length_);
      double mean_pe_far =
          mean_pe * exp(-1. *
                        ((half_total_width + distance_along_bar) /
                         (scint_bar_length * 0.5)) /
                        attenuation_length_);
      int pe_close{
          std::poisson_distribution<int>(mean_pe_close + mean_noise_)(rng_)};
      int pe_far{
          std::poisson_distribution<int>(mean_pe_far + mean_noise_)(rng_)};
      rec_hit.setPE(pe_close + pe_far);
      rec_hit.setMinPE(std::min(pe_close, pe_far));
    } else {
      // Side HCAL, no attenuation business since single ended readout
      int pe{std::poisson_distribution<int>(mean_pe + mean_noise_)(rng_)};
      rec_hit.setPE(pe);
      rec_hit.setMinPE(pe);

      // Checks orientation of side Hcal bars, sets center positions and add
      // smearing along bar orientation axis
      if (orientation ==
          ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
        xpos += (*position_resolution_smear_)(rng_);
        ypos = strip_center.y();
        zpos = strip_center.z();
      } else if (orientation ==
                 ldmx::HcalGeometry::ScintillatorOrientation::vertical) {
        xpos = strip_center.x();
        ypos += (*position_resolution_smear_)(rng_);
        zpos = strip_center.z();
      } else if (orientation ==
                 ldmx::HcalGeometry::ScintillatorOrientation::depth) {
        xpos = strip_center.x();
        ypos = strip_center.y();
        zpos += (*position_resolution_smear_)(rng_);
      } else {
        xpos = strip_center.x();
        ypos = strip_center.y();
        zpos = strip_center.z();
        ldmx_log(warn) << "Bar orientation not found. Hit" << hit_id.raw()
                       << "positioned at bar center.";
      }
    }

    rec_hit.setID(hit_id.raw());
    rec_hit.setXPos(xpos);
    rec_hit.setNoise(false);
    rec_hit.setYPos(ypos);
    rec_hit.setZPos(zpos);
    rec_hit.setTime(time);
    rec_hit.setSection(hit_id.section());
    rec_hit.setStrip(hit_id.strip());
    rec_hit.setLayer(hit_id.layer());
    rec_hit.setEnergy(edep);
    rec_hit.setOrientation(static_cast<int>(orientation));
  }
  event.add(output_coll_name_, hcal_rec_hits);
}

}  // namespace hcal

DECLARE_PRODUCER(hcal::HcalSimpleDigiAndRecProducer);
