#ifndef TRACKING_RECO_VERTEXPROCESSOR_H_
#define TRACKING_RECO_VERTEXPROCESSOR_H_

//--- Framework ---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

// --- Tracking --- //
#include "Tracking/Event/Track.h"
#include "Tracking/Sim/BFieldXYZUtils.h"
#include "Tracking/Sim/TrackingUtils.h"

// --- ACTS --- //

// Vertexing

#include "Acts/Vertexing/FullBilloirVertexFitter.hpp"
#include "Acts/Vertexing/HelicalTrackLinearizer.hpp"
#include "Acts/Vertexing/Vertex.hpp"

// Magfield

#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/MagneticField/MagneticFieldProvider.hpp"

// Propagator

#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
// #include "Acts/Propagator/Navigator.hpp"
// #include "Acts/Propagator/StandardAborters.hpp"

// Geometry
#include "Acts/Surfaces/PerigeeSurface.hpp"

// Particle Data
#include "Acts/Definitions/ParticleData.hpp"

// Root
#include "TFile.h"
#include "TH1F.h"
#include "Math/GenVector/LorentzVector.h"
// Propagator with void navigator
using VoidPropagator = Acts::Propagator<Acts::EigenStepper<>>;
using  LorentzVector = ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>>;
namespace tracking {
  namespace reco {
    
    class VertexProcessor : public framework::Producer {
    public:
      /**
       * Constructor.
       *
       * @param name The name of the instance of this object.
       * @param process The process running this producer.
       */
      
      VertexProcessor(const std::string &name, framework::Process &process);

      /// Destructor
      virtual ~VertexProcessor() = default;
      
      void onProcessStart() override;
      void onProcessEnd() override;
      
      /**
       * Configure the processor using the given user specified parameters.
       *
       * @param parameters Set of parameters used to configure this processor.
       */
      void configure(framework::config::Parameters &parameters) override;
      
      /**
       * Run the processor
       *
       * @param event The event to process.
       */
      void produce(framework::Event &event) override;
      /*
      struct InputTrack{
	InputTrack(const Acts::BoundTrackParameters& params):  m_parameters(params){}; 
	const Acts::BoundTrackParameters& parameters() const {return m_parameters; };

	int charge(){return m_parameters.charge();};
      private:
	Acts::BoundTrackParameters m_parameters;
      };
      */
      //      std::function<Acts::BoundTrackParameters(Acts::InputTrack)> extractParameters = [](Acts::InputTrack params){return params.parameters();};
    private:
      /// The contexts - TODO: they should move to some global location, I guess
      Acts::GeometryContext gctx_;
      Acts::MagneticFieldContext bctx_;
      
      // Event counter
      int nevents_{0};
      
      // The interpolated bfield
      std::shared_ptr<InterpolatedMagneticField3> sp_interpolated_bField_;
      
      /// Path to the magnetic field map.
      std::string field_map_{""};
      
      // Track collection name
      
      std::string trk_coll_name_{"Tracks"};
       // The output track collection
      std::string out_vtx_collection_{"Vertices"};
      std::string input_pass_name_{""};
      
      // The propagator
      std::shared_ptr<VoidPropagator> propagator_;
      
      // Processing time counter
      double processing_time_{0.};
      
      TH1F *h_m_;
      TH1F *h_m_truthFilter_;
      TH1F *h_m_truth_;

      ldmx::Vertex::FittedTrack makeFittedTrack(Acts::TrackAtVertex trkOnVtx,int pdgId=0){
	ldmx::Vertex::FittedTrack fittrk; 
	Acts::BoundTrackParameters fitPars=trkOnVtx.fittedParams;
	fittrk.momentum=tracking::sim::utils::Acts2LdmxStdVec(fitPars.momentum());
	fittrk.params=tracking::sim::utils::convertActsToLdmxPars(fitPars.parameters());
	const Acts::BoundMatrix& trk_cov = *fitPars.covariance();
	tracking::sim::utils::flatCov(trk_cov, fittrk.cov);     
	fittrk.chiSqContrib=trkOnVtx.chi2Track;
	fittrk.distToVtx=trkOnVtx.vertexCompatibility;
	fittrk.pdgId=pdgId; 
	return fittrk;
      };   
      double calculateFittedInvariantMass(ldmx::Vertex vtx){
	LorentzVector pTot;
	std::vector<ldmx::Vertex::FittedTrack> fittrks=vtx.getFittedTracks();
	for(int iFt=0; iFt<fittrks.size(); iFt++){
	  std::vector<double> mom=fittrks.at(iFt).momentum;
	  int pdgId=fittrks.at(iFt).pdgId;
	  //make an Acts::PdgParticle
	  Acts::PdgParticle  pdgPart{pdgId};
	  double mass=-666.666; 
	  if(Acts::findMass(pdgPart).has_value()){
	    mass=Acts::findMass(pdgPart).value();	    
	  }
	  double E=sqrt( mom[0]*mom[0]+mom[1]*mom[1]+mom[2]*mom[2]+mass*mass); 
	  pTot+=LorentzVector{mom[0], mom[1], mom[2], E}; 
	}	
	return  pTot.mass(); 
      };
      
    };  // class Vertex    
  }  // namespace reco
}  // namespace tracking

#endif  // TRACKING_RECO_VERTEXPROCESSOR_H_
