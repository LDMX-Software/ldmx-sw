/**
 * @file EcalSim2Rec.cxx
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Ecal/EcalSim2Rec.h"

namespace ldmx {

    const std::vector<double> LAYER_WEIGHTS 
        = {1.641, 3.526, 5.184, 6.841,
          8.222, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775,
          8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 12.642, 16.51,
          16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 8.45}; //v2
   
    const double EcalSim2Rec::ELECTRONS_PER_MIP = 33000.0; // e-

    const double EcalSim2Rec::MIP_SI_RESPONSE = 0.130; // MeV

    EcalSim2Rec::EcalSim2Rec(const std::string& name, Process& process) :
        Producer(name, process) {
        noiseInjector_  = std::make_unique<TRandom3>(time(nullptr));
        noiseGenerator_ = std::make_unique<NoiseGenerator>();
    }

    void EcalSim2Rec::configure(const ParameterSet& ps) {

        hexReadout_ = std::make_unique<EcalHexReadout>();

        noiseIntercept_ = ps.getDouble("noiseIntercept",0.); 
        noiseSlope_     = ps.getDouble("noiseSlope",1.);
        padCapacitance_ = ps.getDouble("padCapacitance",0.1); 

        // Calculate the noise RMS based on the properties of the readout pad
        noiseRMS_ = this->calculateNoise(padCapacitance_, noiseIntercept_, noiseSlope_);  
        //std::cout << "[ EcalSim2Rec ]: Noise RMS: " << noiseRMS_ << " e-" << std::endl;

        // Convert the noise RMS in electrons to energy
        noiseRMS_ = noiseRMS_*(MIP_SI_RESPONSE/ELECTRONS_PER_MIP); 
        //std::cout << "[ EcalSim2Rec ]: Noise RMS: " << noiseRMS_ << " MeV" << std::endl;

        // Calculate the readout threhsold
        readoutThreshold_ = ps.getDouble("readoutThreshold")*noiseRMS_;
        //std::cout << "[ EcalSim2Rec ]: Readout threshold: " << readoutThreshold_ << " MeV" << std::endl;

        secondOrderEnergyCorrection_ = ps.getDouble( "secondOrderEnergyCorrection" , 4000./4220. );

        layerWeights_ = ps.getVDouble( "layerWeights" , LAYER_WEIGHTS );

        noiseGenerator_->setNoise(noiseRMS_); 
        noiseGenerator_->setPedestal(0); 
        noiseGenerator_->setNoiseThreshold(ps.getDouble("readoutThreshold")*noiseRMS_); 

    }

    void EcalSim2Rec::produce(Event& event) {

        std::vector<SimCalorimeterHit> ecalSimHits = event.getCollection<SimCalorimeterHit>(EventConstants::ECAL_SIM_HITS);
        std::vector<EcalHit> ecalRecHits;

        //std::cout << "[ EcalSim2Rec ] : Got " << numEcalSimHits 
        //          << " ECal hits in event " << event.getEventHeader()->getEventNumber()
        //          << std::endl;

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
        //std::cout << "[ EcalSim2Rec ]: Total number of empty channels: " << emptyChannels << std::endl;
        std::vector<double> noiseHits = noiseGenerator_->generateNoiseHits(emptyChannels);
        //std::cout << "[ EcalSim2Rec ]: Total number of noise hits: " << noiseHits.size() << std::endl; 
        for (double noiseHit : noiseHits) { 
            //std::cout << "[ EcalSim2Rec ]: Noise hit amplitude: " << noiseHit << std::endl;

            // Construct a hit in the ith position
            EcalHit recHit;
            
            // Set the raw energy of the hit
            recHit.setAmplitude(noiseHit);

            // Generate a random ID and pack it
            int layerID = noiseInjector_->Integer(NUM_ECAL_LAYERS); 
            //std::cout << "[ EcalSim2Rec ]: Random layer ID: " << layerID << std::endl;
            int moduleID = noiseInjector_->Integer(HEX_MODULES_PER_LAYER); 
            //std::cout << "[ EcalSim2Rec ]: Random module ID: " << moduleID << std::endl;
            int cellID = noiseInjector_->Integer(CELLS_PER_HEX_MODULE); 
            //std::cout << "[ EcalSim2Rec ]: Random cell ID: " << cellID << std::endl;
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
