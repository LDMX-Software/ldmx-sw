/**
 * @file MultiElectronVeto.h
 * @brief Class that computes feature vars for multi electron PN discrimination
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENTPROC_MULTIELECTRONVETO_H_
#define EVENTPROC_MULTIELECTRONVETO_H_

// std lib
#include <string>

// ROOT
#include "TString.h"
#include "TFile.h"
#include "TTree.h"

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"
#include "Event/MultiEleVetoResult.h"
#include "Event/SimTrackerHit.h"
#include "Framework/EventProcessor.h"

namespace ldmx {


    bool compareSimTrackerHits(SimTrackerHit* a, SimTrackerHit* b){
      
        std::vector<double> b_mom_vec = a->getMomentum();
	std::vector<double> a_mom_vec = b->getMomentum();
	double a_mom = sqrt(pow(a_mom_vec[0],2)+pow(a_mom_vec[1],2)+pow(a_mom_vec[2],2));
	double b_mom = sqrt(pow(b_mom_vec[0],2)+pow(b_mom_vec[1],2)+pow(b_mom_vec[2],2));
	return a_mom < b_mom ;
    }

    /**
     * @class MultiElectronVeto
     * @brief compute features for multi electron PN discrimination
     */
    class MultiElectronVeto: public Producer {

        public:

            typedef std::pair<float, float> XYCoords;

            MultiElectronVeto(const std::string& name, Process& process) :
                    Producer(name, process) {
            }

            virtual ~MultiElectronVeto() {
	      delete hexReadout_;
            }
	    
 	    std::vector<SimTrackerHit*> getRecoilElectrons(Event& event);
	    
            void configure(const ParameterSet&);

            void produce(Event& event);

	    void clearProcessor();

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

	    bool verbose_{false};
	    const double moliere_r{25.};
            EcalHexReadout* hexReadout_{nullptr};
	    MultiEleVetoResult result_;
	    
	    std::vector<double> layer_z{223.8,226.7,233.05,
		                        237.45,245.3,251.2,
		                        260.3,266.7,275.8,
		                        282.2,291.3,297.7,
		                        306.8,313.2,322.3,
		                        337.8,344.2,353.3,
		                        359.7,368.8,375.2,
		                        384.3,390.7,403.3,
		                        413.2,425.8,435.7,
    		                        448.3,458.2,470.8,
		                        480.7,493.3,503.2};
    };

}

#endif
