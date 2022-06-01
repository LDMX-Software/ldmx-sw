#ifndef TRACKUTILS_H_
#define TRACKUTILS_H_

//TODO:: MAKE A CXX!!


//Recoil back layers numbering scheme for module

//    +Y  /\   4  3  2  1  0
//        |
//        | 
//    -Y  \/   9  8  7  6  5      
//          -X <----  ----> +X

//ModN (x,    y,   z)
//0    (96,   40,  z2)
//1    (48,   40,  z1)
//2    (0,    40,  z2)
//3    (-48,  40,  z1)
//4    (-96,  40,  z2)

//5    (96,  -40,  z2)
//6    (48,  -40,  z1)
//7    (0,   -40,  z2)
//8    (-48, -40,  z1)
//9    (-96, -40,  z2)


// ---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"
#include "Tracking/Sim/LdmxSpacePoint.h"

// --- Tracking ---//
#include "Tracking/Event/Track.h"

// --- < ACTS > --- //
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

namespace tracking{
namespace sim{
namespace utils {

//This method converts a SimHit in a LdmxSpacePoint for the Acts seeder.
// (1) Rotate the coordinates into acts::seedFinder coordinates defined by B-Field along z axis [Z_ldmx -> X_acts, X_ldmx->Y_acts, Y_ldmx->Z_acts]
// (2) Saves the error information. At the moment the errors are fixed. They should be obtained from the digitized hits.
      
//TODO::Move to shared pointers?!
//TODO::Pass to instances?
//Vol==2 for tagger, Vol==3 for recoil

inline ldmx::LdmxSpacePoint* convertSimHitToLdmxSpacePoint(const ldmx::SimTrackerHit& hit,
                                                           unsigned int vol = 2,
                                                           double sigma_u = 0.05,
                                                           double sigma_v = 1.) {
  
  bool debug = false;
  
  std::vector<float> sim_hit_pos = hit.getPosition();
  
  //check that if the coordinate along the beam is positive, then it's a recoil hit
  //TODO!! FIX THIS HARDCODE!
  if (sim_hit_pos[2] > 0)
    vol = 3;
  
  float ldmxsp_x = sim_hit_pos[2];
  float ldmxsp_y = sim_hit_pos[0];
  float ldmxsp_z = sim_hit_pos[1];

  unsigned int sensorId = 0;
  unsigned int layerId  = 0;
  
  //tagger numbering scheme for surfaces mapping
  //Layers from 1 to 14  => transform to 0->13
  if (vol == 2) {
    sensorId = (hit.getLayerID() + 1) % 2; //0,1,0,1 ...
    layerId  = (hit.getLayerID() + 1) / 2; //1,2,3,4,5,6,7    
  }

  //recoil numbering scheme for surfaces mapping 
  if (vol == 3) {

    //For axial-stereo modules use the same numbering scheme as the tagger
    if (hit.getLayerID() < 9 ) {
      sensorId = (hit.getLayerID() + 1 ) % 2;
      layerId  = (hit.getLayerID() + 1 ) / 2;
    }

    //For the axial only modules 
    else {
      sensorId = hit.getModuleID();
      layerId  = (hit.getLayerID() + 2 ) / 2;  //9->11 /2 = 5 10->12 / 2 = 6
    }
    
    
  }

  //vol * 1000 + ly * 100 + sensor
  unsigned int index  = vol * 1000 + layerId * 100 + sensorId;

  if (debug) {
    std::cout<<"LdmxSpacePointConverter::Check index::"<<vol<<"--"<<layerId<<"--"<<sensorId<<"==>"<<index<<std::endl;
    std::cout<<vol<<"==="<<hit.getLayerID()<<"==="<<hit.getModuleID()<<std::endl;
    std::cout<<"("<<ldmxsp_x<<","<<ldmxsp_y<<","<<ldmxsp_z<<")"<<std::endl;
  }

  return new ldmx::LdmxSpacePoint(ldmxsp_x, ldmxsp_y,ldmxsp_z,
                                  hit.getTime(), index, hit.getEdep(), 
                                  sigma_u*sigma_u, sigma_v*sigma_v,
                                  hit.getID());
  
}

inline void flatCov(Acts::BoundSymMatrix cov, std::vector<double>& v_cov) {
  
  v_cov.clear();
  v_cov.reserve(cov.rows() * (cov.rows() + 1) / 2);
  for (int i = 0 ; i<cov.rows(); i++)
    for (int j = i ; j<cov.cols(); j++)
      v_cov.push_back(cov(i,j));
}

inline Acts::BoundSymMatrix unpackCov(const std::vector<double>& v_cov) {
  
  Acts::BoundSymMatrix cov;
  int e{0};
  for (int i = 0; i < cov.rows(); i++)
    for (int j = i; j < cov.cols(); j++) {
      cov(i,j)=v_cov.at(e);
      cov(j,i)=cov(i,j);
      e++;
    }
  
  return cov;
}

//Rotate to ACTS frame
//z->x, x->y, y->z

//(0 0 1) x  = z 
//(1 0 0) y  = x
//(0 1 0) z  = y
 
inline Acts::Vector3 Ldmx2Acts(Acts::Vector3 ldmx_v) {
  
  //TODO::Move it to a static member
  Acts::SymMatrix3 acts_rot;
  acts_rot << 0.,0.,1.,
      1.,0.,0.,
      0.,1,0.;
  
  return acts_rot * ldmx_v;
}

//Transform position, momentum and charge to free parameters

inline Acts::FreeVector toFreeParameters(Acts::Vector3 pos, Acts::Vector3 mom, Acts::ActsScalar q) {
  
  Acts::FreeVector free_params;
  Acts::ActsScalar p = mom.norm() * Acts::UnitConstants::MeV;
  
  free_params[Acts::eFreePos0]   = pos(Acts::ePos0) * Acts::UnitConstants::mm;
  free_params[Acts::eFreePos1]   = pos(Acts::ePos1) * Acts::UnitConstants::mm;
  free_params[Acts::eFreePos2]   = pos(Acts::ePos2) * Acts::UnitConstants::mm;
  free_params[Acts::eFreeTime]   = 0.;  
  free_params[Acts::eFreeDir0]   = mom(0) / mom.norm();
  free_params[Acts::eFreeDir1]   = mom(1) / mom.norm();
  free_params[Acts::eFreeDir2]   = mom(2) / mom.norm();
  free_params[Acts::eFreeQOverP] = (q != Acts::ActsScalar(0)) ? (q / p) : (1 / p);

  return free_params;
}

//Pack the acts track parameters into something that is serializable for the event bus

inline std::vector<double> convertActsToLdmxPars(Acts::BoundVector acts_par) {
  
  std::vector<double> v_ldmx(acts_par.data(), acts_par.data() + acts_par.rows() * acts_par.cols());
  return v_ldmx;
}

inline Acts::BoundVector boundState(const ldmx::Track& trk) {
  Acts::BoundVector paramVec;
  paramVec << trk.getD0(),
      trk.getZ0(),
      trk.getPhi(),
      trk.getTheta(),
      trk.getQoP(),
      trk.getT();
  return paramVec;
}

inline Acts::BoundTrackParameters boundTrackParameters(const ldmx::Track& trk,
                                                       std::shared_ptr<Acts::PerigeeSurface> perigee) {
  Acts::BoundVector paramVec  = boundState(trk);
  Acts::BoundSymMatrix covMat = unpackCov(trk.getPerigeeCov());
  return Acts::BoundTrackParameters(perigee, paramVec, std::move(covMat));
}

/*
  for (size_t it = 0; it < ntests_; ++it) {
    
  double d0     = d0Sigma * (*normal_)(generator_);
  double z0     = z0Sigma * (*normal_)(generator_);
  //double d0 = 0.;
  //double z0 = 0.;
    
  double phi    = (*uniform_phi_)(generator_);
  double theta = (*uniform_theta_)(generator_);
  double pt     = pt_ * Acts::UnitConstants::GeV;
  double p      = pt / sin(theta);
  double charge = -1.;
  double qop    = charge / p;
  double t      = 0.;
    
    
  Acts::BoundVector pars;
  d0 = -7.54499;
  z0 = -23.4946;
  phi = 0.0785398;
  theta = 1.5708;
  qop = -0.25;
  t = 0.;
         
  pars << d0, z0, phi, theta, qop, t;

  //std::cout<<"CHECKING TRUTH PARAMETERS"<<std::endl;
  //pars = bound_params;

    
  if (debug_){
  std::cout<<"CHECK START PARAMETERS"<<std::endl;
  std::cout<<pars<<std::endl;
  }
        
  Acts::Vector3 sPosition(0., 0., 0.);
  Acts::Vector3 sMomentum(0., 0., 0.);
    
  //no covariance transport
  auto cov = std::nullopt;
    
        
  // charged extrapolation - with hit recording
  Acts::BoundTrackParameters startParameters(perigee_surface, std::move(pars),
  std::move(cov));
  sPosition = startParameters.position(gctx_);
  sMomentum = startParameters.momentum();

  if (debug_)
  std::cout<<startParameters<<std::endl;
    
  //run the propagator
  PropagationOutput pOutput;
    
  auto result   = propagator_->propagate(startParameters,propagator_options);
    
  if (result.ok()) {
  const auto& resultValue = result.value();
  auto steppingResults =
  resultValue.template get<Acts::detail::SteppingLogger::result_type>();
  // Set the stepping result
  pOutput.first = std::move(steppingResults.steps);
      
  //TODO:: ADD THE MATERIAL INTERACTION
      
  // Record the propagator steps
  propagationSteps.push_back(std::move(pOutput.first));
  }
  else
  std::cout<<"PF::ERROR::PROPAGATION RESULTS ARE NOT OK!!"<<std::endl;
  }//ntests_


  //Only save if you made some tests
  if ( ntests_>0 )
  writer_->WriteSteps(event,propagationSteps);

*/


/* Generation of truth seeds by hand
   auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles")};
   ldmx::SimParticle gen_e = particleMap[1];
  
   Acts::Vector3 gen_e_pos{gen_e.getVertex().data()};
   Acts::Vector3 gen_e_mom{gen_e.getMomentum().data()};
   Acts::ActsScalar  gen_e_time = 0.;
  
   if (hit_collection_ == "RecoilSimHits") {
    
   //Get the Target Scoring plane hits
   const std::vector<ldmx::SimTrackerHit> target_scoring_hits = event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits");

   //Select the hit on the layer right before the recoil tracker

   std::vector<ldmx::SimTrackerHit> sel_scoring_hits;
    
   for (auto & t_sp_hit : target_scoring_hits) {

   //Only select the hits at the last Target scoring plane
   if (t_sp_hit.getPosition()[2] < 4.4)
   continue;

   //Brem/Eloss electrons only
   if (t_sp_hit.getTrackID() != 1)
   continue;

   //Make sure they are electrons
   if (abs(t_sp_hit.getPdgID()) != 11)
   continue;

   sel_scoring_hits.push_back(t_sp_hit);
      
   }
    
   if (sel_scoring_hits.size() != 1) {
   std::cout<<__PRETTY_FUNCTION__<<"::ERROR::Found wrong number of scoring hits. Skipping event..."<<std::endl;
   return;
   }
    
   gen_e_pos(0) = sel_scoring_hits.at(0).getPosition()[0];
   gen_e_pos(1) = sel_scoring_hits.at(0).getPosition()[1];
   gen_e_pos(2) = sel_scoring_hits.at(0).getPosition()[2];


   gen_e_mom(0) = sel_scoring_hits.at(0).getMomentum().at(0);
   gen_e_mom(1) = sel_scoring_hits.at(0).getMomentum().at(1);
   gen_e_mom(2) = sel_scoring_hits.at(0).getMomentum().at(2);
    
   }// Preparing recoil reconstruction

   //TODO:: Avoid copy
   gen_e_pos = tracking::sim::utils::Ldmx2Acts(gen_e_pos);
   gen_e_mom = tracking::sim::utils::Ldmx2Acts(gen_e_mom);

   Acts::ActsScalar q = -1 * Acts::UnitConstants::e;
   Acts::FreeVector free_params = tracking::sim::utils::toFreeParameters(gen_e_pos, gen_e_mom, q);
  
   //if (p < 3.95 * Acts::UnitConstants::GeV)
   //  return;
  
   //The Kalman Filter needs to use bound trackParameters. Express the track parameters with respect the generation point
   //TODO:: This is kind of an approximation of the curvilinear frame at generation. => Update with curvilinear / or propagation to a known surface
   std::shared_ptr<const Acts::PerigeeSurface> gen_surface =
   Acts::Surface::makeShared<Acts::PerigeeSurface>(
   Acts::Vector3(free_params[Acts::eFreePos0], free_params[Acts::eFreePos1], free_params[Acts::eFreePos2]));
  
   //Transform the free parameters to the bound parameters
   auto bound_params =  Acts::detail::transformFreeToBoundParameters(free_params, *gen_surface, gctx_).value();
  
   //The Kalman Filter needs to use bound trackParameters. Express the track parameters with respect the curvilinear surface
  
   //Acts::Vector3 dir{free_params[Acts::eFreeDir0],free_params[Acts::eFreeDir1],free_params[Acts::eFreeDir2]};
   //auto bound_params = Acts::detail::transformFreeToCurvilinearParameters(free_params[Acts::eFreeTime], dir, free_params[Acts::eFreeQOverP]);

  
   Acts::BoundSymMatrix bound_cov = 100. * Acts::BoundSymMatrix::Identity();
   Acts::BoundTrackParameters gen_track_params_bound{gen_surface, bound_params, q, bound_cov};
  
*/
  
}//utils
}//sim
}//tracking
      
      
#endif
