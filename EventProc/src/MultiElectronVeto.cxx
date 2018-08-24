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

    std::vector<SimParticle*> MultiElectronVeto::getRecoilElectrons(TClonesArray* simParticles) { 
        
        // Loop through all the sim particles and search for the recoil electrons 
        // i.e. electrons which don't have any parents
        std::vector<SimParticle*> recoils; 
        for (int particleCount = 0; particleCount < simParticles->GetEntriesFast(); 
                ++particleCount) { 
            
            // Get the ith particle from the collection of sim particles
            SimParticle* particle = static_cast<SimParticle*>(simParticles->At(particleCount)); 

            // If the particle is an electron and has no parents, a recoil has 
            // been found
            if (particle->getPdgID() && (particle->getParentCount() == 0)) { 
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
            const TClonesArray* simParticles, const TClonesArray* hcal_SPhits) { 
    
    //std::vector<SimTrackerHit*> hits;

    
    //std::vector<SimTrackerHit*> MultiElectronVeto::getRecoilElectrons(Event& event){
    //    const TClonesArray* simParticles = event.getCollection("SimParticles");
    //    const TClonesArray* hcal_SPhits = event.getCollection("HcalScoringPlaneHits");

        std::vector<SimTrackerHit*> recoil_electrons;
        std::sort(recoil_electrons.begin(),recoil_electrons.end(),compareSimTrackerHits);
        for( int iPar = 0 ; iPar < simParticles->GetEntriesFast() ; iPar++ ){
            SimParticle* simPar = (SimParticle*) simParticles->At(iPar);
            //simPar->Print();
            SimTrackerHit* recoil_electron=NULL;
            if( simPar->getGenStatus()==1 and simPar->getPdgID()==11 ){
                float max_hit_mom = 0.;
                for( int iHit = 0 ; iHit < hcal_SPhits->GetEntriesFast() ; iHit++ ){
                    SimTrackerHit* hit = (SimTrackerHit*) hcal_SPhits->At(iHit);
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

        return recoil_electrons;
    }

    void MultiElectronVeto::produce(Event& event) {
        
        // Clear the previous result.
        result_.Clear(); 

        const TClonesArray* simParticles = event.getCollection("SimParticles");
        const TClonesArray* hcal_SPhits = event.getCollection("HcalScoringPlaneHits");
        //std::vector<SimTrackerHit*> recoil_electrons = getRecoilElectrons(event);
        std::vector<SimTrackerHit*> recoil_electrons = getRecoilElectronHcalSPHits(simParticles, hcal_SPhits);
        for( auto electron : recoil_electrons ){
            if( electron ){
                result_.addElectron();
                std::vector<double> electron_mom = electron->getMomentum();
                Debug("electron momentum: ",electron_mom);
                std::vector<float> electron_pos = electron->getPosition();
                Debug("electron position: ",electron_pos);

                // Get the collection of digitized Ecal hits from the event. 
                const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
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
                    int raw_id = hit->getID();
                    EcalDetectorID detId;
                    detId.setRawValue(raw_id);
                    detId.unpack();
                    int layer = detId.getFieldValue("layer");
                    int cellid = detId.getFieldValue("cell");
                    int moduleid = detId.getFieldValue("module_position");
                    int combinedid = cellid*10+moduleid;
                    //std::cout << " [ MultiElectronVeto ] : rawid " << raw_id << " layer " << layer << " cellid " << cellid << " moduleid " << moduleid << " combineid " << combinedid << std::endl;
                    std::pair<double,double> hit_pos = hexReadout_->getCellCenterAbsolute(combinedid);
                    //std::cout << " [ MultiElectronVeto ] : hit position: " << hit_pos.first << " " << hit_pos.second << std::endl;

                    std::vector<double> rel_pos(2,0.);
                    std::vector<double> ray_pos(2,0.);
                    ray_pos[0] = electron_pos[0]+electron_mom[0]/electron_mom[2]*(layer_z[layer]-electron_pos[2]);
                    ray_pos[1] = electron_pos[1]+electron_mom[1]/electron_mom[2]*(layer_z[layer]-electron_pos[2]);
                    Debug("ray position: ",ray_pos);
                    rel_pos[0] = hit_pos.first - ray_pos[0];
                    rel_pos[1] = hit_pos.second - ray_pos[1];
                    Debug("relative position: ",rel_pos);


                    double r = sqrt(pow(rel_pos[0],2)+pow(rel_pos[1],2));

                    Debug("grouping hits by radius");
                    if( r < moliere_r ){
                        cylinder_0_1[layer]+=hit->getEnergy();
                    }else if( r < 3*moliere_r ){
                        cylinder_1_3[layer]+=hit->getEnergy();
                    }else if( r < 5*moliere_r ){
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

            }// end if block for non-NULL trackerHit

        }// end for loop for recoil electrons

        Debug("done, adding information to event");
        event.addToCollection("MultiElectronVeto", result_);
    }
}

DECLARE_PRODUCER_NS(ldmx, MultiElectronVeto);
