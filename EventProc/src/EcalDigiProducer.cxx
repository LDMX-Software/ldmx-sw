/**
 * @file EcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "EventProc/EcalDigiProducer.h"

namespace ldmx {

    const std::vector<double> LAYER_WEIGHTS 
        = {1.641, 3.526, 5.184, 6.841,
        8.222, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775,
        8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 12.642, 16.51,
        16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 8.45}; 
   
    const double EcalDigiProducer::ELECTRONS_PER_MIP = 33000.0; // e-

    const double EcalDigiProducer::MIP_SI_RESPONSE = 0.130; // MeV

    EcalDigiProducer::EcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
        noiseGenerator_ = std::make_unique<NoiseGenerator>();
        noiseInjector_ = std::make_unique<TRandom3>(time(nullptr));
    }

    EcalDigiProducer::~EcalDigiProducer() { }

    void EcalDigiProducer::configure(const ParameterSet& ps) {

        hexReadout_ = std::make_unique<EcalHexReadout>();

        noiseIntercept_ = ps.getDouble("noiseIntercept",0.); 
        noiseSlope_     = ps.getDouble("noiseSlope",1.);
        padCapacitance_ = ps.getDouble("padCapacitance",0.1); 

        // Calculate the noise RMS based on the properties of the readout pad
        noiseRMS_ = this->calculateNoise(padCapacitance_, noiseIntercept_, noiseSlope_);  
        //std::cout << "[ EcalDigiProducer ]: Noise RMS: " << noiseRMS_ << " e-" << std::endl;

        // Convert the noise RMS in electrons to energy
        noiseRMS_ = noiseRMS_*(MIP_SI_RESPONSE/ELECTRONS_PER_MIP); 
        //std::cout << "[ EcalDigiProducer ]: Noise RMS: " << noiseRMS_ << " MeV" << std::endl;

        // Calculate the readout threhsold
        readoutThreshold_ = ps.getDouble("readoutThreshold")*noiseRMS_;
        //std::cout << "[ EcalDigiProducer ]: Readout threshold: " << readoutThreshold_ << " MeV" << std::endl;

        noiseGenerator_->setNoise(noiseRMS_); 
        noiseGenerator_->setPedestal(0); 
        noiseGenerator_->setNoiseThreshold(ps.getDouble("readoutThreshold")*noiseRMS_); 

    }

    void EcalDigiProducer::produce(Event& event) {

        std::vector<SimCalorimeterHit> ecalSimHits = event.getCollection<SimCalorimeterHit>("EcalSimHits" , "sim" );
        std::vector<EcalHit> ecalRecHits;

        //First we simulate noise injection into each hit and store layer-wise 
        // max cell ids
        for ( SimCalorimeterHit &simHit : ecalSimHits ) {
            
            double hitNoise = noiseInjector_->Gaus(0, noiseRMS_);
            layer_cell_pair hit_pair = hitToPair(simHit);

            EcalHit recHit;

            recHit.setID(simHit.getID());
            double energy = simHit.getEdep() + hitNoise;
            recHit.setAmplitude(energy);
            if (energy > readoutThreshold_) {
                recHit.setEnergy(((energy/MIP_SI_RESPONSE)*LAYER_WEIGHTS[hit_pair.first]+energy)*0.948);
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
        int emptyChannels = TOTAL_CELLS - ecalSimHits.size();
        std::vector<double> noiseHits = noiseGenerator_->generateNoiseHits(emptyChannels);
        for (double noiseHit : noiseHits) { 

            // Construct a hit in the ith position
            EcalHit recHit;
            
            // Set the raw energy of the hit
            recHit.setAmplitude(noiseHit);

            // Generate a random ID and pack it
            int layerID = noiseInjector_->Integer(NUM_ECAL_LAYERS); 
            //std::cout << "[ EcalDigiProducer ]: Random layer ID: " << layerID << std::endl;
            int moduleID = noiseInjector_->Integer(HEX_MODULES_PER_LAYER); 
            //std::cout << "[ EcalDigiProducer ]: Random module ID: " << moduleID << std::endl;
            int cellID = noiseInjector_->Integer(CELLS_PER_HEX_MODULE); 
            //std::cout << "[ EcalDigiProducer ]: Random cell ID: " << cellID << std::endl;
            detID_.setFieldValue(1, layerID); 
            detID_.setFieldValue(2, moduleID); 
            detID_.setFieldValue(3, cellID); 
            recHit.setID(detID_.pack()); 

            // Set the calibrated energy of the hit
            recHit.setEnergy(((noiseHit/MIP_SI_RESPONSE)*LAYER_WEIGHTS[layerID]+noiseHit)*0.948);
            
            // Identify this hit as a noise hit.
            recHit.setNoiseHit(true);

            ecalRecHits.push_back( recHit );
        } 

        event.add( "EcalRecHits", ecalRecHits );
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
