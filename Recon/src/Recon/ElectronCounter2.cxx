/**
 * @file ElectronCounter2.cxx
 * @brief Class that combines TS and Ecal information for event electron counting.
 * @author Erik Lundblad, Lund University
 */

#include "Recon/ElectronCounter2.h"

namespace recon {

void ElectronCounter2::configure(framework::config::Parameters& parameters) {
  inputCollections_ = parameters.getParameter<std::vector<std::string>>("input_collections");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  xTolerance = parameters.getParameter<double>("x_tolerance");
  yTolerance = parameters.getParameter<double>("y_tolerance");
  verbose_ = parameters.getParameter<bool>("verbose");
  ecalPosShift_ = parameters.getParameter<std::vector<double>>("ecal_position_shift_xy");
  ecalPeakMean_ = parameters.getParameter<std::vector<double>>("ecal_peak_mean");
  tsPeakMean_ = parameters.getParameter<std::vector<double>>("ts_peak_mean");

  ldmx_log(debug) << "ElectronCounter2 is using parameters: "
                  << " \n\tinput_collection (TS tracks) = " << inputCollections_[0]
                  << " \n\tinput_collection (Ecal clusters) = " << inputCollections_[1]
                  << " \n\tinput_pass_name = " << inputPassName_
                  << " \n\toutput_collection = " << outputCollection_;
}

void ElectronCounter2::produce(framework::Event& event) {
    // Check if input TS track collection exists
    if (!event.exists(inputCollections_[0], inputPassName_)) {
      ldmx_log(fatal) << "Attemping to use non-existing input collection "
                      << inputCollections_[0] << "_" << inputPassName_
                      << " to count electrons! Exiting.";
      return;
    }
    // Check if input Ecal cluster collection exists
    if (!event.exists(inputCollections_[1], inputPassName_)) {
      ldmx_log(fatal) << "Attemping to use non-existing input collection "
                      << inputCollections_[1] << "_" << inputPassName_
                      << " to count electrons! Exiting.";
      return;
    }

    // Load TS tracks
    const std::vector<ldmx::TrigScintTrack> tracks =
        event.getCollection<ldmx::TrigScintTrack>(inputCollections_[0], inputPassName_);

    // Load Ecal clusters
    const std::vector<ldmx::EcalCluster> ecalClusters =
        event.getCollection<ldmx::EcalCluster>(inputCollections_[1], inputPassName_);


    /** Electron counter general idea:
     * The general idea of this electron counter is to compare the
     * predictive capabilities of the TS and the Ecal. When they agree,
     * the amount of electrons is easily determined. When they do not agree,
     * something else needs to veto the amount of electrons. In this producer,
     * it is the total amount of deposited energy in the Ecal clusters, as
     * there is a rather clear separation between energy distributions.
     * In the energy regions where different distributions overlap, we can
     * look at the total amount of photoelectrons in the TS tracks.
     */

    // Initialise number of electrons variable
    int nElectrons{0};

    // Number of electrons predicted by TS
    int nElectronsTS = tracks.size();
    // Number of electrons predicted by Ecal
    int nElectronsEcal = ecalClusters.size();


    // TODO: Optimise the Energy and PE electron predictions.
    // Known issues:
    //       - PE prediction tends to severly overcount and is pretty much useless atm
    //       - Energy prediction is a very good veto, but can probably be tweaked a bit



    // Number of electrons predicted by total deposited Ecal cluster energy
    float nElectronsEnergy;
    // Total amount of deposited energy in Ecal clusters
    float ecalTotEnergy = 0;
    for (int i = 0; i < ecalClusters.size(); i++) {
      ecalTotEnergy += ecalClusters.at(i).getEnergy();
    }
    // Here it is assumed that there is no overcounting if there is more
    // deposited energy in the ecal clusters than the mean value of the energy-distribution
    int minElectronsEnergy = 0;
    for (int i = 0; i < ecalPeakMean_.size(); i++) {
      if (std::floor(ecalTotEnergy / ecalPeakMean_.at(i)) != 0) {
        minElectronsEnergy += 1;
        continue;
      } else {
        break;
      }
    }
    // If passed all peak means, set number of electrons to number of peak means passed
    if (minElectronsEnergy == ecalPeakMean_.size()) {
      nElectronsEnergy = ecalPeakMean_.size();
    } else if (minElectronsEnergy == 0) { // If passed no peak mean, it is either zero or one
      if (nElectronsEcal != 0) {
        nElectronsEnergy = 1;
      } else {
        nElectronsEnergy = 0;
      }
      
    } else {
      // To prevent undercounting: Add exponentially increasing float,
      // that is zero at the passed peak and one at the next peak
      float diff = std::pow(2, (fabs(ecalTotEnergy - ecalPeakMean_.at(minElectronsEnergy - 1)) /
                      fabs(ecalPeakMean_.at(minElectronsEnergy) - ecalPeakMean_.at(minElectronsEnergy - 1)))) - 1;
      nElectronsEnergy = minElectronsEnergy + diff;
    }

    // Number of electrons predicted by total number of TS tracks photoelectrons
    float nElectronsPE = 0;
    
    // Sum of PE in TS tracks
    float tsTotPE = 0;
    for (int i = 0; i < tracks.size(); i++) {
      tsTotPE += tracks.at(i).getPE();
    }
    // Here it is assumed that there is no overcounting if there is more
    // photoelectrons in the TS tracks than the mean value of the photoelectron-distribution
    int minElectronsPE = 0;
    for (int i = 0; i < tsPeakMean_.size(); i++) {
      if (std::floor(tsTotPE / tsPeakMean_.at(i)) != 0) {
        minElectronsPE++;
        continue;
      } else {
        break;
      }
    }
    // If passed all peak means, set number of electrons to number of peak means passed
    if (minElectronsPE == tsPeakMean_.size()) {
      nElectronsPE = tsPeakMean_.size();
    } else if (minElectronsPE == 0) { // There are no tracks with zero electrons
      if (nElectronsTS != 0) {
        nElectronsPE = 1;
      } else {
        nElectronsPE = 0;
      }
      
    } else {
      // To prevent undercounting: Add exponentially increasing float,
      // that is zero at the passed peak and one at the next peak
      float diff = std::pow(2, fabs(tsTotPE - tsPeakMean_.at(minElectronsPE - 1)) /
                      fabs(tsPeakMean_.at(minElectronsPE) - tsPeakMean_.at(minElectronsPE - 1))) - 1;
      nElectronsPE = minElectronsPE + diff;
    }

    if (verbose_) {
      ldmx_log(debug) << "TS found " << nElectronsTS << " based on number of tracks.";
      ldmx_log(debug) << "TS found " << nElectronsPE << " based on number of track photoelectrons.";
      ldmx_log(debug) << "Ecal found " << nElectronsEcal << " based on number of clusters.";
      ldmx_log(debug) << "Ecal found " << nElectronsEnergy << " based on total cluster energy.";
    }
    
    // If Ecal and TS agree, but there are no found tracks/clusters
    if (nElectronsTS == nElectronsEcal && nElectronsTS == 0) {
      // TODO: - There should not be zero electron events. An additional check needs to exist
      //         for the case where there are no TS tracks or Ecal clusters.
      //          * Suggestions include: Looking at total TS deposited energy per pad
      //       - It would be very interesting to look at the predictions for 0e events

      // Now this should always be false, but can still be here in case
      // nElectronsTS or nElectronsEcal are redefined
      if (nElectronsEnergy != 0 || nElectronsPE != 0) {
        nElectrons = 1;
      }
      else {
        nElectrons = 0;
      }
    }
    // If Ecal and TS agree and there are tracks/clusters
    else if (nElectronsTS == nElectronsEcal) {
      // TODO: - There are cases where the Ecal and TS guesses the same and it is wrong.
      //         A very common case is that it is a 2e event and both guess 1e. The code
      //         below tries to solve it by looking at if there is way more energy than predicted
      //         by number of clusters/tracks, then increase with one. However it needs to be:
      //          * Generalise: What if 3e event and guess 1e, then energy veto increases to 2e. Still wrong!
      //          * Probably need to add another veto. Suggestion is once again TS deposited energy per pad.

      // Check if there is risk of undercounting
      // Could lead to overcounting, but can be prevented by increasing energy and PE needed for min electrons
      if ((nElectronsEnergy - nElectronsTS) > 2. &&
          (nElectronsPE - nElectronsTS) > 3.) {
        nElectrons = nElectronsTS + 1;
      }
      // Else this is the correct prediction
      else {
        nElectrons = nElectronsTS;
      }
    }
    // If Ecal and TS do not agree
    else {
      // If there are no TS tracks, but Ecal clusters exists
      if (nElectronsTS == 0 && nElectronsEcal != 0) {
        nElectrons = std::min((int)std::round(nElectronsEnergy), nElectronsEcal);
      }
      // If there are no Ecal clusters, but TS tracks exists
      else if (nElectronsEcal == 0 && nElectronsTS != 0) {
        // TODO: - This code does not stop TS undercounting, but prevents overcounting somewhat.
        //       - Should compare to TS PE prediction when it has been tweaked and does not overcount anymore.
        //          * Suggestion: When we know that PE prediciton is better, check if there is much more PE
        //                        than there should be. In that case, add an additional electron.

        nElectrons = nElectronsTS;
      }
      // If not same prediction and both non-zero predictions
      // This is probably where the most overcounting could appear
      else {
        // If TS predicts less than Ecal, probably no overcounting
        // However undercounting could be a problem
        if (nElectronsTS < nElectronsEcal) {
          // Check if TS undercounts and try to catch some of it
          // without overcounting
          // TODO: - change PE TS comparison to >3. instaed of >2. and compare, maybe not needed?
          //       - Optimise. A lot can be done here to reduce undercounting
          if ((nElectronsEnergy - nElectronsTS) > 2. &&
              (nElectronsPE - nElectronsTS) > 2.) {
                nElectrons = nElectronsTS + 1;
          } else {
            nElectrons = nElectronsTS;
          }
        }
        // If Ecal predicts less than TS, undercounting should not be a problem
        // However overcounting could in extreme cases be a problem
        else {
          // Check if TS overcounts
          if ((nElectronsTS - std::round(nElectronsEnergy)) > 0 ||
              (nElectronsTS - std::round(nElectronsPE)) > 0) {
            nElectrons = nElectronsTS - 1;
          } else {
            nElectrons = nElectronsTS;
          }
        }
      }
    }

    if(verbose_) {
      ldmx_log(debug) << "Found " << nElectrons
                  << " electron(s) (tracks) using input collections "
                  << inputCollections_[0] << "_" << inputPassName_
                  << " and "
                  << inputCollections_[1] << "_" << inputPassName_;
    }

    // Now this code is still very new and has not been checked.
    // The purpose of it is to determine if an electron is non-interacting.
    // It should probably be it's own script and should take the number of electrons as input.
    // TODO: - Fix this. It does not work as sometimes the TS uncertainty bounds are very large
    //       if no x tracks are found and sometimes the TS underpredicts and the Ecal overpredicts,
    //       leading to more combinations than there are electrons.
    //       - To properly fix this we should incorporate it into the code to determine number of electrons
    //       above and change how we determine if it is non-interacting depending on what case of electron
    //       counting is being looked at.
    //       - The code here that compares if an ecal cluster is within a TS track uncertainty bound should only
    //       be there for the case where number TS tracks = number Ecal clusters = number electrons
    //      - The idea behind the code is that if a beam electron has passed through the TS along the curved path
    //      to the Ecal without changing trajectory, it has not scattered and has thus not interacted with the target.
    //      - Ideally, we need a script after determining the amount of electrons, that finds their position.
    //      - First after that a comparison can be made.
    // Initialise is(are) electron(s) non-interacting?
    std::vector<bool> nonInteracting;
    if (nElectrons > 0) {                                           // If there are electrons in event
      for (ldmx::EcalCluster cluster : ecalClusters) {                // For every cluster
        bool hasFoundMatch = false;                                     // Define has found non-interacting electron
        for (ldmx::TrigScintTrack track : tracks) {                       // For every track
          // If there exists an Ecal cluster within the uncertainty
          // in position of a TS track, it is deemed booring
          if (fabs(track.getX() - (cluster.getCentroidX() - ecalPosShift_.at(0))) <= track.getSigmaX() + xTolerance &&
              fabs(track.getY() - (cluster.getCentroidY() - ecalPosShift_.at(1))) <= track.getSigmaY() + yTolerance &&
              nElectrons < tracks.size()) {
            hasFoundMatch = true;
            break;
          }
        }
        nonInteracting.push_back(hasFoundMatch);
      }

      // Clean up if there are more booleans than there are electrons
      if (nonInteracting.size() > nElectrons) {
        if (verbose_) {
          ldmx_log(debug) << "Found " << nonInteracting.size()
                          << " matches, but only "
                          << nElectrons << " electron(s). Cleanup!";
        }
        // Find out how many non-interacting in vector
        int numberTrue = 0;
        auto it = find(nonInteracting.begin(),nonInteracting.end(),true);
        while (it != nonInteracting.end()) {  
          numberTrue++;
          it = std::find(it + 1, nonInteracting.end(), true);
        }
        if (verbose_) {
          ldmx_log(debug) << "Found " << numberTrue << " non-interacting electron(s)";
        }
        // Clean up
        if (numberTrue >= nElectrons) {            // If only non-interacting electrons
          std::vector<bool> nonInteractingCleaned(nElectrons,true);
          nonInteracting = nonInteractingCleaned;
          if (verbose_) {
            ldmx_log(debug) << "Only non-interacting electrons in event. Cleanup complete.";
          }
        } else {                                  // If there are interacting electrons
          std::vector<bool> nonInteractingCleaned(nonInteracting.begin(),(nonInteracting.begin()+nElectrons));
          nonInteracting = nonInteractingCleaned;
          if (verbose_) {
            ldmx_log(debug) << "Mix of non-interacting and interacting electrons in event. Cleanup complete.";
          }
        }
      }
    }

  // Add number of electrons and if non-interacting
  event.add(outputCollection_, nElectrons);
  event.add(outputCollection_ + "TS", nElectronsTS);
  event.add(outputCollection_ + "Ecal", nElectronsEcal);
  event.add(outputCollection_ + "EcalEnergy", nElectronsEnergy);
  event.add(outputCollection_ + "TSPE", nElectronsPE);
  event.add(outputCollection_ + "Interacting", nonInteracting);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, ElectronCounter2)