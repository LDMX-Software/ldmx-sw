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
//#include "Acts/Propagator/Navigator.hpp"
//#include "Acts/Propagator/StandardAborters.hpp"

// Geometry
#include "Acts/Surfaces/PerigeeSurface.hpp"

//Root
#include "TH1F.h"
#include "TFile.h"
#include "TLorentzVector.h"

//Propagator with void navigator
using VoidPropagator = Acts::Propagator<Acts::EigenStepper<>>;

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
    
  ///Destructor
  ~VertexProcessor();

  /**
   *
   */
  void onProcessStart() final override;

  /**
   *
   */
  void onProcessEnd() final override;

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Run the processor
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event);


 private:
  ///The contexts - TODO: they should move to some global location, I guess
  Acts::GeometryContext gctx_;
  Acts::MagneticFieldContext bctx_;
  
  bool debug_{false};

  //Event counter
  int nevents_{0};
  
  //The interpolated bfield
  std::shared_ptr<InterpolatedMagneticField3> sp_interpolated_bField_;
  std::string bfieldMap_;

  //Track collection name

  std::string trk_coll_name_{"Tracks"};

  //The propagator
  std::shared_ptr<VoidPropagator> propagator_;

  //Processing time counter
  double processing_time_{0.};


  TH1F* h_m_;
  TH1F* h_m_truthFilter_;
  TH1F* h_m_truth_;
  
};

                  
} // namespace reco
} // namespace tracking

#endif // TRACKING_RECO_VERTEXPROCESSOR_H_
