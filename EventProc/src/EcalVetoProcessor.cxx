#include "EventProc/EcalVetoProcessor.h"

// ROOT
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TPython.h"

// LDMX
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"

// C++
#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <cmath>

namespace ldmx {

    const std::vector<double> LAYER_Z_POSITIONS = {223.8000030517578, 226.6999969482422, 233.0500030517578, 237.4499969482422, 245.3000030517578, 251.1999969482422, 260.29998779296875,
        266.70001220703125, 275.79998779296875, 282.20001220703125, 291.29998779296875, 297.70001220703125, 306.79998779296875, 313.20001220703125,
        322.29998779296875, 328.70001220703125, 337.79998779296875, 344.20001220703125, 353.29998779296875, 359.70001220703125, 368.79998779296875,
        375.20001220703125, 384.29998779296875, 390.70001220703125, 403.29998779296875, 413.20001220703125, 425.79998779296875, 435.70001220703125,
        448.29998779296875, 458.20001220703125, 470.79998779296875, 480.70001220703125, 493.29998779296875, 503.20001220703125};

    // arrays holding 68% containment radius per layer for different bins in momentum/angle
    const std::vector<double> radius68_thetalt10_plt500 = {4.045666158618167, 4.086393662224346, 4.359141107602775, 4.666549994726691, 5.8569181911416015, 6.559716356124256, 8.686967529043072, 10.063482736354674, 13.053528344041274, 14.883496407943747, 18.246694748611368, 19.939799900443724, 22.984795944506224, 25.14745829663406, 28.329169392203216, 29.468032123356345, 34.03271241527079, 35.03747443690781, 38.50748727211848, 39.41576583301171, 42.63622296033334, 45.41123601592071, 48.618139095742876, 48.11801717451056, 53.220539860213655, 58.87753380915155, 66.31550881539764, 72.94685877928593, 85.95506228335348, 89.20607201266672, 93.34370253818409, 96.59471226749734, 100.7323427930147, 103.98335252232795};

    const std::vector<double> radius68_thetalt10_pgt500 = {4.081926458777424, 4.099431732299409, 4.262428482867968, 4.362017581473145, 4.831341579961153, 4.998346041276382, 6.2633736512415705, 6.588371889265881, 8.359969947444522, 9.015085558044309, 11.262722588206483, 12.250305471269183, 15.00547660437276, 16.187264014640103, 19.573764900578503, 20.68072032434797, 24.13797140783321, 25.62942209291236, 29.027596514735617, 30.215039667389316, 33.929540248019585, 36.12911729771914, 39.184563500620946, 42.02062468386282, 46.972125628650204, 47.78214816041894, 55.88428562462974, 59.15520134927332, 63.31816666637158, 66.58908239101515, 70.75204770811342, 74.022963432757, 78.18592874985525, 81.45684447449884};

    const std::vector<double> radius68_theta10to20 = {4.0251896715647115, 4.071661598616328, 4.357690094817289, 4.760224640141712, 6.002480766325418, 6.667318981016246, 8.652513285172342, 9.72379373302137, 12.479492693251478, 14.058548828317289, 17.544872909347912, 19.43616066939176, 23.594162859513734, 25.197329065282954, 29.55995803074302, 31.768946746958296, 35.79247330197688, 37.27810357669942, 41.657281051476545, 42.628141392692626, 47.94208483539388, 49.9289473559796, 54.604030254423975, 53.958762417361655, 53.03339560920388, 57.026277390001425, 62.10810455035879, 66.10098633115634, 71.1828134915137, 75.17569527231124, 80.25752243266861, 84.25040421346615, 89.33223137382352, 93.32511315462106};

    const std::vector<double> radius68_thetagt20 = {4.0754238481177705, 4.193693485630508, 5.14209420056253, 6.114996249971468, 7.7376807326481645, 8.551663213602291, 11.129110612057813, 13.106293737495639, 17.186617323282082, 19.970887612094604, 25.04088272634407, 28.853696411302344, 34.72538105333071, 40.21218694947545, 46.07344239520299, 50.074953583805346, 62.944045771758645, 61.145621459396814, 69.86940198299047, 74.82378572939959, 89.4528387422834, 93.18228303096758, 92.51751129204555, 98.80228884380018, 111.17537347472128, 120.89712563907408, 133.27021026999518, 142.99196243434795, 155.36504706526904, 165.08679922962185, 177.45988386054293, 187.18163602489574, 199.55472065581682, 209.2764728201696};



    BDTHelper::BDTHelper(TString importBDTFile) {

        // Import the python packages and load the features into xgboost.
        TPython::Exec("print 'Loading xgboost BDT'"); 
        TPython::Exec("import xgboost as xgb");
        TPython::Exec("import numpy as np");
        TPython::Exec("import pickle as pkl");
        TPython::Exec("model = pkl.load(open('" + importBDTFile + "','r'))");
    }

    void BDTHelper::buildFeatureVector(std::vector<float>& bdtFeatures, ldmx::EcalVetoResult& result) {
        bdtFeatures.push_back(result.getNReadoutHits());
        bdtFeatures.push_back(result.getSummedDet());
        bdtFeatures.push_back(result.getSummedTightIso());
        bdtFeatures.push_back(result.getMaxCellDep());
        bdtFeatures.push_back(result.getShowerRMS());
        bdtFeatures.push_back(result.getXStd());
        bdtFeatures.push_back(result.getYStd());
        bdtFeatures.push_back(result.getAvgLayerHit());
        bdtFeatures.push_back(result.getDeepestLayerHit());
        bdtFeatures.push_back(result.getStdLayerHit());
        for(int ireg = 0; ireg < result.getElectronContainmentEnergy().size(); ++ireg) {
            bdtFeatures.push_back(result.getElectronContainmentEnergy()[ireg]);
        }
        for(int ireg = 0; ireg < result.getPhotonContainmentEnergy().size(); ++ireg) {
            bdtFeatures.push_back(result.getPhotonContainmentEnergy()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentEnergy().size(); ++ireg) {
            bdtFeatures.push_back(result.getOutsideContainmentEnergy()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentNHits().size(); ++ireg) {
            bdtFeatures.push_back(result.getOutsideContainmentNHits()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentXStd().size(); ++ireg) {
            bdtFeatures.push_back(result.getOutsideContainmentXStd()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentYStd().size(); ++ireg) {
            bdtFeatures.push_back(result.getOutsideContainmentYStd()[ireg]);
        }
        bdtFeatures.push_back(result.getEcalBackEnergy());
    }

    float BDTHelper::getSinglePred(std::vector<float> bdtFeatures) {
        TString cmd = vectorToPredCMD(bdtFeatures);
        TPython::Exec("pred = " + cmd);
        float pred = TPython::Eval("pred");
        //std::cout << "  pred = " << pred << std::endl;

        return pred;
    }

    TString BDTHelper::vectorToPredCMD(std::vector<float> bdtFeatures) {
        TString featuresStrVector = "[[";
        for (int i = 0; i < bdtFeatures.size(); i++) {
            featuresStrVector += std::to_string(bdtFeatures[i]);
            if (i < bdtFeatures.size() - 1)
                featuresStrVector += ",";
        }
        featuresStrVector += "]]";
        TString cmd = "float(model.predict(xgb.DMatrix(np.array(" + featuresStrVector + ")))[0])";

        return cmd;
    }

    void EcalVetoProcessor::configure(const ParameterSet& ps) {
        doBdt_ = ps.getInteger("do_bdt");
        if (doBdt_){
            // Config and init the BDT.
            bdtFileName_ = ps.getString("bdt_file");
            if (!std::ifstream(bdtFileName_).good()) {
                EXCEPTION_RAISE("EcalVetoProcessor",
                        "The specified BDT file '" + bdtFileName_ + "' does not exist!");
            }

            BDTHelper_ = new BDTHelper(bdtFileName_);
        }

        cellFileNamexy_ = ps.getString("cellxy_file");
        if (!std::ifstream(cellFileNamexy_).good()) {
            EXCEPTION_RAISE("NonFidEcalVetoProcessor",
                            "The specified x,y cell file '" + cellFileNamexy_ + "' does not exist!");
        } else {
            std::ifstream cellxyfile(cellFileNamexy_);
            float valuex;
            float valuey;
            while ( cellxyfile >> valuex >> valuey) {
                mapsx.push_back(valuex);
                mapsy.push_back(valuey);
            }
        }


        hexReadout_ = new EcalHexReadout();
        nEcalLayers_ = ps.getInteger("num_ecal_layers");

        bdtCutVal_ = ps.getDouble("disc_cut");
        ecalLayerEdepRaw_.resize(nEcalLayers_, 0);
        ecalLayerEdepReadout_.resize(nEcalLayers_, 0);
        ecalLayerTime_.resize(nEcalLayers_, 0);
        cellMap_.resize(nEcalLayers_, std::map<int, float>());
        cellMapTightIso_.resize(nEcalLayers_, std::map<int, float>());

        // Set the collection name as defined in the configuration
        collectionName_ = ps.getString("collection_name"); 
    }

    void EcalVetoProcessor::clearProcessor(){
        for (int i = 0; i < nEcalLayers_; i++) {
            cellMap_[i].clear();
            cellMapTightIso_[i].clear();
        }
        bdtFeatures_.clear();

        nReadoutHits_ = 0;
        summedDet_ = 0;
        summedTightIso_ = 0;
        maxCellDep_ = 0;
        showerRMS_ = 0;
        xStd_ = 0;
        yStd_ = 0;
        avgLayerHit_ = 0;
        stdLayerHit_ = 0;
        deepestLayerHit_ = 0;
        ecalBackEnergy_ = 0;

        std::fill(ecalLayerEdepRaw_.begin(), ecalLayerEdepRaw_.end(), 0);
        std::fill(ecalLayerEdepReadout_.begin(), ecalLayerEdepReadout_.end(), 0);
        std::fill(ecalLayerTime_.begin(), ecalLayerTime_.end(), 0);
    }

    void EcalVetoProcessor::produce(Event& event) {
        result_.Clear();
        clearProcessor();

        // Get the collection of Ecal scoring plane hits. If it doesn't exist,
        // don't bother adding any truth tracking information.

        std::vector<double> recoilP;
        std::vector<float> recoilPos;
        std::vector<double> recoilPAtTarget;
        std::vector<float> recoilPosAtTarget;

        if (event.exists("EcalScoringPlaneHits")) {
            const TClonesArray* ecalSpHits{event.getCollection("EcalScoringPlaneHits")};

            // Loop through all of the sim particles and find the recoil 
            // electron.
            const TClonesArray* simParticles{event.getCollection("SimParticles")};
            SimParticle* recoilElectron{nullptr}; 
            for (int simParticleIndex = 0; simParticleIndex < simParticles->GetEntriesFast();
                    ++simParticleIndex) { 
                SimParticle* particle = static_cast<SimParticle*>(simParticles->At(simParticleIndex)); 

                // We only care about the recoil electron
                if ((particle->getPdgID() == 11) && (particle->getParentCount() == 0)) { 
                    recoilElectron = particle;
                    break;
                } 
            }

            // Find ECAL SP hit for recoil electron
            float pmax = 0;
            for (int ecalSpIndex = 0; ecalSpIndex < ecalSpHits->GetEntriesFast(); ++ecalSpIndex) {
                SimTrackerHit* spHit =  static_cast<SimTrackerHit*>(ecalSpHits->At(ecalSpIndex)); 
                
                if (spHit->getLayerID() != 1 || spHit->getMomentum()[2] <= 0) continue;
                
                SimParticle* spParticle = spHit->getSimParticle();
                if (spParticle == recoilElectron) { 
                    if(sqrt(pow(spHit->getMomentum()[0],2) + pow(spHit->getMomentum()[1],2) + pow(spHit->getMomentum()[2],2)) > pmax) {
                        recoilP = spHit->getMomentum();
                        recoilPos = spHit->getPosition();
                        pmax = sqrt(pow(recoilP[0],2) + pow(recoilP[1],2) + pow(recoilP[2],2));
                    }
                } 
            }

            // Find target SP hit for recoil electron
            if (event.exists("TargetScoringPlaneHits")) {
                const TClonesArray* targetSpHits{event.getCollection("TargetScoringPlaneHits")};
                pmax = 0;
                for (int targetSpIndex = 0; targetSpIndex < targetSpHits->GetEntriesFast(); ++targetSpIndex) {
                    SimTrackerHit* spHit =  static_cast<SimTrackerHit*>(targetSpHits->At(targetSpIndex)); 
                    
                    if (spHit->getLayerID() != 2 || spHit->getMomentum()[2] <= 0) continue;
                    
                    SimParticle* spParticle = spHit->getSimParticle();
                    if (spParticle == recoilElectron) { 
                        if(sqrt(pow(spHit->getMomentum()[0],2) + pow(spHit->getMomentum()[1],2) + pow(spHit->getMomentum()[2],2)) > pmax) {
                            recoilPAtTarget = spHit->getMomentum();
                            recoilPosAtTarget = spHit->getPosition();
                            pmax = sqrt(pow(recoilPAtTarget[0],2) + pow(recoilPAtTarget[1],2) + pow(recoilPAtTarget[2],2));
                        }
                    } 
                }
            }
        }


        // Get projected trajectories for electron and photon
        std::vector<XYCoords> ele_trajectory, photon_trajectory;
        if(recoilP.size() > 0) { 
            ele_trajectory = getTrajectory(recoilP, recoilPos);
            std::vector<double> pvec = recoilPAtTarget.size() ? recoilPAtTarget : std::vector<double>{0.0, 0.0, 0.0};
            std::vector<float>  posvec = recoilPosAtTarget.size() ? recoilPosAtTarget : std::vector<float>{0.0, 0.0, 0.0};
            photon_trajectory = getTrajectory({-recoilPAtTarget[0], -recoilPAtTarget[1], 4000.0-recoilPAtTarget[2]}, recoilPosAtTarget);
        }

        float recoilPMag = recoilP.size() ? sqrt(pow(recoilP[0],2) + pow(recoilP[1],2) + pow(recoilP[2],2)) : -1.0;
        float recoilTheta = recoilPMag > 0 ? recoilP[2]/recoilPMag : -1.0;

        std::vector<double> ele_radii = radius68_thetalt10_plt500;
        if(recoilTheta<10 && recoilPMag >= 500) ele_radii = radius68_thetalt10_pgt500;
        else if(recoilTheta>=10 && recoilTheta<20) ele_radii = radius68_theta10to20;
        else if(recoilTheta>=20) ele_radii = radius68_thetagt20;
        // Use default binning
        std::vector<double> photon_radii = radius68_thetalt10_plt500;


        // Get the collection of digitized Ecal hits from the event. 
        const TClonesArray* ecalDigis = event.getCollection("ecalDigis");
        int nEcalHits = ecalDigis->GetEntriesFast();

        //std::cout << "[ EcalVetoProcessor ] : Got " << nEcalHits << " ECal digis in event "
        //        << event.getEventHeader()->getEventNumber() << std::endl;

        int globalCentroid = GetShowerCentroidIDAndRMS(ecalDigis, showerRMS_);
        /* ~~ Fill the hit map ~~ O(n)  */
        fillHitMap(ecalDigis, cellMap_);
        bool doTight = true;
        /* ~~ Fill the isolated hit maps ~~ O(n)  */
        fillIsolatedHitMap(ecalDigis, globalCentroid, cellMap_, cellMapTightIso_, doTight);

        //Loop over the hits from the event to calculate the rest of the important quantities

        float wavgLayerHit = 0;
        float xMean = 0;
        float yMean = 0;

        // Containment variables
        unsigned int nregions = 5;
        std::vector<float> electronContainmentEnergy (nregions, 0.0);
        std::vector<float> photonContainmentEnergy (nregions, 0.0);
        std::vector<float> outsideContainmentEnergy (nregions, 0.0);
        std::vector<int>   outsideContainmentNHits (nregions, 0);
        std::vector<float> outsideContainmentXmean (nregions, 0.0);
        std::vector<float> outsideContainmentYmean (nregions, 0.0);
        std::vector<float> outsideContainmentXstd (nregions, 0.0);
        std::vector<float> outsideContainmentYstd (nregions, 0.0);
        
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            //Layer-wise quantities
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            ecalLayerEdepRaw_[hit_pair.first] = ecalLayerEdepRaw_[hit_pair.first] + hit->getEnergy();
            if(hit->getLayer() >= 20)
                ecalBackEnergy_ += hit->getEnergy();
            if (maxCellDep_ < hit->getEnergy())
                maxCellDep_ = hit->getEnergy();
            if (hit->getEnergy() > 0) {
                nReadoutHits_++;
                ecalLayerEdepReadout_[hit_pair.first] += hit->getEnergy();
                ecalLayerTime_[hit_pair.first] += (hit->getEnergy()) * hit->getTime();
                xMean += getCellCentroidXYPair(hit_pair.second).first * hit->getEnergy();
                yMean += getCellCentroidXYPair(hit_pair.second).second * hit->getEnergy();
                avgLayerHit_ += hit->getLayer();
                wavgLayerHit += hit->getLayer() * hit->getEnergy();
                if (deepestLayerHit_ < hit->getLayer()) {
                    deepestLayerHit_ = hit->getLayer();
                }
                XYCoords xy_pair = getCellCentroidXYPair(hit_pair.second);
                float distance_ele_trajectory = ele_trajectory.size() ? sqrt( pow((xy_pair.first - ele_trajectory[hit->getLayer()].first),2) + pow((xy_pair.second - ele_trajectory[hit->getLayer()].second),2) ) : -1.0;
                float distance_photon_trajectory = photon_trajectory.size() ? sqrt( pow((xy_pair.first - photon_trajectory[hit->getLayer()].first),2) + pow((xy_pair.second - photon_trajectory[hit->getLayer()].second),2) ) : -1.0;
                // Decide which region a hit goes into and add to sums
                for(unsigned int ireg = 0; ireg < nregions; ireg++) {
                    if(distance_ele_trajectory >= ireg*ele_radii[hit->getLayer()] && distance_ele_trajectory < (ireg+1)*ele_radii[hit->getLayer()])
                        electronContainmentEnergy[ireg] += hit->getEnergy();
                    if(distance_photon_trajectory >= ireg*photon_radii[hit->getLayer()] && distance_photon_trajectory < (ireg+1)*photon_radii[hit->getLayer()])
                        photonContainmentEnergy[ireg] += hit->getEnergy();
                    if(distance_ele_trajectory > (ireg+1)*ele_radii[hit->getLayer()] && distance_photon_trajectory > (ireg+1)*photon_radii[hit->getLayer()]) {
                        outsideContainmentEnergy[ireg] += hit->getEnergy();
                        outsideContainmentNHits[ireg] += 1;
                        outsideContainmentXmean[ireg] += xy_pair.first*hit->getEnergy();
                        outsideContainmentYmean[ireg] += xy_pair.second*hit->getEnergy();
                    }
                }
            }
        }
        
        for (int iLayer = 0; iLayer < ecalLayerEdepReadout_.size(); iLayer++) {
            for (auto cell : cellMapTightIso_[iLayer]) {
                if (cell.second > 0) {
                    summedTightIso_ += cell.second;
                }
            }
            ecalLayerTime_[iLayer] = ecalLayerTime_[iLayer] / ecalLayerEdepReadout_[iLayer];
            summedDet_ += ecalLayerEdepReadout_[iLayer];
        }
        
        if (nReadoutHits_ > 0) {
            avgLayerHit_ /= nReadoutHits_;
            wavgLayerHit /= summedDet_;
            xMean /= summedDet_;
            yMean /= summedDet_;
        } else {
            wavgLayerHit = 0;
            avgLayerHit_ = 0;
            xMean = 0;
            yMean = 0;
        }

        for(unsigned int ireg = 0; ireg < nregions; ireg++) {
            if(outsideContainmentEnergy[ireg] > 0) {
                outsideContainmentXmean[ireg] /= outsideContainmentEnergy[ireg];
                outsideContainmentYmean[ireg] /= outsideContainmentEnergy[ireg];
            }
        }

        // Loop over hits a second time to find the standard deviations.
        for (int iHit = 0; iHit < nEcalHits; iHit++) {
            EcalHit* hit = (EcalHit*) ecalDigis->At(iHit);
            LayerCellPair hit_pair = hitToPair(hit);
            if (hit->getEnergy() > 0) {
                xStd_ += pow((getCellCentroidXYPair(hit_pair.second).first - xMean), 2) * hit->getEnergy();
                yStd_ += pow((getCellCentroidXYPair(hit_pair.second).second - yMean), 2) * hit->getEnergy();
                stdLayerHit_ += pow((hit->getLayer() - wavgLayerHit), 2) * hit->getEnergy();
            }
            XYCoords xy_pair = getCellCentroidXYPair(hit_pair.second);
            float distance_ele_trajectory = ele_trajectory.size() ? sqrt( pow((xy_pair.first - ele_trajectory[hit->getLayer()].first),2) + pow((xy_pair.second - ele_trajectory[hit->getLayer()].second),2) ) : -1.0;
            float distance_photon_trajectory = photon_trajectory.size() ? sqrt( pow((xy_pair.first - photon_trajectory[hit->getLayer()].first),2) + pow((xy_pair.second - photon_trajectory[hit->getLayer()].second),2) ) : -1.0;
            for(unsigned int ireg = 0; ireg < nregions; ireg++) {
                if(distance_ele_trajectory > (ireg+1)*ele_radii[hit->getLayer()] && distance_photon_trajectory > (ireg+1)*photon_radii[hit->getLayer()]) {
                    outsideContainmentXstd[ireg] += pow((xy_pair.first - outsideContainmentXmean[ireg]),2) * hit->getEnergy();
                    outsideContainmentYstd[ireg] += pow((xy_pair.second - outsideContainmentYmean[ireg]),2) * hit->getEnergy();
                }
            }
        }
        
        if (nReadoutHits_ > 0) {
            xStd_ = sqrt (xStd_ / summedDet_);
            yStd_ = sqrt (yStd_ / summedDet_);
            stdLayerHit_ = sqrt (stdLayerHit_ / summedDet_);
        } else {
            xStd_ = 0;
            yStd_ = 0;
            stdLayerHit_ = 0;
        }
        
        for(unsigned int ireg = 0; ireg < nregions; ireg++) {
            if(outsideContainmentEnergy[ireg] > 0) {
                outsideContainmentXstd[ireg] = sqrt(outsideContainmentXstd[ireg]/outsideContainmentEnergy[ireg]);
                outsideContainmentYstd[ireg] = sqrt(outsideContainmentYstd[ireg]/outsideContainmentEnergy[ireg]);
            }
        }

        // end loop over sim hits


        /* Code for fiducial region below */
        
        std::vector<float> faceXY(2);
        
        if (!recoilP.empty() && recoilP[2] != 0) {
            faceXY[0] = ((223.8 - 220.0) * (recoilP[0] / recoilP[2])) + recoilPos[0];
            faceXY[1] = ((223.8 - 220.0) * (recoilP[1] / recoilP[2])) + recoilPos[1];
        } else {
            faceXY[0] = -9999.0;
            faceXY[1] = -9999.0;
        }
        
        int inside = 0;
        int up = 0;
        int step = 0;
        int index;
        float cell_radius = 5.0;
        
        std::vector<float>::iterator it;
        it = std::lower_bound(mapsx.begin(), mapsx.end(), faceXY[0]);
        
        index = std::distance( mapsx.begin(), it);
        
        if (index == mapsx.size()) {
            index += -1;
        }
        
        if (!recoilP.empty() && faceXY[0] != -9999.0) {
            while (true) {
                std::vector<double> dis(2);
                
                dis[0] = faceXY[0] - mapsx[index + step];
                dis[1] = faceXY[1] - mapsy[index + step];
                
                float celldis = sqrt (pow(dis[0],2) + pow(dis[1],2));
                
                if (celldis <= cell_radius) {
                    inside = 1;
                    break;
                }
                
                if ((abs(dis[0]) > 5 && up == 0) || index + step == mapsx.size()-1) {
                    up = 1;
                    step = 0;
                } else if ((abs(dis[0]) > 5 && up == 1) || (index + step == 0 && up == 1)) {
                    break;
                }
                
                if (up == 0) {
                    step += 1;
                } else {
                    step += -1;
                }
            }
        }

        result_.setVariables(nReadoutHits_, deepestLayerHit_, summedDet_, summedTightIso_, maxCellDep_,
            showerRMS_, xStd_, yStd_, avgLayerHit_, stdLayerHit_, ecalBackEnergy_, electronContainmentEnergy, photonContainmentEnergy, outsideContainmentEnergy, outsideContainmentNHits, outsideContainmentXstd, outsideContainmentYstd, ecalLayerEdepReadout_, recoilP, recoilPos);
        
        if (doBdt_) {
            BDTHelper_->buildFeatureVector(bdtFeatures_, result_);
            float pred = BDTHelper_->getSinglePred(bdtFeatures_);
            result_.setVetoResult(pred > bdtCutVal_);
            result_.setDiscValue(pred);
            //std::cout << "  pred > bdtCutVal = " << (pred > bdtCutVal_) << std::endl;
        
            // If the event passes the veto, keep it. Otherwise, 
            // drop the event.
            if (result_.passesVeto() && inside) { 
                setStorageHint(hint_shouldKeep); 
            } else { 
                setStorageHint(hint_shouldDrop);
            }
        }
        
        if (inside) {
            setStorageHint(hint_shouldKeep);
        } else {
            setStorageHint(hint_shouldDrop);
        }
        event.addToCollection(collectionName_, result_);
    }

    EcalVetoProcessor::LayerCellPair EcalVetoProcessor::hitToPair(EcalHit* hit) {
        int detIDraw = hit->getID();
        detID_.setRawValue(detIDraw);
        detID_.unpack();
        int layer = detID_.getFieldValue("layer");
        int cellid = detID_.getFieldValue("cell");
        int moduleid = detID_.getFieldValue("module_position");
        int combinedid = cellid*10+moduleid;
        return (std::make_pair(layer, combinedid));
    }

    /* Function to calculate the energy weighted shower centroid */
    int EcalVetoProcessor::GetShowerCentroidIDAndRMS(const TClonesArray* ecalDigis, double& showerRMS) {
        int nEcalHits = ecalDigis->GetEntriesFast();
        XYCoords wgtCentroidCoords = std::make_pair<float, float>(0., 0.);
        float sumEdep = 0;
        int returnCellId = 1e6;
        //Calculate Energy Weighted Centroid
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);
            CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
            XYCoords centroidCoords = getCellCentroidXYPair(hit_pair.second);
            wgtCentroidCoords.first = wgtCentroidCoords.first + centroidCoords.first * cell_energy_pair.second;
            wgtCentroidCoords.second = wgtCentroidCoords.second + centroidCoords.second * cell_energy_pair.second;
            sumEdep += cell_energy_pair.second;
        }
        wgtCentroidCoords.first = (sumEdep > 1E-6) ? wgtCentroidCoords.first / sumEdep : wgtCentroidCoords.first;
        wgtCentroidCoords.second = (sumEdep > 1E-6) ? wgtCentroidCoords.second / sumEdep : wgtCentroidCoords.second;
        //Find Nearest Cell to Centroid
        float maxDist = 1e6;
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);
            XYCoords centroidCoords = getCellCentroidXYPair(hit_pair.second);

            float deltaR = pow(pow((centroidCoords.first - wgtCentroidCoords.first), 2) + pow((centroidCoords.second - wgtCentroidCoords.second), 2), .5);
            showerRMS += deltaR * hit->getEnergy();
            if (deltaR < maxDist) {
                maxDist = deltaR;
                returnCellId = hit_pair.second;
            }
        }
        if (sumEdep > 0)
            showerRMS = showerRMS / sumEdep;
        return returnCellId;
    }

                /* Function to load up empty vector of hit maps */
    void EcalVetoProcessor::fillHitMap(const TClonesArray* ecalDigis,
            std::vector<std::map<int, float>>& cellMap_) {
        int nEcalHits = ecalDigis->GetEntriesFast();
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(
                    hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);

            CellEnergyPair cell_energy_pair = std::make_pair(
                    hit_pair.second, hit->getEnergy());
            cellMap_[hit_pair.first].insert(cell_energy_pair);
        }
    }

    void EcalVetoProcessor::fillIsolatedHitMap(const TClonesArray* ecalDigis, float globalCentroid,
            std::vector<std::map<int, float>>& cellMap_, std::vector<std::map<int, float>>& cellMapIso_, bool doTight) {
        int nEcalHits = ecalDigis->GetEntriesFast();
        for (int hitCounter = 0; hitCounter < nEcalHits; ++hitCounter) {
            std::pair<bool, int> isolatedHit = std::make_pair(true, 0);
            EcalHit* hit = static_cast<EcalHit*>(ecalDigis->At(hitCounter));
            LayerCellPair hit_pair = hitToPair(hit);
            if (doTight) {
                //Disregard hits that are on the centroid.
                if (hit_pair.second == globalCentroid)
                    continue;

                //Skip hits that are on centroid inner ring
                if (isInShowerInnerRing(globalCentroid, hit_pair.second)) {
                    continue;
                }
            }

            //Skip hits that have a readout neighbor
            std::vector<int> cellNbrIds = getInnerRingCellIds(hit_pair.second);

            //Get neighboring cell id's and try to look them up in the full cell map (constant speed algo.)
            for (int k = 0; k < 6; k++) {
                std::map<int, float>::iterator it = cellMap_[hit_pair.first].find(cellNbrIds[k]);
                if (it != cellMap_[hit_pair.first].end()) {
                    isolatedHit = std::make_pair(false, cellNbrIds[k]);
                    break;
                }
            }
            if (!isolatedHit.first) {
                continue;
            }
            //Insert isolated hit
            CellEnergyPair cell_energy_pair = std::make_pair(hit_pair.second, hit->getEnergy());
            cellMapIso_[hit_pair.first].insert(cell_energy_pair);
        }
    }

    // Calculate where trajectory intersects ECAL layers using position and momentum at scoring plane
    std::vector<std::pair<float,float> > EcalVetoProcessor::getTrajectory(std::vector<double> momentum, std::vector<float> position) {
        std::vector<XYCoords> positions;
        for(int iLayer = 0; iLayer < nEcalLayers_; iLayer++) {
            float posX = position[0] + (momentum[0]/momentum[2])*(LAYER_Z_POSITIONS[iLayer] - position[2]);
            float posY = position[1] + (momentum[1]/momentum[2])*(LAYER_Z_POSITIONS[iLayer] - position[2]);
            positions.push_back(std::make_pair(posX, posY));
        }
        return positions;
    }

}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
