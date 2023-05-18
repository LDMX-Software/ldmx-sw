#include "CustomStatePropagator.h"

#include "Acts/Utilities/Logger.hpp"


#include <iostream>

namespace tracking {
namespace reco {

CustomStatePropagator::CustomStatePropagator(const std::string&name, framework::Process& process)
    : framework::Producer(name,process) {
  
}

CustomStatePropagator::~CustomStatePropagator() {}


void CustomStatePropagator::onProcessStart() {


  outFile_ = new TFile("prop_states.root","RECREATE");
  outTree_ = new TTree("prop_states","prop_states");
  
  outTree_->Branch("state_nr",&state_nr);
  outTree_->Branch("charge",&charge);
  outTree_->Branch("gen_x",&gen_x);
  outTree_->Branch("gen_y",&gen_y);
  outTree_->Branch("gen_z",&gen_z);
  outTree_->Branch("gen_px",&gen_px);
  outTree_->Branch("gen_py",&gen_py);
  outTree_->Branch("gen_pz",&gen_pz);

  outTree_->Branch("end_x",&end_x);
  outTree_->Branch("end_y",&end_y);
  outTree_->Branch("end_z",&end_z);
  outTree_->Branch("end_loc0",&end_loc0);
  outTree_->Branch("end_loc1",&end_loc1);
  
  outTree_->Branch("end_px",&end_px);
  outTree_->Branch("end_py",&end_py);
  outTree_->Branch("end_pz",&end_pz);

  
  // TODO:: Move this to an external file
  auto localToGlobalBin_xyz = [](std::array<size_t, 3> bins,
                                 std::array<size_t, 3> sizes) {
    return (bins[0] * (sizes[1] * sizes[2]) + bins[1] * sizes[2] +
            bins[2]);  // xyz - field space
    // return (bins[1] * (sizes[2] * sizes[0]) + bins[2] * sizes[0] + bins[0]);
    // //zxy
  };

  
  // Setup a interpolated bfield map
  const auto map = std::make_shared<InterpolatedMagneticField3>(
      makeMagneticFieldMapXyzFromText(
          std::move(localToGlobalBin_xyz), field_map_,
          1. * Acts::UnitConstants::mm,    // default scale for axes length
          1000. * Acts::UnitConstants::T,  // The map is in kT, so scale it to T
          false,                           // not symmetrical
          true                             // rotate the axes to tracking frame
                                      ));
  
  const auto stepper = Acts::EigenStepper<>{map};

  // Set up propagator with void navigator (No material)
  auto propagator = std::make_shared<Acts::Propagator<Acts::EigenStepper<>>>(stepper);
  
  Acts::Vector3 gen_pos{0., 0., 0.};
  Acts::Vector3 gen_mom{0., 0., 0.};
  
  std::default_random_engine generator;
  generator.seed(1);

  //Beamspot
  std::uniform_real_distribution<double> bY(-bs_size_[0],bs_size_[0]);
  std::uniform_real_distribution<double> bX(-bs_size_[1],bs_size_[1]);

  //3-momentum in cartesian coordinates
  std::uniform_real_distribution<double> PX(-4, 4);
  std::uniform_real_distribution<double> PY(-4, 4);
  std::uniform_real_distribution<double> PZ(0.0, 4);


  //3-momentum in polar coordinates
  std::uniform_real_distribution<double> P(prange_[0], prange_[1]);
  std::uniform_real_distribution<double> THETA(thetarange_[0], thetarange_[1]);
  std::uniform_real_distribution<double> PHI(phirange_[0], phirange_[1]);

  std::uniform_real_distribution<double> CHARGE(-1,1);
    
  for (int i_state = 0; i_state < nstates_; i_state++){

    double p = P(generator);
    double theta = THETA(generator);
    double phi = PHI(generator);
    int charge = CHARGE(generator) > 0 ? 1 : -1;

    double px = p * cos(theta);
    double py = p * sin(theta) * cos(phi);
    double pz = p * sin(theta) * sin(phi);
    
    double by = bY(generator);
    double bx = bX(generator);

    gen_pos(0) = 0.;
    gen_pos(1) = bx;
    gen_pos(2) = by;

    // Transform to MeV because that's what TrackUtils assumes
    gen_mom(0) = px / Acts::UnitConstants::MeV;
    gen_mom(1) = py / Acts::UnitConstants::MeV;
    gen_mom(2) = pz / Acts::UnitConstants::MeV;

    //std::cout<<"Generated position"<<std::endl;
    //std::cout<<gen_pos<<std::endl;
    //std::cout<<std::endl;
    //std::cout<<"Generated momentum"<<std::endl;
    //std::cout<<gen_mom<<std::endl;

    Acts::ActsScalar q = charge * Acts::UnitConstants::e;

    Acts::FreeVector part_free =
        tracking::sim::utils::toFreeParameters(gen_pos, gen_mom, q);

        
    // perigee on the track
    std::shared_ptr<const Acts::PerigeeSurface> gen_surface =
        Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(
            part_free[Acts::eFreePos0], part_free[Acts::eFreePos1],
            part_free[Acts::eFreePos2]));


    // Transform the free parameters to the bound parameters
    auto bound_params = Acts::detail::transformFreeToBoundParameters(
                            part_free, *gen_surface, gctx_)
                            .value();

    Acts::BoundTrackParameters startParams(
        gen_surface, std::move(bound_params), std::move(std::nullopt));
    

    //const auto pLogger = Acts::getDefaultLogger("Propagator", Acts::Logging::INFO);
    Acts::PropagatorOptions<> propagator_options(
        gctx_, bctx_);//, Acts::LoggerWrapper{*pLogger});

    propagator_options.direction = Acts::NavigationDirection::Forward; // should be the default
    propagator_options.pathLimit = std::numeric_limits<double>::max();
    propagator_options.loopProtection = true;
    propagator_options.maxStepSize = 1 * Acts::UnitConstants::mm;
    propagator_options.maxSteps = 2000;


    //Define the target surface - be careful:
    // x - downstream
    // y - left (when looking along x)
    // z - up
    // Passing identity here means that your target surface is oriented in the same way
    Acts::RotationMatrix3 surf_rotation = Acts::RotationMatrix3::Zero();
    //u direction along +Y
    surf_rotation(1,0) = 1;
    //v direction along +Z
    surf_rotation(2,1) = 1;
    //w direction along +X
    surf_rotation(0,2) = 1;

    Acts::Vector3 pos(surf_location_, 0., 0.);
    Acts::Translation3 surf_translation(pos);
    Acts::Transform3 surf_transform(surf_translation * surf_rotation);

    //Unbounded surface
    const std::shared_ptr<Acts::PlaneSurface> target_surface =
        Acts::Surface::makeShared<Acts::PlaneSurface>(surf_transform);

    
    const Acts::BoundTrackParameters* endParams = nullptr;

    //Do the propagation to the surface

    //auto result = propagator->propagate(startParams,*target_surface, propagator_options);

    //if (result.ok()) {
    //  endParams = (*result).endParameters.get();
    //  fillTree(i_state, q, gen_pos, gen_mom, endParams);
      
    //}
    //else
     // continue;
    
    //loc0 // loc1 will give you the u-v location of the hit on the ecal face
    
  }//state propagation
  
  
}//on Process Start

void CustomStatePropagator::configure(framework::config::Parameters& parameters) {

  const double PIo2 = 1.57079632679;
      
  field_map_     = parameters.getParameter<std::string>("field_map", "");
  surf_location_ = parameters.getParameter<double>("surf_location",350.);
  nstates_       = parameters.getParameter<int>("nstates",10);
  bs_size_       = parameters.getParameter<std::vector<double>>("bs_size",{40.,10.});
  prange_        = parameters.getParameter<std::vector<double>>("prange",{0.05,4.});
  thetarange_    = parameters.getParameter<std::vector<double>>("thetarange",{0,PIo2});
  phirange_      = parameters.getParameter<std::vector<double>>("phirange",{0,PIo2 * 4.});
  
}


void CustomStatePropagator::fillTree(int state,
                                     int q,
                                     const Acts::Vector3 gen_pos,
                                     const Acts::Vector3 gen_mom,
                                     const Acts::BoundTrackParameters* endParams) {

  state_nr = state;
  charge  = q;
  gen_x = gen_pos(0);
  gen_y = gen_pos(1);
  gen_z = gen_pos(2);

  gen_px = gen_mom(0);
  gen_py = gen_mom(1);
  gen_pz = gen_mom(2);

  Acts::Vector3 end_pos = endParams->position(gctx_);
  end_x  = end_pos(0);
  end_y  = end_pos(1);
  end_z  = end_pos(2);

  Acts::BoundVector bound_parameters = endParams->parameters();
  end_loc0 = bound_parameters[Acts::eBoundLoc0];
  end_loc1 = bound_parameters[Acts::eBoundLoc1];
  
  Acts::Vector3 end_mom = endParams->momentum();
  end_px = end_mom(0);
  end_py = end_mom(1);
  end_pz = end_mom(2);
  
  outTree_->Fill();
}


void CustomStatePropagator::onProcessEnd() {

  outFile_->cd();
  outTree_->Write();
  outFile_->Close();
  
}


}//namespace reco
}//namespace tracking


DECLARE_PRODUCER_NS(tracking::reco, CustomStatePropagator)
