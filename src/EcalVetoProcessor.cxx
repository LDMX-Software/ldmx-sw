#include "Ecal/EcalVetoProcessor.h"

// LDMX
#include "Event/EcalHit.h"
#include "Event/EventConstants.h"

/*~~~~~~~~~~~*/
/*   Tools   */
/*~~~~~~~~~~~*/
#include "Tools/AnalysisUtils.h"

// C++
#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <cmath>

// ROOT (MIP tracking)
#include "TMatrixD.h"
#include "TDecompSVD.h"
#include "TVector3.h"

namespace ldmx {

    // arrays holding 68% containment radius per layer for different bins in momentum/angle
    const std::vector<double> radius68_thetalt10_plt500 = {4.045666158618167, 4.086393662224346, 4.359141107602775, 4.666549994726691, 5.8569181911416015, 6.559716356124256, 8.686967529043072, 10.063482736354674, 13.053528344041274, 14.883496407943747, 18.246694748611368, 19.939799900443724, 22.984795944506224, 25.14745829663406, 28.329169392203216, 29.468032123356345, 34.03271241527079, 35.03747443690781, 38.50748727211848, 39.41576583301171, 42.63622296033334, 45.41123601592071, 48.618139095742876, 48.11801717451056, 53.220539860213655, 58.87753380915155, 66.31550881539764, 72.94685877928593, 85.95506228335348, 89.20607201266672, 93.34370253818409, 96.59471226749734, 100.7323427930147, 103.98335252232795};

    const std::vector<double> radius68_thetalt10_pgt500 = {4.081926458777424, 4.099431732299409, 4.262428482867968, 4.362017581473145, 4.831341579961153, 4.998346041276382, 6.2633736512415705, 6.588371889265881, 8.359969947444522, 9.015085558044309, 11.262722588206483, 12.250305471269183, 15.00547660437276, 16.187264014640103, 19.573764900578503, 20.68072032434797, 24.13797140783321, 25.62942209291236, 29.027596514735617, 30.215039667389316, 33.929540248019585, 36.12911729771914, 39.184563500620946, 42.02062468386282, 46.972125628650204, 47.78214816041894, 55.88428562462974, 59.15520134927332, 63.31816666637158, 66.58908239101515, 70.75204770811342, 74.022963432757, 78.18592874985525, 81.45684447449884};

    const std::vector<double> radius68_theta10to20 = {4.0251896715647115, 4.071661598616328, 4.357690094817289, 4.760224640141712, 6.002480766325418, 6.667318981016246, 8.652513285172342, 9.72379373302137, 12.479492693251478, 14.058548828317289, 17.544872909347912, 19.43616066939176, 23.594162859513734, 25.197329065282954, 29.55995803074302, 31.768946746958296, 35.79247330197688, 37.27810357669942, 41.657281051476545, 42.628141392692626, 47.94208483539388, 49.9289473559796, 54.604030254423975, 53.958762417361655, 53.03339560920388, 57.026277390001425, 62.10810455035879, 66.10098633115634, 71.1828134915137, 75.17569527231124, 80.25752243266861, 84.25040421346615, 89.33223137382352, 93.32511315462106};

    const std::vector<double> radius68_thetagt20 = {4.0754238481177705, 4.193693485630508, 5.14209420056253, 6.114996249971468, 7.7376807326481645, 8.551663213602291, 11.129110612057813, 13.106293737495639, 17.186617323282082, 19.970887612094604, 25.04088272634407, 28.853696411302344, 34.72538105333071, 40.21218694947545, 46.07344239520299, 50.074953583805346, 62.944045771758645, 61.145621459396814, 69.86940198299047, 74.82378572939959, 89.4528387422834, 93.18228303096758, 92.51751129204555, 98.80228884380018, 111.17537347472128, 120.89712563907408, 133.27021026999518, 142.99196243434795, 155.36504706526904, 165.08679922962185, 177.45988386054293, 187.18163602489574, 199.55472065581682, 209.2764728201696};

    // MIP tracking:  List of all layer z positions
    const std::vector<double> LAYER_Z_POSITIONS = {223.8000030517578, 226.6999969482422, 233.0500030517578, 237.4499969482422, 245.3000030517578, 251.1999969482422, 260.29998779296875,
        266.70001220703125, 275.79998779296875, 282.20001220703125, 291.29998779296875, 297.70001220703125, 306.79998779296875, 313.20001220703125,
        322.29998779296875, 328.70001220703125, 337.79998779296875, 344.20001220703125, 353.29998779296875, 359.70001220703125, 368.79998779296875,
        375.20001220703125, 384.29998779296875, 390.70001220703125, 403.29998779296875, 413.20001220703125, 425.79998779296875, 435.70001220703125,
        448.29998779296875, 458.20001220703125, 470.79998779296875, 480.70001220703125, 493.29998779296875, 503.20001220703125};


    void EcalVetoProcessor::buildBDTFeatureVector(const ldmx::EcalVetoResult& result) {
        bdtFeatures_.clear();
        bdtFeatures_.push_back(result.getNReadoutHits());
        bdtFeatures_.push_back(result.getSummedDet());
        bdtFeatures_.push_back(result.getSummedTightIso());
        bdtFeatures_.push_back(result.getMaxCellDep());
        bdtFeatures_.push_back(result.getShowerRMS());
        bdtFeatures_.push_back(result.getXStd());
        bdtFeatures_.push_back(result.getYStd());
        bdtFeatures_.push_back(result.getAvgLayerHit());
        bdtFeatures_.push_back(result.getDeepestLayerHit());
        bdtFeatures_.push_back(result.getStdLayerHit());
        // MIP tracking (unused but could be uncommented)
        //bdtFeatures.push_back(result.getNStraightTracks());
        //bdtFeatures.push_back(result.getNLinregTracks());
        //bdtFeatures.push_back(result.getFirstNearPhLayer());
        //bdtFeatures.push_back(result.getEpAng());
        //bdtFeatures.push_back(result.getEpSep());

        for(int ireg = 0; ireg < result.getElectronContainmentEnergy().size(); ++ireg) {
            bdtFeatures_.push_back(result.getElectronContainmentEnergy()[ireg]);
        }
        for(int ireg = 0; ireg < result.getPhotonContainmentEnergy().size(); ++ireg) {
            bdtFeatures_.push_back(result.getPhotonContainmentEnergy()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentEnergy().size(); ++ireg) {
            bdtFeatures_.push_back(result.getOutsideContainmentEnergy()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentNHits().size(); ++ireg) {
            bdtFeatures_.push_back(result.getOutsideContainmentNHits()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentXStd().size(); ++ireg) {
            bdtFeatures_.push_back(result.getOutsideContainmentXStd()[ireg]);
        }
        for(int ireg = 0; ireg < result.getOutsideContainmentYStd().size(); ++ireg) {
            bdtFeatures_.push_back(result.getOutsideContainmentYStd()[ireg]);
        }
        bdtFeatures_.push_back(result.getEcalBackEnergy());
    }

    void EcalVetoProcessor::configure(Parameters& parameters) {
        doBdt_ = parameters.getParameter< bool >("do_bdt");
        if (doBdt_){
            rt_ = std::make_unique<Ort::ONNXRuntime>(parameters.getParameter<std::string>("bdt_file"));
        }

        cellFileNamexy_ = parameters.getParameter< std::string >("cellxy_file");
        if (!std::ifstream(cellFileNamexy_).good()) {
            EXCEPTION_RAISE("EcalVetoProcessor",
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


        auto hexReadout{parameters.getParameter<Parameters>("hexReadout")};
        hexReadout_ = std::make_unique<EcalHexReadout>(hexReadout);

        nEcalLayers_ = parameters.getParameter< int >("num_ecal_layers");

        bdtCutVal_ = parameters.getParameter< double >("disc_cut");
        ecalLayerEdepRaw_.resize(nEcalLayers_, 0);
        ecalLayerEdepReadout_.resize(nEcalLayers_, 0);
        ecalLayerTime_.resize(nEcalLayers_, 0);

        // Set the collection name as defined in the configuration
        collectionName_ = parameters.getParameter< std::string >("collection_name");
    }

    void EcalVetoProcessor::clearProcessor(){
        cellMap_.clear();
        cellMapTightIso_.clear();
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
        // MIP tracking
        nStraightTracks_ = 0;
        nLinregTracks_ = 0;
        firstNearPhLayer_ = 0;
        epAng_ = 0;
        epSep_ = 0;

        std::fill(ecalLayerEdepRaw_.begin(), ecalLayerEdepRaw_.end(), 0);
        std::fill(ecalLayerEdepReadout_.begin(), ecalLayerEdepReadout_.end(), 0);
        std::fill(ecalLayerTime_.begin(), ecalLayerTime_.end(), 0);
    }

    void EcalVetoProcessor::produce(Event& event) {

        EcalVetoResult result;

        clearProcessor();

        // Get the collection of Ecal scoring plane hits. If it doesn't exist,
        // don't bother adding any truth tracking information.

        std::vector<double> recoilP;
        std::vector<float> recoilPos;
        std::vector<double> recoilPAtTarget;
        std::vector<float> recoilPosAtTarget;

        if (event.exists("EcalScoringPlaneHits")) {

            //
            // Loop through all of the sim particles and find the recoil electron.
            //

            // Get the collection of simulated particles from the event
            auto particleMap{event.getMap< int, SimParticle >("SimParticles")};

            // Search for the recoil electron
            auto [recoilTrackID, recoilElectron] = Analysis::getRecoil(particleMap);

            // Find ECAL SP hit for recoil electron
            auto ecalSpHits{event.getCollection< SimTrackerHit >( "EcalScoringPlaneHits" )};
            float pmax = 0;
            for ( SimTrackerHit &spHit : ecalSpHits ) {

                if (spHit.getLayerID() != 31 || spHit.getMomentum()[2] <= 0) continue;

                if (spHit.getTrackID() == recoilTrackID) {
                    if(sqrt(pow(spHit.getMomentum()[0],2) + pow(spHit.getMomentum()[1],2) + pow(spHit.getMomentum()[2],2)) > pmax) {
                        recoilP = spHit.getMomentum();
                        recoilPos = spHit.getPosition();
                        pmax = sqrt(pow(recoilP[0],2) + pow(recoilP[1],2) + pow(recoilP[2],2));
                    }
                }
            }

            // Find target SP hit for recoil electron
            if (event.exists("TargetScoringPlaneHits")) {
                std::vector< SimTrackerHit > targetSpHits = event.getCollection< SimTrackerHit >( "TargetScoringPlaneHits" );
                pmax = 0;
                for ( SimTrackerHit &spHit : targetSpHits ) {

                    if (spHit.getLayerID() != 1 || spHit.getMomentum()[2] <= 0) continue;

                    if (spHit.getTrackID() == recoilTrackID) {
                        if(sqrt(pow(spHit.getMomentum()[0],2) + pow(spHit.getMomentum()[1],2) + pow(spHit.getMomentum()[2],2)) > pmax) {
                            recoilPAtTarget = spHit.getMomentum();
                            recoilPosAtTarget = spHit.getPosition();
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
            photon_trajectory = getTrajectory({-pvec[0], -pvec[1], 4000.0 - pvec[2]}, posvec);
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
        const std::vector< EcalHit > ecalRecHits = event.getCollection< EcalHit >( "EcalRecHits" );

        EcalID globalCentroid = GetShowerCentroidIDAndRMS( ecalRecHits , showerRMS_);
        /* ~~ Fill the hit map ~~ O(n)  */
        fillHitMap( ecalRecHits , cellMap_);
        bool doTight = true;
        /* ~~ Fill the isolated hit maps ~~ O(n)  */
        fillIsolatedHitMap( ecalRecHits , globalCentroid, cellMap_, cellMapTightIso_, doTight);

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

        // MIP tracking: list storing all hit info used for tracking
        std::vector<HitData> trackingHitList;

        for ( const EcalHit &hit : ecalRecHits ) {
            //Layer-wise quantities
            EcalID id=hitID(hit);
            ecalLayerEdepRaw_[id.layer()] = ecalLayerEdepRaw_[id.layer()] + hit.getEnergy();
            if(id.layer() >= 20)
                ecalBackEnergy_ += hit.getEnergy();
            if (maxCellDep_ < hit.getEnergy())
                maxCellDep_ = hit.getEnergy();
            if (hit.getEnergy() > 0) {
                nReadoutHits_++;
                ecalLayerEdepReadout_[id.layer()] += hit.getEnergy();
                ecalLayerTime_[id.layer()] += (hit.getEnergy()) * hit.getTime();
                xMean += getCellCentroidXYPair(id).first * hit.getEnergy();
                yMean += getCellCentroidXYPair(id).second * hit.getEnergy();
                avgLayerHit_ += id.layer();
                wavgLayerHit += id.layer() * hit.getEnergy();
                if (deepestLayerHit_ < id.layer()) {
                    deepestLayerHit_ = id.layer();
                }
                XYCoords xy_pair = getCellCentroidXYPair(id);
                float distance_ele_trajectory = ele_trajectory.size() ? sqrt( pow((xy_pair.first - ele_trajectory[id.layer()].first),2) + pow((xy_pair.second - ele_trajectory[id.layer()].second),2) ) : -1.0;
                float distance_photon_trajectory = photon_trajectory.size() ? sqrt( pow((xy_pair.first - photon_trajectory[id.layer()].first),2) + pow((xy_pair.second - photon_trajectory[id.layer()].second),2) ) : -1.0;
                // Decide which region a hit goes into and add to sums
                for(unsigned int ireg = 0; ireg < nregions; ireg++) {
                    if(distance_ele_trajectory >= ireg*ele_radii[id.layer()] && distance_ele_trajectory < (ireg+1)*ele_radii[id.layer()])
                        electronContainmentEnergy[ireg] += hit.getEnergy();
                    if(distance_photon_trajectory >= ireg*photon_radii[id.layer()] && distance_photon_trajectory < (ireg+1)*photon_radii[id.layer()])
                        photonContainmentEnergy[ireg] += hit.getEnergy();
                    if(distance_ele_trajectory > (ireg+1)*ele_radii[id.layer()] && distance_photon_trajectory > (ireg+1)*photon_radii[id.layer()]) {
                        outsideContainmentEnergy[ireg] += hit.getEnergy();
                        outsideContainmentNHits[ireg] += 1;
                        outsideContainmentXmean[ireg] += xy_pair.first*hit.getEnergy();
                        outsideContainmentYmean[ireg] += xy_pair.second*hit.getEnergy();
                    }
                }

                // Decide whether hit should be added to trackingHitList:  add if inside e- RoC or if etraj is missing
                if(distance_ele_trajectory >= ele_radii[id.layer()] || distance_ele_trajectory == -1.0) {
                    HitData hd;
                    hd.pos = TVector3(xy_pair.first, xy_pair.second, LAYER_Z_POSITIONS[id.layer()]);
                    hd.layer = id.layer();
                    trackingHitList.push_back(hd);
                }
            }
        }

        for ( const auto& [ id , energy ] : cellMapTightIso_ ) {
            if ( energy > 0 ) summedTightIso_ += energy;
        }

        for (int iLayer = 0; iLayer < ecalLayerEdepReadout_.size(); iLayer++) {
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
        for ( const EcalHit &hit : ecalRecHits ) {
            EcalID id=hitID(hit);
            if (hit.getEnergy() > 0) {
                xStd_ += pow((getCellCentroidXYPair(id).first - xMean), 2) * hit.getEnergy();
                yStd_ += pow((getCellCentroidXYPair(id).second - yMean), 2) * hit.getEnergy();
                stdLayerHit_ += pow((id.layer() - wavgLayerHit), 2) * hit.getEnergy();
            }
            XYCoords xy_pair = getCellCentroidXYPair(id);
            float distance_ele_trajectory = ele_trajectory.size() ? sqrt( pow((xy_pair.first - ele_trajectory[id.layer()].first),2) + pow((xy_pair.second - ele_trajectory[id.layer()].second),2) ) : -1.0;
            float distance_photon_trajectory = photon_trajectory.size() ? sqrt( pow((xy_pair.first - photon_trajectory[id.layer()].first),2) + pow((xy_pair.second - photon_trajectory[id.layer()].second),2) ) : -1.0;
            for(unsigned int ireg = 0; ireg < nregions; ireg++) {
                if(distance_ele_trajectory > (ireg+1)*ele_radii[id.layer()] && distance_photon_trajectory > (ireg+1)*photon_radii[id.layer()]) {
                    outsideContainmentXstd[ireg] += pow((xy_pair.first - outsideContainmentXmean[ireg]),2) * hit.getEnergy();
                    outsideContainmentYstd[ireg] += pow((xy_pair.second - outsideContainmentYmean[ireg]),2) * hit.getEnergy();
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



        // MIP tracking starts here
        // std::cout << "BEGINNING MIP TRACKING" << std::endl;  //temp

        /* Goal:  Calculate nStraightTracks (self-explanatory), nLinregTracks (tracks found by linreg algorithm),
        ** firstNearPhLayer (layer of the first hit near the photon trajectory), epAng (angle between the projected
        ** electron and photon trajectories), and epSep (distance between the projected electron and photon
        ** trajectories at the scoring plane). */

        // Find epAng and epSep, and prepare EP trajectory vectors:
        TVector3 e_traj_start;
        TVector3 e_traj_end;
        TVector3 p_traj_start;
        TVector3 p_traj_end;
        if(ele_trajectory.size()==3 && photon_trajectory.size()==3) {
            /*make ele_trajectory and photon_trajectory into TVector3 pairs for later use*/
            //std::cout << "Expected, size==3" << std::endl;
            e_traj_start = TVector3(ele_trajectory[0].first, ele_trajectory[0].second, LAYER_Z_POSITIONS.front());
            e_traj_end =   TVector3(ele_trajectory[0].first, ele_trajectory[0].second, LAYER_Z_POSITIONS.back());
            p_traj_start = TVector3(photon_trajectory[0].first, photon_trajectory[0].second, LAYER_Z_POSITIONS.front());
            p_traj_end =   TVector3(photon_trajectory[0].first, photon_trajectory[0].second, LAYER_Z_POSITIONS.back());

            // TEMPORARILY COMMENTED for testing purposes
            TVector3 evec = e_traj_end - e_traj_start;
            TVector3 e_norm = evec.Unit();
            TVector3 pvec = p_traj_end - p_traj_start;
            TVector3 p_norm = pvec.Unit();
            float epDot = e_norm.Dot(p_norm);
            //std::cout << "Assigning norm" << std::endl;
            epAng_ = acos(epDot) * 180.0 / M_PI;  //In degrees for legibility
            epSep_ = sqrt( pow(e_traj_start.X() - p_traj_start.X(), 2) + 
                           pow(e_traj_start.Y() - p_traj_start.Y(), 2) );
        } else {
            /*All hits in the Ecal are fair game.  Pick e/ptraj so that they won't restrict anything.*/
            //std::cout << "Unexpected traj size " << ele_trajectory.size() << ", " << photon_trajectory.size() << std::endl;
            e_traj_start = TVector3(999,999,0);
            e_traj_end = TVector3(999,999,999);
            p_traj_start = TVector3(1000,1000,0);
            p_traj_end = TVector3(1000,1000,1000);
            epAng_ = 3.0 + 1.0;  /*ensures event will not be vetoed by angle/separation cut  */
            epSep_ = 10.0 + 1.0;
        }

        /* Compute firstNearPhLayer */
        firstNearPhLayer_ = LAYER_Z_POSITIONS.size();
        //std::cout << "trackingHitList contains " << trackingHitList.size() << " hits." << std::endl;

        if(photon_trajectory.size() != 0) {  /*if ptraj doesn't exist, leave phlayer at the default value*/
            for(std::vector<HitData>::iterator it = trackingHitList.begin(); it != trackingHitList.end(); ++it) {
                float ehDist = sqrt( pow((*it).pos.X() - photon_trajectory[(*it).layer].first,  2)
                                   + pow((*it).pos.Y() - photon_trajectory[(*it).layer].second, 2));
                if(ehDist < 8.7 && (*it).layer < firstNearPhLayer_) {
                    firstNearPhLayer_ = (*it).layer;
                }
            }
        }
        //std::cout << "COMPLETED INITIAL ANALYSIS.  STARTING TRACKING..." << std::endl;

        // Actual tracking begins here.


        // Straight track finding:
        
        /*First, sort list by decreasing layer number*/
        std::sort(trackingHitList.begin(), trackingHitList.end(), [](HitData ha, HitData hb) {return ha.layer > hb.layer;});

        // Info for current track:
        int track[34];  /*list of hit numbers in track; 34=maximum theoretical length*/
        int currenthit;
        int trackLen;
        float cellWidth = 8.7;

        for (int iHit = 0; iHit < trackingHitList.size(); iHit++) {
            track[0] = iHit;  /*automatically start searching for a new track*/
            currenthit = iHit;
            trackLen = 1;

            for (int jHit = 0; jHit < trackingHitList.size(); jHit++) {
                if (trackingHitList[jHit].layer == trackingHitList[currenthit].layer ||
                    trackingHitList[jHit].layer > trackingHitList[currenthit].layer + 3) {
                    continue;  //if hit is not in the next two layers, continue
                }
                /*if it is:  add to track if the (x,y) coordinates are identical to current hit.*/
                if (trackingHitList[jHit].pos.X() == trackingHitList[currenthit].pos.X() &&
                    trackingHitList[jHit].pos.Y() == trackingHitList[currenthit].pos.Y()) {
                    /*add it to the track*/
                    track[trackLen] = jHit;
                    trackLen++;
                }
            }
            /*confirm valid track*/
            if (trackLen < 2) continue;
            float closest_e = distTwoLines(trackingHitList[track[0]].pos, trackingHitList[track[trackLen-1]].pos,
                                           e_traj_start, e_traj_end);
            float closest_p = distTwoLines(trackingHitList[track[0]].pos, trackingHitList[track[trackLen-1]].pos,
                                           p_traj_start, p_traj_end);
            if (closest_p > cellWidth and closest_e < 2*cellWidth) continue;
            if (trackLen < 4 and closest_e > closest_p) continue;

            /*if track found, increment nStraightTracks and remove hits from future consideration*/
            if (trackLen >= 2) {
                for (int kHit = 0; kHit < trackLen; kHit++) {
                    trackingHitList.erase(trackingHitList.begin() + track[kHit]);
                }
                /*The *current* hit will have been removed, so iHit is currently pointing to the next hit.*/
                iHit--;  /*Decrement iHit so no hits will get skipped by iHit++*/
                nStraightTracks_++;
            }
            /*Optional addition:  Merge nearby straight tracks.  Not necessary for veto.*/
        }


        // OPTIONAL:  If a track is found, immediately skip the relatively slow linreg section.
        // Downside:  Can't do this if nLinregTracks is used in the BDT.




        // Linreg tracking:
        //std::cout << "BEGINNING LINREG TRACKING" << std::endl;

        //int track[34];
        //int trackLen;
        int hitsInRegion[50];
        int nHitsInRegion;
        //int currenthit;
        
        TMatrixD svdMatrix(3,3);
        TMatrixD Vm;
        TMatrixD hdt(3,3);
        TVector3 slopeVec;
        TVector3 hmean;
        TVector3 hpoint;
        TVector3 mean_best;
        TVector3 point_best;
        float r_corr_best;
        int hitNums_best[3];
        int hitNums[3];
        
        /* for (int iHit = 0; iHit < trackingHitList.size(); iHit++) {
            trackLen = 0;
            nHitsInRegion = 1;
            currenthit = iHit;
            hitsInRegion[0] = iHit;

            for (int jHit = 0; jHit < trackingHitList.size(); jHit++) {
                float dstToHit = (trackingHitList[iHit].pos - trackingHitList[jHit].pos).Mag();
                if (dstToHit <= 2*cellWidth) { //consider only hits within 2 cells of the primary hit
                    hitsInRegion[nHitsInRegion] = jHit;
                    nHitsInRegion++;
                }
            } */







        //std::cout << "TRACKING COMPLETED." << std::endl;

        result.setVariables(nReadoutHits_, deepestLayerHit_, summedDet_, summedTightIso_, maxCellDep_,
            showerRMS_, xStd_, yStd_, avgLayerHit_, stdLayerHit_, ecalBackEnergy_,
            nStraightTracks_, nLinregTracks_, firstNearPhLayer_, epAng_, epSep_,  // MIP tracking
            electronContainmentEnergy, photonContainmentEnergy, outsideContainmentEnergy, outsideContainmentNHits, outsideContainmentXstd, outsideContainmentYstd, ecalLayerEdepReadout_, recoilP, recoilPos);

        if (doBdt_) {
            buildBDTFeatureVector(result);
            Ort::FloatArrays inputs({bdtFeatures_});
            float pred = rt_->run({"features"}, inputs, {"probabilities"})[0].at(1);
            result.setVetoResult(pred > bdtCutVal_);
            result.setDiscValue(pred);
            //std::cout << "  pred > bdtCutVal = " << (pred > bdtCutVal_) << std::endl;

            // If the event passes the veto, keep it. Otherwise,
            // drop the event.
            if (result.passesVeto() && inside) {
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
        event.add( collectionName_, result );
    }

    /* Function to calculate the energy weighted shower centroid */
    EcalID EcalVetoProcessor::GetShowerCentroidIDAndRMS(const std::vector<EcalHit> &ecalRecHits, double& showerRMS) {
        auto wgtCentroidCoords = std::make_pair<float, float>(0., 0.);
        float sumEdep = 0;
        EcalID returnCellId;

        //Calculate Energy Weighted Centroid
        for (const EcalHit &hit : ecalRecHits ) {
        EcalID id = hitID(hit);
            CellEnergyPair cell_energy_pair = std::make_pair(id, hit.getEnergy());
            XYCoords centroidCoords = getCellCentroidXYPair(id);
            wgtCentroidCoords.first = wgtCentroidCoords.first + centroidCoords.first * cell_energy_pair.second;
            wgtCentroidCoords.second = wgtCentroidCoords.second + centroidCoords.second * cell_energy_pair.second;
            sumEdep += cell_energy_pair.second;
        }
        wgtCentroidCoords.first = (sumEdep > 1E-6) ? wgtCentroidCoords.first / sumEdep : wgtCentroidCoords.first;
        wgtCentroidCoords.second = (sumEdep > 1E-6) ? wgtCentroidCoords.second / sumEdep : wgtCentroidCoords.second;
        //Find Nearest Cell to Centroid
        float maxDist = 1e6;
        for ( const EcalHit &hit : ecalRecHits ) {
            XYCoords centroidCoords = getCellCentroidXYPair(hitID(hit));

            float deltaR = pow(pow((centroidCoords.first - wgtCentroidCoords.first), 2) + pow((centroidCoords.second - wgtCentroidCoords.second), 2), .5);
            showerRMS += deltaR * hit.getEnergy();
            if (deltaR < maxDist) {
                maxDist = deltaR;
                returnCellId = hitID(hit);
            }
        }
        if (sumEdep > 0)
            showerRMS = showerRMS / sumEdep;
        return EcalID(0,returnCellId.module(),returnCellId.cell()); // flatten
    }

    /* Function to load up empty vector of hit maps */
    void EcalVetoProcessor::fillHitMap(const std::vector<EcalHit> &ecalRecHits,
            std::map<EcalID, float>& cellMap_) {
        for ( const EcalHit &hit : ecalRecHits ) {
            EcalID id=hitID(hit);
            cellMap_.emplace( id , hit.getEnergy() );
        }
    }

    void EcalVetoProcessor::fillIsolatedHitMap(const std::vector<EcalHit> &ecalRecHits, EcalID globalCentroid,
            std::map<EcalID, float>& cellMap_, std::map<EcalID, float>& cellMapIso_, bool doTight) {
        for (const EcalHit &hit : ecalRecHits ) {
            auto isolatedHit = std::make_pair(true, EcalID());
            EcalID id=hitID(hit);
            EcalID flatid(0,id.module(),id.cell());
            if (doTight) {
                //Disregard hits that are on the centroid.
                if (flatid==globalCentroid) 
                    continue;

                //Skip hits that are on centroid inner ring
                if (isInShowerInnerRing(globalCentroid, flatid)) {
                    continue;
                }
            }

            //Skip hits that have a readout neighbor
            //Get neighboring cell id's and try to look them up in the full cell map (constant speed algo.)
            //  these ideas are only cell/module (must ignore layer)
            std::vector<EcalID> cellNbrIds = getInnerRingCellIds(id);

            for (int k = 0; k < 6; k++) {
                //update neighbor ID to the current layer
                cellNbrIds[k] = EcalID( id.layer() , cellNbrIds[k].module() , cellNbrIds[k].cell() );
                //look in cell hit map to see if it is there
                if (cellMap_.find(cellNbrIds[k]) != cellMap_.end()) {
                    isolatedHit = std::make_pair(false, cellNbrIds[k]);
                    break;
                }
            }
            if (!isolatedHit.first) {
                continue;
            }
            //Insert isolated hit
            cellMapIso_.emplace( id , hit.getEnergy() );
        }
    }

    // Calculate where trajectory intersects ECAL layers using position and momentum at scoring plane
    std::vector<std::pair<float,float> > EcalVetoProcessor::getTrajectory(std::vector<double> momentum, std::vector<float> position) {
        std::vector<XYCoords> positions;
        for(int iLayer = 0; iLayer < nEcalLayers_; iLayer++) {
            float posX = position[0] + (momentum[0]/momentum[2])*(hexReadout_->getZPosition(iLayer) - position[2]);
            float posY = position[1] + (momentum[1]/momentum[2])*(hexReadout_->getZPosition(iLayer) - position[2]);
            positions.push_back(std::make_pair(posX, posY));
        }
        return positions;
    }


    // MIP tracking

    // Return the minimum distance between the lines passing through points (v1,v2) and (w1,w2).
    float EcalVetoProcessor::distTwoLines(TVector3 v1, TVector3 v2, TVector3 w1, TVector3 w2) {
        TVector3 e1 = v1 - v2;
        TVector3 e2 = w1 - w2;
        TVector3 crs = e1.Cross(e2);
        if (crs.Mag() == 0) {
            return 100.0;  // arbitrary large number; edge case that shouldn't cause problems.
        } else {
            return std::abs(crs.Dot(v1 - w1) / crs.Mag());
        }
    }

    // Return the minimum distance between the point h1 and the line passing through p1 and p2.
    float EcalVetoProcessor::distPtToLine(TVector3 h1, TVector3 p1, TVector3 p2) {
        return ((h1 - p1).Cross(h1 - p2)).Mag() / (p1 - p2).Mag();
    }

}

DECLARE_PRODUCER_NS(ldmx, EcalVetoProcessor);
