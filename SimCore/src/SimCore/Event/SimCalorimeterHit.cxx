#include "SimCore/Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(ldmx::SimCalorimeterHit)

    namespace ldmx {
  const std::string SimCalorimeterHit::ECAL_COLLECTION = "EcalSimHits";

  const std::string SimCalorimeterHit::HCAL_COLLECTION = "HcalSimHits";

  void SimCalorimeterHit::Clear() {
    incidentIDContribs_.clear();
    trackIDContribs_.clear();
    pdgCodeContribs_.clear();
    edepContribs_.clear();
    timeContribs_.clear();

    nContribs_ = 0;
    id_ = 0;
    edep_ = 0;
    x_ = 0;
    y_ = 0;
    z_ = 0;
    time_ = 0;
  }

  void SimCalorimeterHit::Print() const {
    std::cout << "SimCalorimeterHit { "
              << "id: " << id_ << ",  edep: " << edep_
              << ", "
                 "position: ( "
              << x_ << ", " << y_ << ", " << z_
              << " ), num contribs: " << nContribs_ << " }" << std::endl;
  }

  void SimCalorimeterHit::addContrib(int incidentID, int trackID, int pdgCode,
                                     float edep, float time) {
    incidentIDContribs_.push_back(incidentID);
    trackIDContribs_.push_back(trackID);
    pdgCodeContribs_.push_back(pdgCode);
    edepContribs_.push_back(edep);
    timeContribs_.push_back(time);
    edep_ += edep;
    if (time < time_ || time_ == 0) {
      time_ = time;
    }
    ++nContribs_;
  }

  SimCalorimeterHit::Contrib SimCalorimeterHit::getContrib(int i) const {
    Contrib contrib;
    contrib.incidentID = incidentIDContribs_.at(i);
    contrib.trackID = trackIDContribs_.at(i);
    contrib.edep = edepContribs_.at(i);
    contrib.time = timeContribs_.at(i);
    contrib.pdgCode = pdgCodeContribs_.at(i);
    return contrib;
  }

  int SimCalorimeterHit::findContribIndex(int trackID, int pdgCode) const {
    int contribIndex = -1;
    for (int iContrib = 0; iContrib < nContribs_; iContrib++) {
      Contrib contrib = getContrib(iContrib);
      if (contrib.trackID == trackID && contrib.pdgCode == pdgCode) {
        contribIndex = iContrib;
        break;
      }
    }
    return contribIndex;
  }

  void SimCalorimeterHit::updateContrib(int i, float edep, float time) {
    this->edepContribs_[i] += edep;
    if (time < this->timeContribs_.at(i)) {
      this->timeContribs_[i] = time;
    }
    edep_ += edep;
  }
}  // namespace ldmx
