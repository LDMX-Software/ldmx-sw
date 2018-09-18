/**
 * @file MultiElectronVeto.cxx
 * @brief Class that computes feature vars for multi electron PN discrimination
 * @author Andrew Whitbeck, FNAL
 */

#include "EventProc/MultiElectronVeto.h"

namespace ldmx {

    MultiElectronVeto::MultiElectronVeto(const std::string &name, Process &process):
        Producer(name, process) { 
    }

    MultiElectronVeto::~MultiElectronVeto() {
        delete hexReadout_; 
    }

    void MultiElectronVeto::configure(const ParameterSet& ps) {
        verbose_ = ps.getBool("verbose",false);
    }

    std::vector<SimParticle*> MultiElectronVeto::getRecoilElectrons(const TClonesArray* simParticles) { 
        
        // Loop through all the sim particles and search for the recoil electrons 
        // i.e. electrons which don't have any parents
        std::vector<SimParticle*> recoils; 
        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); 
                ++particleCount) { 
            
            // Get the ith particle from the collection of sim particles
            SimParticle* particle = static_cast<SimParticle*>(simParticles->At(particleCount)); 

            // If the particle is an electron and has no parents, a recoil has 
            // been found
            if ((particle->getPdgID() == 11) && (particle->getParentCount() == 0)) { 
                recoils.push_back(particle);
            }
        }

        // All events should contain at least 1 recoil electron
        if (recoils.size() == 0) { 
            throw std::runtime_error("Event doesn't contain any recoil electrons!");  
        }

        return recoils; 
    }

    std::vector<SimTrackerHit*> MultiElectronVeto::getRecoilElectronHcalSPHits(
            const TClonesArray* simParticles, const TClonesArray* spHits) { 
    
        // Collection of recoil electron HCal hits
        std::vector<SimTrackerHit*> hits;

        // Start by getting all recoil electrons in the event
        std::vector<SimParticle*> recoils = this->getRecoilElectrons(simParticles); 

        // Loop over the recoil electrons and find the associated HCal scoring 
        // plane hit.
        for (const auto& recoil : recoils) { 
            
            // Loop through all the HCal scoring plane hits and find the 
            // one associated with the recoil electron.  Note: There may be 
            // multiple hits associated with a recoil electron.  In this case,
            // take the one that has the highest momentum and a positive pz 
            // component.  Also, only consider hits produced in the front 
            // HCal scoring plane.
            double pMax = 0; 
            SimTrackerHit* recoilHCalSPHit = nullptr; 
            for( int iHit = 0 ; iHit < spHits->GetEntriesFast() ; iHit++ ) {
                
                // Get the ith hit in the collection
                SimTrackerHit* hit = (SimTrackerHit*) spHits->At(iHit);
               
                    // Only consider hits associated with recoil electrons 
                    if (hit->getSimParticle() == recoil) { 
                        
                        // Make sure the hit wasn't created by a backwards going 
                        // recoil. 
                        if (hit->getMomentum()[2] <= 0) continue;
                        
                        // Select the hit that has the highest momentum
                        std::vector<double> pVec = hit->getMomentum();
                        double p = sqrt(pVec[0]*pVec[0] + pVec[1]*pVec[1] + pVec[2]*pVec[2]); 
                        if (pMax < p) { 
                            pMax = p; 
                            recoilHCalSPHit = hit;     
                        }     
                    }
            }

            hits.push_back(recoilHCalSPHit); 
        }
    
    return hits;  
    //std::vector<SimTrackerHit*> MultiElectronVeto::getRecoilElectrons(Event& event){
    //    const TClonesArray* simParticles = event.getCollection("SimParticles");
    //    const TClonesArray* spHits = event.getCollection("HcalScoringPlaneHits");
        
        /*
        std::vector<SimTrackerHit*> recoil_electrons;
        std::sort(recoil_electrons.begin(),recoil_electrons.end(),compareSimTrackerHits);
        for( int iPar = 0 ; iPar < simParticles->GetEntriesFast() ; iPar++ ){
            SimParticle* simPar = (SimParticle*) simParticles->At(iPar);
            //simPar->Print();
            SimTrackerHit* recoil_electron=NULL;
            if( simPar->getGenStatus()==1 and simPar->getPdgID()==11 ){
                float max_hit_mom = 0.;
                for( int iHit = 0 ; iHit < spHits->GetEntriesFast() ; iHit++ ){
                    SimTrackerHit* hit = (SimTrackerHit*) spHits->At(iHit);
                    std::vector<double> hit_mom_3vec = hit->getMomentum();
                    double hit_mom = sqrt(pow(hit_mom_3vec[0],2)+
                            pow(hit_mom_3vec[1],2)+
                            pow(hit_mom_3vec[2],2));
                    if( hit->getSimParticle() == simPar && max_hit_mom < hit_mom ){
                        max_hit_mom = hit_mom;
                        recoil_electron = hit;
                    }
                }// end loop over scoring plane hits

            }// end if block for beam electrons
            recoil_electrons.push_back(recoil_electron);
        }// end loop over sim particles

        return recoil_electrons;*/
    }

    void MultiElectronVeto::produce(Event& event) {
        
        // Clear the previous result.
        result_.Clear(); 

        // Get the collection of SimParticles from the event
        const TClonesArray* simParticles = event.getCollection("SimParticles");

        // Check for the existence of the collection of HCal scoring plane hits.
        // If it does exist, use it to get the scoring plane position of the 
        // recoils at the ECal/HCal face.  Otherwise, just use the ECal scoring
        // plane hits.
        const TClonesArray* spHits = nullptr; 
        if (event.exists("HcalScoringPlaneHits")) { 
            // Get the collection of HCal scoring planes hits from the event.
            spHits = event.getCollection("HcalScoringPlaneHits");
        } else { 
            spHits = event.getCollection("EcalScoringPlaneHits");  
        }

        // Get the collection of digitized ECal hits from the event.
        const TClonesArray* ecalDigis = event.getCollection("ecalDigis");

        // Get the HCal scoring plane hits associated with all recoil electrons 
        // in the event.
        std::vector<SimTrackerHit*> recoils = this->getRecoilElectronHcalSPHits(simParticles, spHits);

        // Loop over all of the recoil SP hits and calcualate the veto variables.
        for(const auto& electron : recoils) {
            
            if (!electron) continue;

            result_.addElectron();

            std::vector<double> pvec = electron->getMomentum();
            Debug("electron momentum: ",pvec);
            
            std::vector<float> position = electron->getPosition();    
            Debug("electron position: ",position);

            // Get the collection of digitized Ecal hits from the event. 
            int nEcalHits = ecalDigis->GetEntriesFast();

            std::vector<double> cylinder_0_1(34,0.);
            std::vector<double> cylinder_1_3(34,0.);
            std::vector<double> cylinder_3_5(34,0.);
            std::vector<double> cylinder_5(34,0.);

            // loop over hits and collect them into cylinder sums
            for (int iHit = 0; iHit < nEcalHits; iHit++) {

                EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
                
                //hit->Print();
                //int layer = hit->getLayer();
                int rawID = hit->getID();
                EcalDetectorID detId;
                detId.setRawValue(rawID);
                detId.unpack();
                int layer = detId.getFieldValue("layer");
                int cellID = detId.getFieldValue("cell");
                int moduleID = detId.getFieldValue("module_position");
                int combinedID = cellID*10+moduleID;
                //std::cout << " [ MultiElectronVeto ] : rawid " << rawID << " layer " << layer << " cellID " << cellID << " moduleID " << moduleID << " combineid " << combinedID << std::endl;
                std::pair<double,double> hitPosition = hexReadout_->getCellCenterAbsolute(combinedID);
                //std::cout << " [ MultiElectronVeto ] : hit position: " << hitPosition.first << " " << hitPosition.second << std::endl;

                std::vector<double> relativePosition(2,0.);
                std::vector<double> rayPosition(2,0.);
                rayPosition[0] = position[0]+pvec[0]/pvec[2]*(layer_z[layer]-position[2]);
                rayPosition[1] = position[1]+pvec[1]/pvec[2]*(layer_z[layer]-position[2]);
                Debug("ray position: ",rayPosition);
                relativePosition[0] = hitPosition.first - rayPosition[0];
                relativePosition[1] = hitPosition.second - rayPosition[1];
                Debug("relative position: ",relativePosition);


                double r = sqrt(pow(relativePosition[0],2)+pow(relativePosition[1],2));

                Debug("grouping hits by radius");
                if( r < moliereR_ ){
                    cylinder_0_1[layer]+=hit->getEnergy();
                }else if( r < 3*moliereR_ ){
                    cylinder_1_3[layer]+=hit->getEnergy();
                }else if( r < 5*moliereR_ ){
                    cylinder_3_5[layer]+=hit->getEnergy();
                }else{
                    cylinder_5[layer]+=hit->getEnergy();
                }

            }// end loop over hits

            Debug("grouping layer 0");
            result_.cylinder_0_1_layer_0_0.back() = cylinder_0_1[0];
            result_.cylinder_1_3_layer_0_0.back() = cylinder_1_3[0];
            result_.cylinder_3_5_layer_0_0.back() = cylinder_3_5[0];
            result_.cylinder_5_layer_0_0.back()   = cylinder_5[0];
            Debug("grouping layer 1-2");
            for( int i = 1 ; i < 3 ; i++ ){
                result_.cylinder_0_1_layer_1_2.back() += cylinder_0_1[i];
                result_.cylinder_1_3_layer_1_2.back() += cylinder_1_3[i];
                result_.cylinder_3_5_layer_1_2.back() += cylinder_3_5[i];
                result_.cylinder_5_layer_1_2.back()   += cylinder_5[i];
            }
            Debug("grouping layer 3-6");
            for( int i = 3 ; i < 7 ; i++ ){
                result_.cylinder_0_1_layer_3_6.back() += cylinder_0_1[i];
                result_.cylinder_1_3_layer_3_6.back() += cylinder_1_3[i];
                result_.cylinder_3_5_layer_3_6.back() += cylinder_3_5[i];
                result_.cylinder_5_layer_3_6.back()   += cylinder_5[i];
            }
            Debug("grouping layer 7-14");
             for( int i = 7 ; i < 15 ; i++ ){
                result_.cylinder_0_1_layer_7_14.back() += cylinder_0_1[i];
                result_.cylinder_1_3_layer_7_14.back() += cylinder_1_3[i];
                result_.cylinder_3_5_layer_7_14.back() += cylinder_3_5[i];
                result_.cylinder_5_layer_7_14.back()   += cylinder_5[i];
            }
            Debug("grouping layer 15 and beyond");
                //std::cout << "cylinder sizes: " << cylinder_0_1.size() << " " << cylinder_1_3.size() << " " << cylinder_3_5.size() << " " << cylinder_5.size() << std::endl;
            for( int i = 15 ; i < 33 ; i++ ){
                 result_.cylinder_0_1_layer_15.back() += cylinder_0_1[i];
                result_.cylinder_1_3_layer_15.back() += cylinder_1_3[i];
                result_.cylinder_3_5_layer_15.back() += cylinder_3_5[i];
                result_.cylinder_5_layer_15.back()   += cylinder_5[i];
            }
            Debug("done grouping by layer");
        }

        Debug("done, adding information to event");
        event.addToCollection("MultiElectronVeto", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, MultiElectronVeto);
