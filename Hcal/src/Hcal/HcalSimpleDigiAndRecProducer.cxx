#include "Hcal/HcalSimpleDigiAndRecProducer.h"

#include "Hcal/Event/HcalHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace hcal {

void HcalSimpleDigiAndRecProducer::configure(
    framework::config::Parameters& ps) {
  input_coll_name_ = ps.getParameter<std::string>("input_coll_name");
  input_pass_name_ = ps.getParameter<std::string>("input_pass_name");
  output_coll_name_ = ps.getParameter<std::string>("output_coll_name");
  mev_per_mip_ = ps.getParameter<double>("mev_per_mip");
  pe_per_mip_ = ps.getParameter<double>("pe_per_mip");
  attenuation_length_ = ps.getParameter<double>("attenuation_length");
  readout_threshold_ = ps.getParameter<int>("readout_threshold");
  mean_noise_ = ps.getParameter<double>("mean_noise");
  position_resolution_smear_ =
      std::make_unique<std::normal_distribution<double>>(
          0.0, ps.getParameter<double>("position_resolution"));
}

void HcalSimpleDigiAndRecProducer::onNewRun(const ldmx::RunHeader&) {
  noiseGenerator_ = std::make_unique<ldmx::NoiseGenerator>(mean_noise_, false);
  // hard-code this number, create noise hits for non-zero PEs!
  noiseGenerator_->setNoiseThreshold(1);
  const framework::RandomNumberSeedService& rseed =
      getCondition<framework::RandomNumberSeedService>(
          framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  noiseGenerator_->seedGenerator(
      rseed.getSeed("HcalSimpleDigiAndRecProducer::NoiseGenerator"));
  rng_.seed(rseed.getSeed("HcalSimpleDigiAndRecProducer"));
}

void HcalSimpleDigiAndRecProducer::produce(framework::Event& event) {
  const auto& hcalGeometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);

  std::vector<ldmx::HcalHit> hcalRecHits;

  auto simHits{event.getCollection<ldmx::SimCalorimeterHit>(input_coll_name_,
                                                            input_pass_name_)};
  std::unordered_map<unsigned int, std::vector<const ldmx::SimCalorimeterHit*>>
      hits_by_id{};
  // Important, has to be a reference so that we don't take the address of a
  // variable that goes out of scope!
  for (const auto& hit : simHits) {
    auto id{hit.getID()};
    auto found{hits_by_id.find(id)};
    if (found == hits_by_id.end()) {
      hits_by_id[id] = std::vector<const ldmx::SimCalorimeterHit*>{&hit};
    } else {
      hits_by_id[id].push_back(&hit);
    }
  }
  for (const auto& [barID, simhits_in_bar] : hits_by_id) {
    ldmx::HcalHit& recHit = hcalRecHits.emplace_back();
    double edep{};
    double time{};
    std::vector<double> pos{0, 0, 0};
    for (auto hit : simhits_in_bar) {
      edep += hit->getEdep();
      time += hit->getTime() * edep;
      auto hitPos{hit->getPosition()};
      pos[0] += hitPos[0] * edep;
      pos[1] += hitPos[1] * edep;
      pos[2] += hitPos[2] * edep;
    }
    ldmx::HcalID hitID{barID};

    // Position smearing
    double mean_pe{(edep / mev_per_mip_) * pe_per_mip_};
    double xpos{pos[0] / edep};
    double ypos{pos[1] / edep};
    double zpos{pos[2] / edep};
    time /= edep;

    auto orientation{hcalGeometry.getScintillatorOrientation(barID)};
    double half_total_width{
        hcalGeometry.getHalfTotalWidth(hitID.section(), hitID.layer())};
    double scint_bar_length{hcalGeometry.getScintillatorLength(hitID)};

    auto stripCenter{hcalGeometry.getStripCenterPosition(hitID)};
    if (hitID.section() == ldmx::HcalID::HcalSection::BACK) {
      double distance_along_bar =
          (orientation ==
           ldmx::HcalGeometry::ScintillatorOrientation::horizontal)
              ? xpos
              : ypos;
      if (orientation ==
          ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
        ypos = stripCenter.y();
        xpos += (*position_resolution_smear_)(rng_);
      } else {
        xpos = stripCenter.x();
        ypos += (*position_resolution_smear_)(rng_);
      }
      zpos = stripCenter.z();
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
      int PE_close{
          std::poisson_distribution<int>(mean_pe_close + mean_noise_)(rng_)};
      int PE_far{
          std::poisson_distribution<int>(mean_pe_far + mean_noise_)(rng_)};
      recHit.setPE(PE_close + PE_far);
      recHit.setMinPE(std::min(PE_close, PE_far));
    } else {
      // Side HCAL, no attenuation business since single ended readout
      int PE{std::poisson_distribution<int>(mean_pe + mean_noise_)(rng_)};
      recHit.setPE(PE);
      recHit.setMinPE(PE);
      // TODO: Look into this
      xpos = stripCenter.x();
      ypos = stripCenter.y();
      zpos = stripCenter.z();
    }

    recHit.setID(hitID.raw());
    recHit.setXPos(xpos);
    recHit.setNoise(false);
    recHit.setYPos(ypos);
    recHit.setZPos(zpos);
    recHit.setTime(time);
    recHit.setSection(hitID.section());
    recHit.setStrip(hitID.strip());
    recHit.setLayer(hitID.layer());
    recHit.setEnergy(edep);
    recHit.setOrientation(static_cast<int>(orientation));
  }
  event.add(output_coll_name_, hcalRecHits);
}

}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalSimpleDigiAndRecProducer);
