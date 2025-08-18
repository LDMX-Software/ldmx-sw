#include "Tracking/Sim/PropagatorStepWriter.h"

//--- ACTS --- //
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingVolume.hpp>
#include <Acts/Propagator/ConstrainedStep.hpp>
#include <Acts/Surfaces/Surface.hpp>

#include "Acts/Geometry/DetectorElementBase.hpp"
// mg ... I don't think these are used, and they are not defined in acts v36
// #include "Acts/Plugins/Identification/IdentifiedDetectorElement.hpp"
// #include "Acts/Plugins/Identification/Identifier.hpp"

namespace tracking {
namespace sim {

PropagatorStepWriter::PropagatorStepWriter(
    const tracking::sim::PropagatorStepWriter::Config& cfg)
    : m_cfg_(cfg), m_output_file_(cfg.rootFile) {
  if (m_cfg.treeName.empty()) {
    throw std::invalid_argument("Missing tree name");
  }

  // Setup ROOT I/O
  if (m_outputFile == nullptr) {
    m_output_file_ = TFile::Open(m_cfg.filePath.c_str(), m_cfg.fileMode.c_str());
    if (m_outputFile == nullptr) {
      throw std::ios_base::failure("Could not open '" + m_cfg.filePath);
    }
  }
  m_output_file_->cd();

  m_output_tree_ = new TTree(m_cfg.treeName.c_str(),
                           "TTree from RootPropagationStepsWriter");
  if (m_outputTree == nullptr) throw std::bad_alloc();

  // Set the branches
  m_output_tree_->Branch("event_nr", &m_event_nr_);
  //  m_outputTree->Branch("volume_id", &m_volumeID);
  m_output_tree_->Branch("boundary_id", &m_boundary_id_);
  m_output_tree_->Branch("layer_id", &m_layerID);
  m_output_tree_->Branch("approach_id", &m_approach_id_);
  m_output_tree_->Branch("sensitive_id", &m_sensitive_id_);
  m_output_tree_->Branch("g_x", &m_x_);
  m_output_tree_->Branch("g_y", &m_y_);
  m_output_tree_->Branch("g_z", &m_z_);
  m_output_tree_->Branch("d_x", &m_dx_);
  m_output_tree_->Branch("d_y", &m_dy_);
  m_output_tree_->Branch("d_z", &m_dz_);
  m_output_tree_->Branch("type", &m_step_type_);
  m_output_tree_->Branch("step_acc", &m_step_acc_);
  m_output_tree_->Branch("step_act", &m_step_act_);
  m_output_tree_->Branch("step_abt", &m_step_abt_);
  m_output_tree_->Branch("step_usr", &m_step_usr_);
  m_output_tree_->Branch("hit_x", &m_hit_x_);
  m_output_tree_->Branch("hit_y", &m_hit_y_);
  m_output_tree_->Branch("hit_z", &m_hit_z_);
  m_output_tree_->Branch("start_pos", &m_start_pos_);
  m_output_tree_->Branch("start_mom", &m_start_mom_);

}  // constructor

PropagatorStepWriter::~PropagatorStepWriter() {
  /// Close the file if it's yours
  if (m_cfg.rootFile == nullptr) {
    m_output_file_->cd();
    m_output_tree_->Write();
    m_output_file_->Close();
  }
}  // destructor

bool PropagatorStepWriter::writeSteps(
    framework::Event& event,
    const std::vector<PropagationSteps>& stepCollection,
    const std::vector<ldmx::Measurement>& measurements,
    const Acts::Vector3& start_pos, const Acts::Vector3& start_mom) {
  // Exclusive access to the tree while writing
  std::lock_guard<std::mutex> lock(m_writeMutex);

  m_output_file_->cd();

  // we get the event number
  m_event_nr_ = event.getEventNumber();

  // fill the hits_
  m_hit_x_.clear();
  m_hit_y_.clear();
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
    //    m_volumeID.clear();
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
      //      Acts::GeometryIdentifier::Value volumeID = 0;
      Acts::GeometryIdentifier::Value boundary_id = 0;
      Acts::GeometryIdentifier::Value layer_id = 0;
      Acts::GeometryIdentifier::Value approach_id = 0;
      Acts::GeometryIdentifier::Value sensitive_id = 0;
      // get the identification from the surface first
      if (step.surface) {
        auto geo_id = step.surface->geometryId();
        //        volumeID = geoID.volume();
        boundary_id = geo_id.boundary();
        layer_id = geo_id.layer();
        approach_id = geo_id.approach();
        sensitive_id = geo_id.sensitive();
      }
      // a current volume overwrites the surface tagged one
      // mg ... v36 Step does not include volume
      //      if (step.volume) {
      //    volumeID = step.volume->geometryId().volume();
      //   }
      // now fill
      m_sensitiveID.push_back(sensitive_id);
      m_approachID.push_back(approach_id);
      m_layerID.push_back(layer_id);
      m_boundaryID.push_back(boundary_id);
      // m_volumeID.push_back(volumeID);

      // kinematic information
      m_x.push_back(step.position.x());
      m_y.push_back(step.position.y());
      m_z.push_back(step.position.z());
      auto direction = step.momentum.normalized();
      m_dx.push_back(direction.x());
      m_dy.push_back(direction.y());
      m_dz.push_back(direction.z());

      //      double accuracy =
      //      step.stepSize.value(Acts::ConstrainedStep::accuracy());
      double accuracy = step.stepSize.accuracy();
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
