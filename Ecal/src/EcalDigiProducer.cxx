/**
 * @file EcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Ecal/EcalDigiProducer.h"

namespace ldmx {

    const double EcalDigiProducer::ELECTRONS_PER_MIP = 33000.0; // e-

    const double EcalDigiProducer::MIP_SI_RESPONSE = 0.130; // MeV

    EcalDigiProducer::EcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
        noiseGenerator_ = std::make_unique<NoiseGenerator>();
        noiseInjector_ = std::make_unique<TRandom3>(time(nullptr));
    }

    EcalDigiProducer::~EcalDigiProducer() {
        if ( ecalDigis_ ) delete ecalDigis_;
    }

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

        ecalDigis_ = new TClonesArray(EventConstants::ECAL_HIT.c_str(), 10000);
    }

    void EcalDigiProducer::produce(Event& event) {

        TClonesArray* ecalSimHits = (TClonesArray*) event.getCollection(EventConstants::ECAL_SIM_HITS);
        int numEcalSimHits = ecalSimHits->GetEntries();

        //create empty buffer to store measurements in

        //First we emulate the measurement process by constructing
        //  a pulse from the timing/energy info and then measuring
        //  it at 25ns increments
        std::map< int , std::vector<double> > detID_ADCBuffer;
        for (int iHit = 0; iHit < numEcalSimHits; iHit++) {
            
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) ecalSimHits->At(iHit);

            //create pulse from timing/energy information
            double amplitude = simHit->getEdep();
            double peakTime  = simHit->getTime();
            
            //measure pulse at 25ns increments
            
            //add measurements to buffer
            
        }

        //iterate through all channels and simulate noise on top of everything

        //take measurement of all channels and put into ecalDigis_ array
        
        //EcalHit* digiHit = (EcalHit*) (ecalDigis_->ConstructedAt(iHit));

        event.add("ecalDigis", ecalDigis_);
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
