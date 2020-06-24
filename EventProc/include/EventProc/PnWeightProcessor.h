/**
 * @file PnWeightProcessor.cxx
 * @brief Processor that calculates an event weight based on the kinematics of 
 *        Photonuclear event.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @note
 * This processor is not used anymore and is only left here for documentation purposes.
 *
 * PnWeightProcessor calculates an event weight which is persisted as a PnWeight
 * object. This weight is based on the kinematics of a photonuclear reaction and 
 * is intended to correct for the overproduction of events with high-momentum,
 * backwards-going nucleons.  A weight is assigned to the event if the variable
 *      W = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p))
 *          p_tot = sqrt(K^2 + 2*K*m),
 *          K = kinetic energy of nucleon at PN vertex
 *          p, p_z = momentum, z-component of nucleon at PN vertex
 * is above some threshold and the hardest nucleon in the event has a polar 
 * angle > 100 degrees.  The W variable is calculated using the hardest 
 * nucleon in the event.
 */

#ifndef EVENTPROC_PNWEIGHTPROCESSOR_H_
#define EVENTPROC_PNWEIGHTPROCESSOR_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory>

//----------//
//   LDMX   //
//----------//
#include "Tools/AnalysisUtils.h"
#include "Event/PnWeightResult.h"
#include "Event/SimParticle.h"
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

//----------//
//   ROOT   //
//----------//
#include "TTree.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"

namespace ldmx {

    class PnWeightProcessor : public Producer {

        public:
            
            //---------------//
            //   Constants   //
            //---------------//

            /** Proton PDG ID */
            static const int PROTON_PDGID; 

            /** Neutron PDG ID */
            static const int NEUTRON_PDGID; 

            /** Constructor */
            PnWeightProcessor(const std::string& name, Process& process);

            /** Destructor */
            ~PnWeightProcessor();

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this processor.
             */
            void configure(Parameters& parameters) final override;

            /** Run the weight calculation and create a pnWeightResult. */
            void produce(Event& event);

            /** Calculate the event weight. */
            double calculateWeight(double w); 

            /**
             * Calculate the measured W defined as
             *     W(measured) = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p)) 
             * where 
             *     p is the total momentum of the particle
             *     K is its kinetic energy
             *     p_z is the z component of the momentum
             * all defined at the hardest PN vertex.
             *
             * @param particle SimParticle used to calculate W.
             * @return W
             */
            double calculateW(const SimParticle* particle, double delta = 0.5);

        private:
    
            /** Threshold after which to apply W reweighting. */
            double wThreshold_{1150 /* MeV */};

            /** Minimum angle for backwards-going hadron. */
            double thetaThreshold_{100 /* degrees */};
            
            /** Fit to lower slope of inclusive W (theta > 100) plot. */
            std::unique_ptr<TF1> lFit;

            /** Fit to high tail of inclusive W (theta > 100) plot. */
            std::unique_ptr<TF1> hFit;
                
    };
}

#endif
