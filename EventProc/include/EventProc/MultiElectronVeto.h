/**
 * @file MultiElectronVeto.h
 * @brief Class that computes feature vars for multi electron PN discrimination
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENTPROC_MULTIELECTRONVETO_H_
#define EVENTPROC_MULTIELECTRONVETO_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include <stdlib.h>

//----------//
//   LDMX   //
//----------//
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"
#include "Event/MultiEleVetoResult.h"
#include "Event/SimCalorimeterHit.h"
#include "Event/SimTrackerHit.h"
#include "Framework/EventProcessor.h"

//----------//
//   ROOT   //
//----------//
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TPython.h"

namespace ldmx {

    class MultiElectronVeto: public Producer {

        public:

            /** Constructor */
            MultiElectronVeto(const std::string& name, Process& process);

            /** Destructor */
            ~MultiElectronVeto(); 

            /** Read in user-specified parameters. */
            void configure(const ParameterSet&);
    
            /** Run the multi-electron veto. */
            void produce(Event& event);

            /** 
             * Get the HCal scoring plane hits associated with all recoil
             * electrons.
             *
             * @param simParticles Collection of SimParticles in the event
             * @param hcalSPHits Collection of HCal scoring plane hits
             *
             * @return Collection of HCal scoring plane hits associted with 
             *         all recoil electrons present in the event.
             *
             */
            std::vector<SimTrackerHit*> getRecoilElectronHcalSPHits(const TClonesArray* simParticles, const TClonesArray* HCalSPHits); 
           
            /** 
             * Get all recoil electron in the event.
             *
             * @param Collection of SimParticles in the event
             *
             * @return Collection of recoil electrons in the event
             */ 
            std::vector<SimParticle*> getRecoilElectrons(const TClonesArray* simParticles);
            
            typedef std::pair<float, float> XYCoords;

        private:

            void Debug(std::string output){ 
                if( verbose_ )
                    std::cout << " [ MultiElectronVeto ] : " << output << std::endl;
            };

            template <class T> void Debug(std::string output_label,std::vector<T> output_value){ 
                if( verbose_ ){
                    std::cout << " [ MultiElectronVeto ] : " << output_label ;
                    for( int i = 0 ; i < output_value.size() ; i++ ){
                        std::cout << output_value[i] << ", " ;
                    }
                    std::cout << std::endl;
                }
            };

            /** */
            EcalHexReadout* hexReadout_{new EcalHexReadout{}};

            /** Object used to persist veto variables and results. */
            MultiEleVetoResult result_;

            /** Flag denoting whether verbose output is enabled/disabled. */
            bool verbose_{false};

            /** The Moliere radius. */
            const double moliere_r{25./* mm */};

            /** 
             * The z position of each of the ECal layers. 
             * TODO: These positions should eventually come from the geometry.
             */
            std::vector<double> layer_z{
                223.8, // Layer 1
                226.7, // Layer 2
                233.05, // Layer 3
                237.45, // Layer 4
                245.3, // Layer 5
                251.2, // Layer 6
                260.3, // Layer 7
                266.7, // Layer 8
                275.8, // Layer 9
                282.2, // Layer 10
                291.3, // Layer 11
                297.7, // Layer 12
                306.8, // Layer 13
                313.2, // Layer 14
                322.3, // Layer 15
                337.8, // Layer 16
                344.2, // Layer 17
                353.3, // Layer 18
                359.7, // Layer 19
                368.8, // Layer 20
                375.2, // Layer 21
                384.3, // Layer 22
                390.7, // Layer 23
                403.3, // Layer 24
                413.2, // Layer 25
                425.8, // Layer 26
                435.7, // Layer 27
                448.3, // Layer 28
                458.2, // Layer 29
                470.8, // Layer 30
                480.7, // Layer 31
                493.3, // Layer 32
                503.2 // Layer 33
            };
    };

    bool compareSimTrackerHits(SimTrackerHit* a, SimTrackerHit* b){

        std::vector<double> b_mom_vec = a->getMomentum();
        std::vector<double> a_mom_vec = b->getMomentum();
        double a_mom = sqrt(pow(a_mom_vec[0],2)+pow(a_mom_vec[1],2)+pow(a_mom_vec[2],2));
        double b_mom = sqrt(pow(b_mom_vec[0],2)+pow(b_mom_vec[1],2)+pow(b_mom_vec[2],2));
        return a_mom < b_mom ;
    }
}

#endif
