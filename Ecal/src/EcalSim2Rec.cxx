/**
 * @file EcalSim2Rec.cxx
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Ecal/EcalSim2Rec.h"

namespace ldmx {

    const double EcalSim2Rec::ELECTRONS_PER_MIP = 33000.0; // e-

    const double EcalSim2Rec::MIP_SI_RESPONSE = 0.130; // MeV

    EcalSim2Rec::EcalSim2Rec(const std::string& name, Process& process) :
        Producer(name, process) {
        noiseInjector_  = std::make_unique<TRandom3>(time(nullptr));
        noiseGenerator_ = std::make_unique<NoiseGenerator>();
    }

    void EcalSim2Rec::configure(Parameters& ps) {

        // These are the v12 parameters
        //  all distances in mm
        double moduleRadius = 85.0; //same as default
        int    numCellsWide = 23; //same as default
        double moduleGap = 1.0;
        double ecalFrontZ = 220;
        std::vector<double> ecalSensLayersZ = {
             7.850,
            13.300,
            26.400,
            33.500,
            47.950,
            56.550,
            72.250,
            81.350,
            97.050,
            106.150,
            121.850,
            130.950,
            146.650,
            155.750,
            171.450,
            180.550,
            196.250,
            205.350,
            221.050,
            230.150,
            245.850,
            254.950,
            270.650,
            279.750,
            298.950,
            311.550,
            330.750,
            343.350,
            362.550,
            375.150,
            394.350,
            406.950,
            426.150,
            438.750
        };

        hexReadout_ = std::make_unique<EcalHexReadout>(
                moduleRadius,
                moduleGap,
                numCellsWide,
                ecalSensLayersZ,
                ecalFrontZ
                );

        noiseIntercept_ = ps.getParameter<double>("noiseIntercept"); 
        noiseSlope_     = ps.getParameter<double>("noiseSlope");
        padCapacitance_ = ps.getParameter<double>("padCapacitance"); 

        // Calculate the noise RMS based on the properties of the readout pad
        noiseRMS_ = this->calculateNoise(padCapacitance_, noiseIntercept_, noiseSlope_);  
        ldmx_log(debug) << "Noise RMS: " << noiseRMS_ << " e-";

        // Convert the noise RMS in electrons to energy
        noiseRMS_ = noiseRMS_*(MIP_SI_RESPONSE/ELECTRONS_PER_MIP); 
        ldmx_log(debug) << "Noise RMS: " << noiseRMS_ << " MeV";

        // Calculate the readout threhsold
        readoutThreshold_ = ps.getParameter<double>("readoutThreshold")*noiseRMS_;
        ldmx_log(debug) << "Readout threshold: " << readoutThreshold_ << " MeV";

        secondOrderEnergyCorrection_ = ps.getParameter<double>( "secondOrderEnergyCorrection" );

        layerWeights_ = ps.getParameter<std::vector<double>>( "layerWeights" );

        noiseGenerator_->setNoise(noiseRMS_); 
        noiseGenerator_->setPedestal(0); 
        noiseGenerator_->setNoiseThreshold(readoutThreshold_);

    }

    void EcalSim2Rec::produce(Event& event) {

        std::vector<SimCalorimeterHit> ecalSimHits = event.getCollection<SimCalorimeterHit>(EventConstants::ECAL_SIM_HITS);
        std::vector<EcalHit> ecalRecHits;

        ldmx_log(debug) 
            << "Got " << ecalSimHits.size()
            << " ECal hits in event " << event.getEventHeader().getEventNumber();

        //First we simulate noise injection into each hit and store layer-wise 
        // max cell ids
        for (const SimCalorimeterHit &simHit : ecalSimHits ) {
            
            double hitNoise = noiseInjector_->Gaus(0, noiseRMS_);
            layer_cell_pair hit_pair = hitToPair(simHit);

            EcalHit recHit;

            recHit.setID(simHit.getID());
            double energy = simHit.getEdep() + hitNoise;
            recHit.setAmplitude(energy);
            if (energy > readoutThreshold_) {
                recHit.setEnergy(((energy/MIP_SI_RESPONSE)*layerWeights_.at(hit_pair.first)+energy)*secondOrderEnergyCorrection_);
                recHit.setTime(simHit.getTime());
            } else {
                recHit.setEnergy(0);
                recHit.setTime(-1000);
            }

            ecalRecHits.push_back( recHit );
        }

        // Given the number of channels without a hit, calculate the expected 
        // number of noise hits above the readout threshold and randomly 
        // assign them to Ecal cells
        int emptyChannels = TOTAL_CELLS - ecalRecHits.size();
        ldmx_log(debug) << "Total number of empty channels: " << emptyChannels;
        std::vector<double> noiseHits = noiseGenerator_->generateNoiseHits(emptyChannels);
        ldmx_log(debug) << "Total number of noise hits: " << noiseHits.size();
        for (double noiseHit : noiseHits) { 
            ldmx_log(debug) << "Noise hit amplitude: " << noiseHit;

            // Construct a hit in the ith position
            EcalHit recHit;
            
            // Set the raw energy of the hit
            recHit.setAmplitude(noiseHit);

            // Generate a random ID and pack it
            int layerID = noiseInjector_->Integer(NUM_ECAL_LAYERS); 
            int moduleID = noiseInjector_->Integer(HEX_MODULES_PER_LAYER); 
            int cellID = noiseInjector_->Integer(CELLS_PER_HEX_MODULE); 
            ldmx_log(debug)
                << "Random layer ID: " << layerID
                << ", Random module ID: " << moduleID
                << ", Random cell ID: " << cellID;
            detID_.setFieldValue(1, layerID); 
            detID_.setFieldValue(2, moduleID); 
            detID_.setFieldValue(3, cellID); 
            recHit.setID(detID_.pack()); 

            // Set the calibrated energy of the hit
            recHit.setEnergy(((noiseHit/MIP_SI_RESPONSE)*layerWeights_.at(layerID)+noiseHit)*secondOrderEnergyCorrection_);
            
            // Identify this hit as a noise hit.
            recHit.setNoise(true);

            ecalRecHits.push_back( recHit );
        } 

        event.add("EcalRecHits", ecalRecHits );
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalSim2Rec);
