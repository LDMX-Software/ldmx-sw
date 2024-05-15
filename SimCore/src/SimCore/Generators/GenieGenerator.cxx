/**
 * @file GenieGenerator.cxx
 * @brief Simple GENIE event generator.
 * @author Wesley Ketchum, FNAL
 */

#include "SimCore/Generators/GenieGenerator.h"
#include "SimCore/UserPrimaryParticleInformation.h"
#include "SimCore/Event/GTruth.h"

// GENIE
#include "Framework/Utils/AppInit.h"
#include "GENIE/Framework/Utils/RunOpt.h"
#include "Framework/Utils/XSecSplineList.h"
#include "GENIE/Framework/Interaction/InitialState.h"
#include "Framework/EventGen/EventRecord.h"
#include "Framework/GHEP/GHepParticle.h"


#include "Framework/Conventions/XmlParserStatus.h"
#include "Framework/Conventions/GBuild.h"
#include "Framework/Conventions/Controls.h"
#include "Framework/Conventions/Units.h"
#include "Framework/EventGen/EventRecord.h"
#include "Framework/EventGen/GFluxI.h"
#include "Framework/EventGen/GEVGDriver.h"
#include "Framework/EventGen/GMCJDriver.h"
#include "Framework/EventGen/GMCJMonitor.h"
#include "Framework/EventGen/InteractionList.h"
#include "Framework/Interaction/Interaction.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Ntuple/NtpWriter.h"
#include "Framework/Ntuple/NtpMCFormat.h"
#include "Framework/Numerical/RandomGen.h"
#include "Framework/Numerical/Spline.h"
#include "Framework/ParticleData/PDGCodes.h"
#include "Framework/ParticleData/PDGUtils.h"
#include "Framework/Utils/AppInit.h"
#include "Framework/Utils/RunOpt.h"
#include "Framework/Utils/XSecSplineList.h"
#include "Framework/Utils/StringUtils.h"
#include "Framework/Utils/PrintUtils.h"
#include "Framework/Utils/SystemUtils.h"
#include "Framework/Utils/CmdLnArgParser.h"

// Geant4
#include "G4Event.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"


// ROOT
#include <TParticle.h>
#include <TVector3.h>
#include <TLorentzVector.h>

// standard
#include <iostream>
#include <sstream>
#include <algorithm>

namespace simcore {
namespace generators {

void GenieGenerator::fillConfig(const framework::config::Parameters& p)
{

  energy_      = p.getParameter<double>("energy");// * GeV;

  targets_     = p.getParameter< std::vector<int> >("targets");
  abundances_  = p.getParameter< std::vector<double> >("abundances");

  time_        = p.getParameter<double>("time");// * ns;
  position_    = p.getParameter< std::vector<double> >("position"); // mm
  beam_size_   = p.getParameter< std::vector<double> >("beam_size"); // mm
  direction_   = p.getParameter< std::vector<double> >("direction");
  target_thickness_ = p.getParameter<double>("target_thickness"); //mm

  tune_        = p.getParameter<std::string>("tune");
  spline_file_ = p.getParameter<std::string>("spline_file");

  message_threshold_file_ = p.getParameter<std::string>("message_threshold_file");

  verbosity_   = p.getParameter<int>("verbosity");
  
}

bool GenieGenerator::validateConfig()
{
  bool ret=true;
  
  if (targets_.size()==0 || abundances_.size()==0)
    {
      std::cout << "targets and/or abundances sizes are zero."
		<< "  " << targets_.size() << ", " << abundances_.size()
		<< std::endl;
      ret=false;
    }
  if (targets_.size() != abundances_.size())
    {
      std::cout << "targets and abundances sizes unequal."
		<< "  " << targets_.size() << " != " << abundances_.size()
		<< std::endl;
      ret=false;
    }

  if (position_.size()!=3 || direction_.size()!=3)
    {
      std::cout << "position and/or direction sizes are not 3."
		<< "  " << position_.size() << ", " << direction_.size()
		<< std::endl;
      ret=false;
    }

  if(target_thickness_ < 0)
    {
      std::cout << "target thickness cannot be less than 0. " << target_thickness_
		<< std::endl;
      std::cout << "Taking absolute value." << std::endl;
      target_thickness_ = std::abs(target_thickness_);
    }

  if(beam_size_.size()!=2)
    {
      if (beam_size_.size()==0){
	std::cout << "beam size not set. Using zero."
		  << std::endl;
	beam_size_.resize(2); beam_size_[0] = 0.0; beam_size_[1] = 0.0;
      }
      else{
	std::cout << "beam size is set, but does not have size 2."
		  << " " << beam_size_.size()
		  << std::endl;
	ret=false;
      }
    }
  else if(beam_size_[0] < 0 || beam_size_[1] < 0 )
    {
      std::cout << "Beam size set as negative value? "
		<< "(" << beam_size_[0] << "," << beam_size_[1] << ")"
		<< std::endl;
      std::cout << "Changing to positive." << std::endl;
      beam_size_[0] = std::abs(beam_size_[0]);
      beam_size_[1] = std::abs(beam_size_[1]);      
    }

  //normalize abundances
  double abundance_sum=0;
  for( auto a : abundances_){
    abundance_sum+=a;
  }
  
  if( std::abs(abundance_sum)<1e-6)
    {
      std::cout << "abundances list sums to zero? " << abundance_sum << std::endl;
      ret=false;
    }
  
  if( std::abs(abundance_sum-1.0)>2e-2)
    {
      std::cout << "abundances list sums is not unity (" << abundance_sum << " instead.)" << std::endl;
      std::cout << "Will renormalize abundances to unity!" << std::endl;
    }

  for(size_t i_a; i_a<abundances_.size(); ++i_a){
    abundances_[i_a] = abundances_[i_a]/abundance_sum;

    if(verbosity_>0)
      std::cout << "Target=" << targets_[i_a]
		<< ", Abundance=" << abundances_[i_a]
		<< std::endl;
  }

  
  double dir_total_sq=0;
  for( auto d : direction_)
    dir_total_sq += d*d;
  
  if( dir_total_sq<1e-6)
    {
      std::cout << "direction vector is zero or negative? "
		<< "(" << direction_[0] << "," << direction_[1] << "," << direction_[2] << ")"
		<< std::endl;
      ret=false;
    }
  for(size_t i_d; i_d<direction_.size(); ++i_d)
    direction_[i_d] = direction_[i_d]/std::sqrt(dir_total_sq);
  

  xsec_by_target_.resize(targets_.size(),-999.);
  n_events_by_target_.resize(targets_.size(),0);
  
  return ret;
}

void GenieGenerator::initializeGENIE()
{
  //initialize some RunOpt by hacking the command line interface
  {
    char *inarr[3] = {const_cast<char*>(""),
		      const_cast<char*>("--event-generator-list"),
		      const_cast<char*>("EM")};
    genie::RunOpt::Instance()->ReadFromCommandLine(3,inarr);
  }

  //set message thresholds
  genie::utils::app_init::MesgThresholds(message_threshold_file_);

  //set tune info
  genie::RunOpt::Instance()->SetTuneName(tune_);
  if ( ! genie::RunOpt::Instance()->Tune() ) {
    EXCEPTION_RAISE("ConfigurationException","No TuneId in RunOption.");
  }
  genie::RunOpt::Instance()->BuildTune();

  /*
  //set random seed
  auto seed = G4Random::getTheEngine()->getSeed();
  if(verbosity_>=1)
    std::cout << "Initializing GENIE with seed " << seed << std::endl;
  genie::utils::app_init::RandGen(seed);
  */
  
  //give it the splint file and require it
  genie::utils::app_init::XSecTable(spline_file_, true);

  //set GHEP print level (needed?)
  genie::GHepRecord::SetPrintLevel(0);

  //setup for event driver
  evg_driver_.SetEventGeneratorList(genie::RunOpt::Instance()->EventGeneratorList());
  evg_driver_.SetUnphysEventMask(*genie::RunOpt::Instance()->UnphysEventMask());
  
}

void GenieGenerator::calculateTotalXS()
{
  //initializing...
  xsec_total_ = 0;
  ev_weighting_integral_.resize(targets_.size(),0.0);
  
  //calculate the total xsec per target...
  for(size_t i_t=0; i_t<targets_.size(); ++i_t){
    
    genie::InitialState initial_state(targets_[i_t],11);
    evg_driver_.Configure(initial_state);
    evg_driver_.UseSplines();
    
    //setup the initial election
    TParticle initial_e;
    initial_e.SetPdgCode(11);
    auto elec_i_p = std::sqrt(energy_*energy_ - initial_e.GetMass()*initial_e.GetMass());
    initial_e.SetMomentum(elec_i_p*direction_[0],
			  elec_i_p*direction_[1],
			  elec_i_p*direction_[2],
			  energy_);
    TLorentzVector e_p4;
    initial_e.Momentum(e_p4);
    
    
    xsec_by_target_[i_t] = evg_driver_.XSecSum(e_p4);
    xsec_total_ += xsec_by_target_[i_t]*abundances_[i_t];

    ev_weighting_integral_[i_t] = xsec_total_; //running sum
    
    //print...
    std::cout << "Target=" << targets_[i_t]
	      << "\tAbundance=" << abundances_[i_t]
	      << "\tXSEC=" << xsec_by_target_[i_t]/genie::units::millibarn << " mb"
	      << std::endl;

  }
  std::cout << "Total XSEC = " << xsec_total_ / genie::units::millibarn << " mb" << std::endl;

  //renormalize our weighting integral
  for(size_t i_t=0; i_t<ev_weighting_integral_.size(); ++i_t)
    ev_weighting_integral_[i_t] = ev_weighting_integral_[i_t]/xsec_total_;

}
  
  
GenieGenerator::GenieGenerator(const std::string& name,
			       const framework::config::Parameters& p)
    : PrimaryGenerator(name, p)
{
  
  fillConfig(p);

  if(!validateConfig())
    EXCEPTION_RAISE("ConfigurationException","Configuration not valid.");

  n_events_generated_ = 0;
  initializeGENIE();
  calculateTotalXS();
  
}

GenieGenerator::~GenieGenerator()
{
  std::cout << "--- GENIE Generation Summary BEGIN ---" << std::endl;
  double total_xsec = 0;
  for(size_t i_t=0; i_t<targets_.size(); ++i_t)
    {
      std::cout << "Target=" << targets_[i_t]
		<< "\tAbundance=" << abundances_[i_t]
		<< "\tXSEC=" << xsec_by_target_[i_t]/genie::units::millibarn << " mb"
		<< "\tEvents=" << n_events_by_target_[i_t]
		<< std::endl;
      if(n_events_by_target_[i_t]>0)
	total_xsec += xsec_by_target_[i_t]*abundances_[i_t];
    }

  std::cout << "Total events generated = " << n_events_generated_ << std::endl;
  std::cout << "Total XSEC = " << total_xsec / genie::units::millibarn << " mb" << std::endl;

  std::cout << "--- GENIE Generation Summary *END* ---" << std::endl;
}
  
void GenieGenerator::GeneratePrimaryVertex(G4Event* event)
{

  if( n_events_generated_==0){
    //set random seed
    //have to do this here since seeds aren't properly set until we know the run number
    auto seed = G4Random::getTheEngine()->getSeed();
    if(verbosity_>=1)
      std::cout << "Initializing GENIE with seed " << seed << std::endl;
    genie::utils::app_init::RandGen(seed);
  }
  
  auto nucl_target_i = 0;

  if(targets_.size()>0)
    {
      double rand_uniform = G4Random::getTheGenerator()->flat();

      nucl_target_i = std::distance(ev_weighting_integral_.begin(),
				    std::lower_bound(ev_weighting_integral_.begin(),
						     ev_weighting_integral_.end(),
						     rand_uniform) );
      if(verbosity_>=1)
	std::cout << "Random number = " << rand_uniform
		  << ", target picked " << targets_.at(nucl_target_i) << std::endl;
      
    }

  auto x_pos = position_[0] + (G4Random::getTheGenerator()->flat()-0.5)*beam_size_[0];
  auto y_pos = position_[1] + (G4Random::getTheGenerator()->flat()-0.5)*beam_size_[1];
  auto z_pos = position_[2] + (G4Random::getTheGenerator()->flat()-0.5)*target_thickness_;
  if(verbosity_>=1)
    std::cout << "Generating interaction at (x,y,z)="
	      << "(" << x_pos << "," << y_pos << "," << z_pos << ")"
	      << std::endl;
  
  genie::InitialState initial_state(targets_.at(nucl_target_i),11);
  evg_driver_.Configure(initial_state);
  evg_driver_.UseSplines();

  //setup the initial election
  TParticle initial_e;
  initial_e.SetPdgCode(11);
  double elec_i_p = std::sqrt(energy_*energy_ - (double)initial_e.GetMass()*(double)initial_e.GetMass());
  initial_e.SetMomentum(elec_i_p*direction_[0],
			elec_i_p*direction_[1],
			elec_i_p*direction_[2],
			energy_);
  TLorentzVector e_p4;
  initial_e.Momentum(e_p4);
  if(verbosity_>=1)
    std::cout << "Generating interation with (px,py,pz,e)="
	      << "(" << e_p4.Px() << "," << e_p4.Py() << "," << e_p4.Pz() << "," << e_p4.E() << ")"
	      << std::endl;

  //calculate total xsec
  if( n_events_by_target_[nucl_target_i]==0)
    xsec_by_target_[nucl_target_i] = evg_driver_.XSecSum(e_p4);
  
  n_events_by_target_[nucl_target_i]+=1;

  
  //GENIE magic
  genie::EventRecord *genie_event = NULL;
  while(!genie_event)
    genie_event = evg_driver_.GenerateEvent(e_p4);

  auto genie_info = new UserEventInformation;
  genie_info->setGENIEEventRecord(genie_event);
  event->SetUserInformation(genie_info);
  
  //setup the primary vertex now
  
  G4PrimaryVertex* vertex = new G4PrimaryVertex();
  vertex->SetPosition(x_pos,
		      y_pos,
		      z_pos);
  vertex->SetWeight(genie_event->Weight());
  
  //loop over the entries and add to the G4Event
  int nentries = genie_event->GetEntries();

  if(verbosity_>=1){
    std::cout << "---------- "
	      << "Generated Event " << n_events_generated_+1
	      << " ----------"
	      << std::endl;
  }
  
  for(int i_p=0; i_p<nentries; ++i_p)
    {
      genie::GHepParticle *p = (genie::GHepParticle*)(*genie_event)[i_p];

      //make sure it's a final state particle
      if (p->Status()!=1)
	continue;

      if(verbosity_>=1)
	std::cout << "\tAdding particle " << p->Pdg() << " with status " << p->Status() << " energy " << p->E() << " ..." << std::endl;

      G4PrimaryParticle* primary = new G4PrimaryParticle();
      primary->SetPDGcode(p->Pdg());
      primary->Set4Momentum(p->Px()*CLHEP::GeV, p->Py()*CLHEP::GeV, p->Pz()*CLHEP::GeV, p->E()*CLHEP::GeV);
      primary->SetProperTime(time_*CLHEP::ns);

      UserPrimaryParticleInformation* primaryInfo =
	new UserPrimaryParticleInformation();
      primaryInfo->setHepEvtStatus(1);
      primary->SetUserInformation(primaryInfo);
      
      vertex->SetPrimary(primary);
    }

  //add the vertex to the event
  event->AddPrimaryVertex(vertex);
  
  ++n_events_generated_;
  //delete genie_event;
  
}

void GenieGenerator::RecordConfig(const std::string& id, ldmx::RunHeader& rh) {
  rh.setStringParameter(id+" Class", "simcore::generators::GenieGenerator");
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators::GenieGenerator)
