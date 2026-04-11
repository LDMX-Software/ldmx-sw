#include "SimCore/Event/SimCalorimeterHit.h"

// STL
#include <iostream>

ClassImp(ldmx::SimCalorimeterHit);

namespace ldmx {
const std::string SimCalorimeterHit::ECAL_COLLECTION = "EcalSimHits";

const std::string SimCalorimeterHit::HCAL_COLLECTION = "HcalSimHits";

void SimCalorimeterHit::clear() {
  incident_id_contribs_.clear();
  track_id_contribs_.clear();
  pdg_code_contribs_.clear();
  edep_contribs_.clear();
  time_contribs_.clear();
  origin_contribs_.clear();

  n_contribs_ = 0;
  id_ = 0;
  edep_ = 0;
  x_ = 0;
  y_ = 0;
  z_ = 0;
  time_ = 0;
}

std::ostream& operator<<(std::ostream& o, const SimCalorimeterHit& hit) {
  return o << "SimCalorimeterHit { " << "id: " << hit.id_
           << ",  edep: " << hit.edep_
           << ", "
              "position: ( "
           << hit.x_ << ", " << hit.y_ << ", " << hit.z_
           << " ), num contribs: " << hit.n_contribs_ << " }";
}

std::ostream& operator<<(std::ostream& o,
                         const SimCalorimeterHit::Contrib& contrib) {
  return o << "Contrib { " << "incident_id: " << contrib.incident_id_
           << ",  track_id: " << contrib.track_id_
           << ", pdg_code: " << contrib.pdg_code_ << ", edep: " << contrib.edep_
           << ", time: " << contrib.time_
           << ", origin_id: " << contrib.origin_id_ << " }";
}

void SimCalorimeterHit::addContrib(int incidentID, int trackID, int pdgCode,
                                   float edep, float time, int originID) {
  incident_id_contribs_.push_back(incidentID);
  track_id_contribs_.push_back(trackID);
  pdg_code_contribs_.push_back(pdgCode);
  edep_contribs_.push_back(edep);
  time_contribs_.push_back(time);
  origin_contribs_.push_back(originID);
  edep_ += edep;
  if (time < time_ || time_ == 0) {
    time_ = time;
  }
  ++n_contribs_;
}

SimCalorimeterHit::Contrib SimCalorimeterHit::getContrib(int i) const {
  Contrib contrib;
  contrib.incident_id_ = incident_id_contribs_.at(i);
  contrib.track_id_ = track_id_contribs_.at(i);
  contrib.edep_ = edep_contribs_.at(i);
  contrib.time_ = time_contribs_.at(i);
  contrib.pdg_code_ = pdg_code_contribs_.at(i);
  contrib.origin_id_ = origin_contribs_.at(i);
  return contrib;
}

int SimCalorimeterHit::findContribIndex(int trackID, int pdgCode) const {
  int contrib_index = -1;
  for (int i_contrib = 0; i_contrib < n_contribs_; i_contrib++) {
    Contrib contrib = getContrib(i_contrib);
    if (contrib.track_id_ == trackID && contrib.pdg_code_ == pdgCode) {
      contrib_index = i_contrib;
      break;
    }
  }
  return contrib_index;
}

void SimCalorimeterHit::updateContrib(int i, float edep, float time) {
  this->edep_contribs_[i] += edep;
  if (time < this->time_contribs_.at(i)) {
    this->time_contribs_[i] = time;
  }
  edep_ += edep;
}

void SimCalorimeterHit::encodeTracks(
    std::function<int(int, unsigned int, unsigned int)> encodeFunc,
    const unsigned int encoding_version, const unsigned int event_index) {
  for (int i_contrib = 0; i_contrib < this->n_contribs_; i_contrib++) {
    this->track_id_contribs_[i_contrib] = encodeFunc(
        this->track_id_contribs_[i_contrib], encoding_version, event_index);
    this->incident_id_contribs_[i_contrib] = encodeFunc(
        this->incident_id_contribs_[i_contrib], encoding_version, event_index);
    this->origin_contribs_[i_contrib] = encodeFunc(
        this->origin_contribs_[i_contrib], encoding_version, event_index);
  }
}

}  // namespace ldmx
