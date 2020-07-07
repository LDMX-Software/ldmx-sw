/**
 * @file PnWeightProcessor.cxx
 * @brief Processor that calculates an event weight based on the kinematics of 
 *        Photonuclear event.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @note
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

#include "EventProc/PnWeightProcessor.h"
#include "Framework/Exception.h"

namespace ldmx {

    const int PnWeightProcessor::PROTON_PDGID = 2212;

    const int PnWeightProcessor::NEUTRON_PDGID = 2112; 

    PnWeightProcessor::PnWeightProcessor(const std::string &name, Process &process) :
        Producer(name, process) {

            lFit = std::make_unique<TF1>( "lfit" , "exp([1]-[0]*x)" , 950 , 1150 );
            lFit->SetParameters(0.01093, 2.766); 

            std::string func = "exp([1]-[0]*x)*([2]+[3]*x + [4]*pow(x,2)"; 
            func += " + [5]*pow(x,3) + [6]*pow(x,4) + [7]*pow(x,5) + [8]*pow(x,6))"; 
            hFit = std::make_unique<TF1>("hfit", func.c_str(), 1150, 3700);
            hFit->SetParameters(0.004008, 11.23, 1.242e-6, -1.964e-9, 
                    8.243e-13, 9.333e-17, -7.584e-20, -1.991e-23, 9.757e-27);
    }

    PnWeightProcessor::~PnWeightProcessor() { 
    }

    void PnWeightProcessor::configure(Parameters& parameters) {
        wThreshold_ = parameters.getParameter< double >("w_threshold");
        thetaThreshold_ = parameters.getParameter< double >("theta_threshold");
    }

    void PnWeightProcessor::produce(Event& event) {
        
        // Get the particle map from the event.  If the particle map is empty,
        // don't process the event.
        auto particleMap{event.getMap<int,SimParticle>("SimParticles")};
        if (particleMap.size() == 0) return; 

        // Get the recoil electron
        auto [ trackID, recoil ] = Analysis::getRecoil(particleMap);

        // Use the recoil electron to retrieve the gamma that underwent a 
        // photo-nuclear reaction.
        auto pnGamma{Analysis::getPNGamma(particleMap, recoil, 2500.)};

        // For PN biased events, there should always be a gamma that
        // underwent a PN reaction.
        if (pnGamma == nullptr) {
            EXCEPTION_RAISE( "PnWeightProc" , "Event doesn't contain a PN Gamma that is the daughter of the recoil electron." );
        }

        double hardestNucleonKe = -9999;
        double hardestNucleonTheta = -9999;
        double hardestNucleonW = -9999; 
        double highestWNucleonKe = -9999;
        double highestWNucleonTheta = -9999;
        double highestWNucleonW = -9999; 
        PnWeightResult result;
        for (const int &daughterTrackID : pnGamma->getDaughters() ) { 
           
            // Get a daughter of the PN gamma 
            const SimParticle *pnDaughter;
            if ( particleMap.count( daughterTrackID ) > 0 ) {
                pnDaughter = &( particleMap.at( daughterTrackID ) );
            } else {
                //pnDaughter not stored in particle map
                EXCEPTION_RAISE(
                        "MissingPNDaughter",
                        "PN Daughter with track ID " + std::to_string(daughterTrackID) +
                        " has not been stored. Have you stored all of the daughters of PN interactions?"
                        );
            }

            // Calculate the kinetic energy
            double ke = (pnDaughter->getEnergy() - pnDaughter->getMass());

            // Calculate the momentum
            double px = pnDaughter->getMomentum()[0];
            double py = pnDaughter->getMomentum()[1];
            double pz = pnDaughter->getMomentum()[2];
            double p = sqrt(px*px + py*py + pz*pz); 

            // Calculate the polar angle
            double theta = acos(pz/p)*180.0/3.14159;

            // Get the PDG ID of the daughter
            long int pdgID = std::abs(pnDaughter->getPdgID());

            double w = this->calculateW(pnDaughter); 

            // Check if the daughter particle is a proton or neutron
            if ((pdgID == PROTON_PDGID) || (pdgID == NEUTRON_PDGID)) { 

                // Add the W of the current nucleon to the inclusive collection.
                result.addW(w);
                
                // Add the theta of the current nucleon to the inclusive collection. 
                result.addTheta(theta); 

                // Find the nucleon with the greatest kinetic energy   
                if (ke > hardestNucleonKe) { 
                    hardestNucleonKe = ke;
                    hardestNucleonTheta = theta;
                    hardestNucleonW = w;  
                }
                
                // Find the nucleon with the highest W 
                if (w > highestWNucleonW) { 
                    highestWNucleonKe = ke;
                    highestWNucleonTheta = theta;
                    highestWNucleonW = w;  
                }
            }//pnDaughter is nucleon
        }//loop through pnDaughters
       
        // If the W of the highest W nucleon is above the threshold and the
        // polar angle is above the angle threshold, reweight the event. 
        // Otherwise, set the event weight to 1. 
        double eventWeight = 1; 
        if ((highestWNucleonW > wThreshold_) && (highestWNucleonTheta > thetaThreshold_)) {
            eventWeight = calculateWeight(highestWNucleonW);   
        }

        result.setHardestNucleonKe(hardestNucleonKe); 
        result.setHardestNucleonTheta(hardestNucleonTheta); 
        result.setHardestNucleonW(hardestNucleonW); 
        result.setHighestWNucleonKe(highestWNucleonKe); 
        result.setHighestWNucleonTheta(highestWNucleonTheta); 
        result.setHighestWNucleonW(highestWNucleonW); 
        result.setWeight(eventWeight); 

        // Add the result to the collection    
        event.add("PNweight", result);
    }

    double PnWeightProcessor::calculateWeight(double w) {
        return lFit->Eval(w)/hFit->Eval(w); 
    }

    double PnWeightProcessor::calculateW(const SimParticle* particle, double delta) {
        double px = particle->getMomentum()[0];
        double py = particle->getMomentum()[1];
        double pz = particle->getMomentum()[2];
        double p = sqrt(px*px + py*py + pz*pz); 
        double ke = particle->getEnergy() - particle->getMass();

        return 0.5*(p + ke)*(sqrt(1 + (delta*delta)) - delta*(pz/p));
    }
}

DECLARE_PRODUCER_NS(ldmx, PnWeightProcessor)
