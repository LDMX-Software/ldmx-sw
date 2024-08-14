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
     * In the energy regions where different distributions overlap, some weighting
     * needs to be done. There is a lot to be optimised on that front!
     */

    // Initialise number of electrons variable
    int nElectrons{0};

    // Number of electrons predicted by TS
    int nElectronsTS = tracks.size();
    // Number of electrons predicted by Ecal
    int nElectronsEcal = ecalClusters.size();

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
      if (ecalTotEnergy > 400.) { // If larger than some small value < ecal clusters seed threshold, it is non-zero
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
      if (tracks.size() != 0) {
        nElectronsPE = 1;
      } else {
        nElectrons = 0;
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

    // Weight when comparing TS prediciton and cluster energy prediction
    // TODO Find optimal weights
    // Note: Larger weight -> More TS, smaller weight -> More cluster energy.
    //       Larger weights lead to less overcounting and more undercounting
    //       and vice versa, as the TS generally should not overcount.
    //       A balance needs to be found where the TS undercounting is minimised
    //       without introducing overcounting.
    float w = 0.75;

    // If Ecal and TS agree, but there are no found tracks/clusters
    if (nElectronsTS == nElectronsEcal && nElectronsTS == 0) {
      nElectrons = 0;
    }
    // If Ecal and TS agree and there are tracks/clusters
    else if (nElectronsTS == nElectronsEcal) {
      // If TS prediction equal to cluster energy prediction rounded up
      if (nElectronsTS == (int)std::ceil(nElectronsEnergy)) {
        nElectrons = nElectronsTS;
      }
      // If TS prediction equal to cluster energy prediction rounded down
      else if (nElectronsTS == (int)std::floor(nElectronsEnergy)) {
        nElectrons = std::round( w*nElectronsTS + (1-w)*nElectronsEnergy);
      }
      // If cluster energy prediction >= 1 prediction different
      else {
        // Double check with track photoelectron prediction
        if (fabs(nElectronsTS - nElectronsPE) < 1) {
          nElectrons = nElectronsTS;
        }
        // If still not sure, do a weighted average between TS and energy prediction
        else {
          nElectrons = std::round( w*nElectronsTS + (1-w)*nElectronsEnergy);
        }
      }
    }
    // If Ecal and TS do not agree
    else {
      // If there are no TS tracks, but Ecal clusters exists
      if (nElectronsTS == 0 && nElectronsEcal != 0) {
        // If Ecal and cluster energy prediction agree
        if ((int)std::round(nElectronsEnergy) == nElectronsEcal) {
          nElectrons = nElectronsEcal;
        }
        // If they do not agree, prevent overcounting by taking smallest
        // possible number of electrons
        else {
          nElectrons = std::min((int)std::floor(nElectronsEnergy), nElectronsEcal);
        }
      }
      // If there are no Ecal clusters, but TS tracks exists
      else if (nElectronsEcal == 0 && nElectronsTS != 0) {
        // If there somehow way too many PE for the maount of tracks, add one extra electron
        if (nElectronsPE - nElectronsTS > 2.) {
          nElectrons = nElectronsTS + 1;
        }
        // Else number of tracks is number of electrons
        else {
          nElectrons = nElectronsTS;
        }
      }
      // If there are more TS tracks than Ecal clusters or vice versa (i.e not same amount)
      else if (fabs(nElectronsEnergy - nElectronsTS) < fabs(nElectronsEnergy - nElectronsEcal)) {
        // If TS and cluster energy agree more than Ecal and cluster energy
        if (nElectronsTS == (int)std::round(nElectronsEnergy)) {
          nElectrons = nElectronsTS;
        } else if (nElectronsPE - nElectronsTS > 1.) {
          nElectrons = std::round(nElectronsEnergy);
        } else {
          nElectrons = std::round( w*nElectronsTS + (1-w)*nElectronsEnergy);
        }  
      } else {
        // If Ecal and cluster energy agree more than TS and cluster energy
        if (nElectronsEcal = (int) std::round(nElectronsEnergy)) {
          nElectrons = nElectronsEcal;
        }
        // If they do not agree, prevent overcounting by taking smallest
        // possible number of electrons
        else {
          nElectrons = std::min((int)std::floor(nElectronsEnergy), nElectronsEcal);
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
  event.add(outputCollection_ + "Interacting", nonInteracting);
}
}  // namespace recon

DECLARE_PRODUCER_NS(recon, ElectronCounter2)