#include "Tracking/Sim/PropagatorStepWriter.h"

//--- ACTS --- //
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Propagator/ConstrainedStep.hpp>
#include <Acts/Surfaces/Surface.hpp>

#include "Acts/Geometry/DetectorElementBase.hpp"
#include "Acts/Plugins/Identification/IdentifiedDetectorElement.hpp"
#include "Acts/Plugins/Identification/Identifier.hpp"

namespace tracking {
namespace sim {

PropagatorStepWriter::PropagatorStepWriter(
    const tracking::sim::PropagatorStepWriter::Config& cfg)
    : m_cfg(cfg), m_outputFile(cfg.rootFile) {
  if (m_cfg.treeName.empty()) {
    throw std::invalid_argument("Missing tree name");
  }

  // Setup ROOT I/O
  if (m_outputFile == nullptr) {
    m_outputFile = TFile::Open(m_cfg.filePath.c_str(), m_cfg.fileMode.c_str());
    if (m_outputFile == nullptr) {
      throw std::ios_base::failure("Could not open '" + m_cfg.filePath);
    }
  }
  m_outputFile->cd();

  m_outputTree = new TTree(m_cfg.treeName.c_str(),
                           "TTree from RootPropagationStepsWriter");
  if (m_outputTree == nullptr) throw std::bad_alloc();

  // Set the branches
  m_outputTree->Branch("event_nr", &m_eventNr);
  m_outputTree->Branch("volume_id", &m_volumeID);
  m_outputTree->Branch("boundary_id", &m_boundaryID);
  m_outputTree->Branch("layer_id", &m_layerID);
  m_outputTree->Branch("approach_id", &m_approachID);
  m_outputTree->Branch("sensitive_id", &m_sensitiveID);
  m_outputTree->Branch("g_x", &m_x);
  m_outputTree->Branch("g_y", &m_y);
  m_outputTree->Branch("g_z", &m_z);
  m_outputTree->Branch("d_x", &m_dx);
  m_outputTree->Branch("d_y", &m_dy);
  m_outputTree->Branch("d_z", &m_dz);
  m_outputTree->Branch("type", &m_step_type);
  m_outputTree->Branch("step_acc", &m_step_acc);
  m_outputTree->Branch("step_act", &m_step_act);
  m_outputTree->Branch("step_abt", &m_step_abt);
  m_outputTree->Branch("step_usr", &m_step_usr);
  m_outputTree->Branch("hit_x", &m_hit_x);
  m_outputTree->Branch("hit_y", &m_hit_y);
  m_outputTree->Branch("hit_z", &m_hit_z);
  m_outputTree->Branch("start_pos", &m_start_pos);
  m_outputTree->Branch("start_mom", &m_start_mom);

}  // constructor

PropagatorStepWriter::~PropagatorStepWriter() {
  /// Close the file if it's yours
  if (m_cfg.rootFile == nullptr) {
    m_outputFile->cd();
    m_outputTree->Write();
    m_outputFile->Close();
  }
}  // destructor

bool PropagatorStepWriter::WriteSteps(
    framework::Event& event,
    const std::vector<PropagationSteps>& stepCollection,
    const std::vector<ldmx::Measurement>& measurements,
    const Acts::Vector3& start_pos, const Acts::Vector3& start_mom) {
  // Exclusive access to the tree while writing
  std::lock_guard<std::mutex> lock(m_writeMutex);

  m_outputFile->cd();

  // we get the event number
  m_eventNr = event.getEventNumber();

  // fill the hits
  m_hit_x.clear();
  m_hit_y.clear();
  m_hit_z.clear();
  // m_hit_erru.clear();
  // m_hit_errv.clear();

  m_start_pos.clear();
  m_start_mom.clear();

  for (auto& meas : measurements) {
    m_hit_x.push_back(meas.getGlobalPosition()[0]);
    m_hit_y.push_back(meas.getGlobalPosition()[1]);
    m_hit_z.push_back(meas.getGlobalPosition()[2]);
  }
  
  for (unsigned int i = 0; i < 3; i++) {
    m_start_pos.push_back(start_pos(i));
    m_start_mom.push_back(start_mom(i));
  }

  // loop over the step vector of each test propagation in this
  for (auto& steps : stepCollection) {
    // clear the vectors for each collection
    m_volumeID.clear();
    m_boundaryID.clear();
    m_layerID.clear();
    m_approachID.clear();
    m_sensitiveID.clear();
    m_x.clear();
    m_y.clear();
    m_z.clear();
    m_dx.clear();
    m_dy.clear();
    m_dz.clear();
    m_step_type.clear();
    m_step_acc.clear();
    m_step_act.clear();
    m_step_abt.clear();
    m_step_usr.clear();

    // loop over single steps
    for (auto& step : steps) {
      // the identification of the step
      Acts::GeometryIdentifier::Value volumeID = 0;
      Acts::GeometryIdentifier::Value boundaryID = 0;
      Acts::GeometryIdentifier::Value layerID = 0;
      Acts::GeometryIdentifier::Value approachID = 0;
      Acts::GeometryIdentifier::Value sensitiveID = 0;
      // get the identification from the surface first
      if (step.surface) {
        auto geoID = step.surface->geometryId();
        volumeID = geoID.volume();
        boundaryID = geoID.boundary();
        layerID = geoID.layer();
        approachID = geoID.approach();
        sensitiveID = geoID.sensitive();
      }
      // a current volume overwrites the surface tagged one
      if (step.volume) {
        volumeID = step.volume->geometryId().volume();
      }
      // now fill
      m_sensitiveID.push_back(sensitiveID);
      m_approachID.push_back(approachID);
      m_layerID.push_back(layerID);
      m_boundaryID.push_back(boundaryID);
      m_volumeID.push_back(volumeID);

      // kinematic information
      m_x.push_back(step.position.x());
      m_y.push_back(step.position.y());
      m_z.push_back(step.position.z());
      auto direction = step.momentum.normalized();
      m_dx.push_back(direction.x());
      m_dy.push_back(direction.y());
      m_dz.push_back(direction.z());

      double accuracy = step.stepSize.value(Acts::ConstrainedStep::accuracy);
      double actor = step.stepSize.value(Acts::ConstrainedStep::actor);
      double aborter = step.stepSize.value(Acts::ConstrainedStep::aborter);
      double user = step.stepSize.value(Acts::ConstrainedStep::user);
      double act2 = actor * actor;
      double acc2 = accuracy * accuracy;
      double abo2 = aborter * aborter;
      double usr2 = user * user;

      // todo - fold with direction
      if (act2 < acc2 && act2 < abo2 && act2 < usr2) {
        m_step_type.push_back(0);
      } else if (acc2 < abo2 && acc2 < usr2) {
        m_step_type.push_back(1);
      } else if (abo2 < usr2) {
        m_step_type.push_back(2);
      } else {
        m_step_type.push_back(3);
      }

      // step size information
      m_step_acc.push_back(accuracy);
      m_step_act.push_back(actor);
      m_step_abt.push_back(aborter);
      m_step_usr.push_back(user);
    }
    m_outputTree->Fill();
  }
  return true;
}
}  // namespace sim
}  // namespace tracking
