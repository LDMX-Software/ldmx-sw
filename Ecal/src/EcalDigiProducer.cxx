/**
 * @file EcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "Ecal/EcalDigiProducer.h"

namespace ldmx {

    const double EcalDigiProducer::ELECTRONS_PER_MIP = 33000.0; // e-
    const double EcalDigiProducer::CLOCK_CYCLE = 25.0; // ns
    const double EcalDigiProducer::MIP_SI_RESPONSE = 0.130; // MeV
    const int    EcalDigiProducer::NUM_ECAL_LAYERS = 34;
    const int    EcalDigiProducer::NUM_HEX_MODULES_PER_LAYER = 7;
    const int    EcalDigiProducer::CELLS_PER_HEX_MODULE = 397;
    const int    EcalDigiProducer::TOTAL_NUM_CHANNELS = NUM_ECAL_LAYERS*NUM_HEX_MODULES_PER_LAYER*CELLS_PER_HEX_MODULE;

    EcalDigiProducer::EcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
        noiseGenerator_ = std::make_unique<NoiseGenerator>();
        noiseInjector_ = std::make_unique<TRandom3>(time(nullptr));
    }

    EcalDigiProducer::~EcalDigiProducer() {
    }

    void EcalDigiProducer::configure(const ParameterSet& ps) {

        hexReadout_ = std::make_unique<EcalHexReadout>();

        gain_           = ps.getDouble("gain", 2000.); 
        pedestal_       = ps.getDouble("pedestal", 1100.); 
        noiseIntercept_ = ps.getDouble("noiseIntercept", 700.); 
        noiseSlope_     = ps.getDouble("noiseSlope", 25.);
        padCapacitance_ = ps.getDouble("padCapacitance", 0.1); 
        nADCs_          = ps.getInteger("nADCs", 10);

        // Calculate the noise RMS based on the properties of the readout pad
        noiseRMS_ = this->calculateNoise(padCapacitance_, noiseIntercept_, noiseSlope_);  
        //std::cout << "[ EcalDigiProducer ]: Noise RMS: " << noiseRMS_ << " e-" << std::endl;

        // Convert the noise RMS in electrons to energy
        noiseRMS_ = noiseRMS_*(MIP_SI_RESPONSE/ELECTRONS_PER_MIP); 
        //std::cout << "[ EcalDigiProducer ]: Noise RMS: " << noiseRMS_ << " MeV" << std::endl;

        // Calculate the readout threhsold
        readoutThreshold_ = ps.getDouble("readoutThreshold", 4.)*noiseRMS_;
        //std::cout << "[ EcalDigiProducer ]: Readout threshold: " << readoutThreshold_ << " MeV" << std::endl;

        noiseGenerator_->setNoise(noiseRMS_); 
        noiseGenerator_->setPedestal(0); 
        noiseGenerator_->setNoiseThreshold(readoutThreshold_); 

        noiseInjector_->SetSeed(0);

        pulseFunc_ = TF1(
                "pulseFunc",
                "[1]/(1.0+exp(-0.345*(x-70.6547+77.732-[0])))/(1.0+exp(0.140068*(x-87.7649+77.732-[0])))",
                0.0,(double) nADCs_*EcalDigiProducer::CLOCK_CYCLE
                );

        ecalDigis_ = new EcalDigiCollection();
        ecalDigis_->setNumSamplesPerDigi( 1 );
    }

    void EcalDigiProducer::produce(Event& event) {

        //Clean up last event
        ecalDigis_->Clear();

        //get simulated ecal hits from Geant4
        TClonesArray* ecalSimHits = (TClonesArray*) event.getCollection(EventConstants::ECAL_SIM_HITS);
        int numEcalSimHits = ecalSimHits->GetEntries();

        //First we emulate the ROC response by constructing
        //  a pulse from the timing/energy info and then measuring
        //  it at 25ns increments
        //Currently, using list of energies to calculate timing of hit and TOT measurement in linear scale
        //  TODO: implement pulse measurement simulation
        std::map< int , std::vector<double> > adcBuffers;
        std::map< int , std::vector<double> > energyBuffers;
        std::map< int , std::vector<double> > timeBuffers;
        for (int iHit = 0; iHit < numEcalSimHits; iHit++) {
            
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) ecalSimHits->At(iHit);

            //create pulse from timing/energy information
            int    hitID     = simHit->getID();
            double hitEnergy = simHit->getEdep();
            double hitTime   = simHit->getTime();
            pulseFunc_.SetParameters( hitTime , gain_*hitEnergy );

            //init buffer for this detID if it doesn't exist yet
            if( adcBuffers.find(hitID) == adcBuffers.end() ) 
            {
                std::vector<double> adcBuff(nADCs_,pedestal_);
                adcBuffers.insert({hitID,adcBuff});
                std::vector<double> energyBuff;
                energyBuffers.insert({hitID,energyBuff});
                std::vector<double> timeBuff;
                timeBuffers.insert({hitID,timeBuff});
            }
            energyBuffers[hitID].push_back(hitEnergy);
            timeBuffers[hitID].push_back(hitTime);

            //measure pulse at 25ns increments and add measurements to buffer
            for (int ss = 0; ss < nADCs_; ss++)
            {
                double pulseEval = pulseFunc_.Eval((double) ss*EcalDigiProducer::CLOCK_CYCLE);
                adcBuffers[hitID].at(ss) += pulseEval;
            }

            //measure TOA and TOT
            double toa = pulseFunc_.GetX(readoutThreshold_, 0.0, hitTime);
            double tut = pulseFunc_.GetX(readoutThreshold_, hitTime, nADCs_*EcalDigiProducer::CLOCK_CYCLE);
            double tot = toa - tut;

        }
        
        //iterate through all channels and simulate noise on top of everything and build digi
        int iHit = 0;
        std::map< int , std::vector<double> >::iterator it;
        EcalDigiSample sampleToAdd;
        std::vector<EcalDigiSample> digisToAdd( ecalDigis_->getNumSamplesPerDigi() , sampleToAdd );
        for ( it = adcBuffers.begin(); it != adcBuffers.end(); it++ )
        {

            int detID = it->first;
            std::vector<double> buff = it->second;
            for(int ss = 0; ss < buff.size(); ss++)
            {
                //add noise in Gaussian distributed ADC units
                buff[ss] += noiseInjector_->Gaus(0.0,noiseRMS_/gain_); 
            }

            std::vector<double> engs = energyBuffers[detID];
            std::vector<double> times = timeBuffers[detID];
            double EtimeSum = 0.0;
            double engTot = 0.0;
            for(int hh = 0; hh < engs.size(); hh++)
            {
                engTot   += engs[hh];
                EtimeSum += engs[hh]*times[hh];
            }

            //construct digi in event bus collection
            //  right now, only creating one digi per hit
            //  TODO: move onto several digis per hit to mimic real DAQ
            sampleToAdd.rawID_ = detID;
            //large smearing on toa to represent bad simulation
            //  toa counts fraction of time within each 25ns window.
            sampleToAdd.toa_   = ( abs(EtimeSum/engTot + noiseInjector_->Gaus( 0.0 , 25.0 ))/25. ) / pow( 2. , 10 );
            //scaling number forces range of 0<->25MeV to 10 bit int
            sampleToAdd.tot_   = (engTot + noiseInjector_->Gaus( 0.0 , noiseRMS_ ) )*41; 
            sampleToAdd.adc_t_ = buff.at(0); //already has noise on top

            digisToAdd[0] = sampleToAdd;
            ecalDigis_->addDigi( digisToAdd );
        }

        //put noise into some empty channels
        int numEmptyChannels = TOTAL_NUM_CHANNELS - ecalDigis_->getNumDigis();
        std::vector<double> noiseHitAmplitudes = noiseGenerator_->generateNoiseHits( numEmptyChannels );
        EcalDetectorID detID;
        for ( double noiseHit : noiseHitAmplitudes ) {

            //generate detector ID for noise hit
            int noiseID;
            do {

                int layerID = noiseInjector_->Integer(NUM_ECAL_LAYERS);
                int moduleID= noiseInjector_->Integer(NUM_HEX_MODULES_PER_LAYER);
                int cellID  = noiseInjector_->Integer(CELLS_PER_HEX_MODULE);
                detID.setFieldValue( 1 , layerID );
                detID.setFieldValue( 2 , moduleID );
                detID.setFieldValue( 3 , cellID );
                noiseID = detID.pack();
            } while ( adcBuffers.count(noiseID) > 0 );

            sampleToAdd.rawID_ = noiseID;
            //large smearing on toa to represent bad simulation
            sampleToAdd.toa_   = noiseInjector_->Integer( 1023 );
            //scaling number forces range of 0<->25MeV to 10 bit int
            sampleToAdd.tot_   = noiseHit*41;
            sampleToAdd.adc_t_ = 1; //arbitrary number - skipping pulse measurement due to lazyness

            digisToAdd[0] = sampleToAdd;
            ecalDigis_->addDigi( digisToAdd );
        }

        event.add("EcalDigis", ecalDigis_ );

        return;
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
