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
  ecalEnergySplit_ = parameters.getParameter<std::vector<double>>("ecal_energy_split");

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

    // TS prediction of number of electrons
    int nElectronsTS = tracks.size();
    // Ecal prediction of number of electrons
    float ecalTotEnergy = 0;
    for (int i = 0; i < ecalClusters.size(); i++) {
      ecalTotEnergy += ecalClusters.at(i).getEnergy();
    }
    int nElectronsEcal = 0;
    for (int i = 0; i < ecalEnergySplit_.size(); i++) {
      if (std::floor(ecalTotEnergy / ecalEnergySplit_.at(i)) != 0) {
        nElectronsEcal++;
        continue;
      } else {
        break;
      }
    }

    // Initialise number of electrons variable
    int nElectrons{0};

    // Determine amount of electrons in event
    if(nElectronsTS == nElectronsEcal) {                        // If same prediction in both
      nElectrons = nElectronsTS;
    } else {                                                    // If not same prediction in both
      if (nElectronsTS == 0 && nElectronsEcal > 0) {              // If zero TS tracks and > 0 Ecal clusters
        nElectrons = nElectronsEcal;
      } else if (nElectronsEcal == 0 && nElectronsTS > 0) {       // If zero Ecal clusters and > 0 TS tracks
        nElectrons = nElectronsTS;
      } else {
        nElectrons = std::max(nElectronsTS, nElectronsEcal);
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