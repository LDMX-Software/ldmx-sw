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

        // Convert the noise RMS in electrons to energy
        noiseRMS_ = noiseRMS_*(MIP_SI_RESPONSE/ELECTRONS_PER_MIP); 

        // Calculate the readout threhsold
        readoutThreshold_ = ps.getDouble("readoutThreshold", 4.)*noiseRMS_;

        noiseGenerator_->setNoise(noiseRMS_); 
        noiseGenerator_->setPedestal(0); 
        noiseGenerator_->setNoiseThreshold(readoutThreshold_); 

        noiseInjector_->SetSeed(0);

        pulseFunc_ = TF1(
                "pulseFunc",
                "[0]/(1.0+exp([1]*(x-[2]+[3]-[4])))/(1.0+exp([5]*(x-[6]+[3]-[4])))",
                0.0,(double) nADCs_*EcalDigiProducer::CLOCK_CYCLE
                );
        pulseFunc_.SetParameter( 1 , -0.345   );
        pulseFunc_.SetParameter( 2 , 70.6547  );
        pulseFunc_.SetParameter( 3 , 77.732   );
        pulseFunc_.SetParameter( 5 , 0.140068 );
        pulseFunc_.SetParameter( 6 , 87.7649  );

    }

    void EcalDigiProducer::produce(Event& event) {

        //get simulated ecal hits from Geant4
        //  the class EcalHitIO in the SimApplication module handles the translation from G4CalorimeterHits to SimCalorimeterHits
        //  this class ensures that only one SimCalorimeterHit is generated per cell, but
        //  multiple "contributions" are still handled within SimCalorimeterHit 
        std::vector<SimCalorimeterHit> ecalSimHits = event.getCollection<SimCalorimeterHit>(EventConstants::ECAL_SIM_HITS);

        //Empty collection to be filled
        EcalDigiCollection ecalDigis;
        //TODO: number samples per digi probably more than 1
        ecalDigis.setNumSamplesPerDigi( 1 ); 

        std::vector<EcalDigiSample> digiToAdd( 1 , EcalDigiSample() );
        std::set<int> simHitIDs;
        for (const SimCalorimeterHit &simHit : ecalSimHits ) {
            
            //First we emulate the ROC response by constructing
            //  a pulse from the timing/energy info and then measuring
            //  it at 25ns increments
            //total energy and average tiem of contribs inside timing window
            double energyInWindow = 0.0;
            double timeInWindow   = 0.0;
            for ( int iContrib = 0; iContrib < simHit.getNumberOfContribs(); iContrib++ ) {

                SimCalorimeterHit::Contrib contrib = simHit.getContrib( iContrib );

                if ( contrib.time == 0 or contrib.time > EcalDigiProducer::CLOCK_CYCLE*nADCs_ ) {
                    //invalid contribution - outside time range or time is unset
                    continue;
                }

                energyInWindow += contrib.edep;
                timeInWindow   += contrib.edep * contrib.time;
            }
            timeInWindow /= energyInWindow; //energy weighted average

            //put noise onto pulse parameters
            energyInWindow += noiseInjector_->Gaus( 0.0 , noiseRMS_/gain_ );
            timeInWindow   += noiseInjector_->Gaus( 0.0 , EcalDigiProducer::CLOCK_CYCLE / 10. );

            pulseFunc_.SetParameter( 0 , gain_*energyInWindow ); //set amplitude to gain * energy
            pulseFunc_.SetParameter( 4 , timeInWindow ); //set time of peak to simulated hit time

            //measure TOA and TOT
            double toa = pulseFunc_.GetX(readoutThreshold_, 0.0, timeInWindow);
            double tut = pulseFunc_.GetX(readoutThreshold_, timeInWindow, nADCs_*EcalDigiProducer::CLOCK_CYCLE);
            double tot = toa - tut;

            int    hitID     = simHit.getID();
            simHitIDs.insert( hitID );
            
            digiToAdd[0].rawID_   = hitID;
            digiToAdd[0].adc_t_   = -1; //NOT IMPLEMENTED
            digiToAdd[0].adc_tm1_ = -1; //NOT IMPLEMENTED
            digiToAdd[0].tot_     = tot / pow( 2 , 10. );
            digiToAdd[0].toa_     = toa / pow( 2 , 10. );

            ecalDigis.addDigi( digiToAdd );
        }

        //put noise into some empty channels
        int numEmptyChannels = TOTAL_NUM_CHANNELS - ecalDigis.getNumDigis();
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
            } while ( simHitIDs.find( noiseID ) != simHitIDs.end() );

            double hitTime = noiseInjector_->Uniform( EcalDigiProducer::CLOCK_CYCLE );
            pulseFunc_.SetParameter( 0 , gain_*noiseHit ); //set amplitude to gain * energy
            pulseFunc_.SetParameter( 4 , hitTime ); //set time of peak to simulated hit time

            //measure TOA and TOT
            double toa = pulseFunc_.GetX(readoutThreshold_, 0.0, hitTime );
            double tut = pulseFunc_.GetX(readoutThreshold_, hitTime, nADCs_*EcalDigiProducer::CLOCK_CYCLE);
            double tot = toa - tut;

            digiToAdd[0].rawID_   = noiseID;
            digiToAdd[0].adc_t_   = -1; //NOT IMPLEMENTED
            digiToAdd[0].adc_tm1_ = -1; //NOT IMPLEMENTED
            digiToAdd[0].tot_     = tot / pow( 2 , 10. );
            digiToAdd[0].toa_     = toa / pow( 2 , 10. );

            ecalDigis.addDigi( digiToAdd );
        }

        event.add("EcalDigis", ecalDigis );

        return;
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
