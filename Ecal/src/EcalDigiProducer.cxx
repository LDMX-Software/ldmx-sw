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

    void EcalDigiProducer::configure(Parameters& ps) {

        gain_            = ps.getParameter<double>("gain");
        pedestal_        = ps.getParameter<double>("pedestal");
        noiseIntercept_  = ps.getParameter<double>("noiseIntercept");
        noiseSlope_      = ps.getParameter<double>("noiseSlope");
        padCapacitance_  = ps.getParameter<double>("padCapacitance");
        nADCs_           = ps.getParameter<int>("nADCs");

        // Calculate the noise RMS based on the properties of the readout pad
        noiseRMS_ = this->calculateNoise(padCapacitance_, noiseIntercept_, noiseSlope_);  

        // Convert the noise RMS in electrons to energy
        noiseRMS_ = noiseRMS_*(MIP_SI_RESPONSE/ELECTRONS_PER_MIP); 

        // Calculate the readout threhsold
        readoutThreshold_ = ps.getParameter<double>("readoutThreshold")*noiseRMS_;

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

        //Option to make configuration histograms
        makeConfigHists_ = ps.getParameter<bool>("makeConfigHists");
        if ( makeConfigHists_ ) {
            getHistoDirectory();

            int nbinsSimE = 33;
            double binsSimE[34] = {
                0.,
                1e-3,
                1e-2, 2e-2, 3e-2, 4e-2, 5e-2, 6e-2, 7e-2, 8e-2, 9e-2,
                1e-1, 2e-1, 3e-1, 4e-1, 5e-1, 6e-1, 7e-1, 8e-1, 9e-1,
                1., 2., 3., 4., 5., 6., 7., 8., 9.,
                1e1, 2e1, 3e1, 4e1, 5e1
            };
            tot_SimE_ = new TH2F( "tot_SimE_" , ";TOT (Clock Counts);Sim E [MeV]",
                    nADCs_*1024, 0 , nADCs_*1024,
                    nbinsSimE , binsSimE
                    );
        }
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

                if ( contrib.time < 0 or contrib.time > EcalDigiProducer::CLOCK_CYCLE*nADCs_ ) {
                    //invalid contribution - outside time range or time is unset
                    continue;
                }

                energyInWindow += contrib.edep;
                timeInWindow   += contrib.edep * contrib.time;
            }
            if ( energyInWindow > 0. ) timeInWindow /= energyInWindow; //energy weighted average

            //put noise onto pulse parameters
            energyInWindow += noiseInjector_->Gaus( 0.0 , noiseRMS_/gain_ );
            timeInWindow   += noiseInjector_->Gaus( 0.0 , EcalDigiProducer::CLOCK_CYCLE / 100. );
            if ( energyInWindow < 0. ) continue; //skip this sim hit TODO this better
            if ( timeInWindow   < 0. ) continue; //skip this sim hit TODO this better

            pulseFunc_.SetParameter( 0 , gain_*energyInWindow ); //set amplitude to gain * energy
            pulseFunc_.SetParameter( 4 , timeInWindow ); //set time of peak to simulated hit time

            //measure TOA and TOT
            double toa = pulseFunc_.GetX(readoutThreshold_, -nADCs_*EcalDigiProducer::CLOCK_CYCLE, timeInWindow);
            double tut = pulseFunc_.GetX(readoutThreshold_, timeInWindow, nADCs_*EcalDigiProducer::CLOCK_CYCLE);
            double tot = tut - toa;

            ldmx_log(debug) << energyInWindow << " MeV at " << timeInWindow << " ns --> " << tot << " TOT " << toa << " TOA " << tut << " TUT";

            int    hitID     = simHit.getID();
            simHitIDs.insert( hitID );
            
            int totalClockCounts = tot*(1024/25.); //conversion from ns to clock counts (converting to int implicitly)

            //TODO currently using adc_t_ to count number of samples that clock is over threshold
            digiToAdd[0].rawID_   = hitID;
            digiToAdd[0].adc_t_   = totalClockCounts / 1024; //Place Holder - not actually how response works
            digiToAdd[0].adc_tm1_ = -1; //NOT IMPLEMENTED
            digiToAdd[0].tot_     = totalClockCounts % 1024; //clock counts since last trigger clock (25ns clock)
            digiToAdd[0].toa_     = toa * (1024/25.); //conversion from ns to clock counts

            ldmx_log(debug) << totalClockCounts << " TOT --> " << digiToAdd[0].adc_t_ << " Clocks and " << digiToAdd[0].tot_ << " TOT";

            ecalDigis.addDigi( digiToAdd );

            if ( makeConfigHists_ ) {
                tot_SimE_->Fill( totalClockCounts , energyInWindow );
            }
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
            double tot = tut - toa;

            int totalClockCounts = tot*(1024/25.); //conversion from ns to clock counts (converting to int implicitly)

            //TODO currently using adc_t_ to count number of samples that clock is over threshold
            digiToAdd[0].rawID_   = noiseID;
            digiToAdd[0].adc_t_   = totalClockCounts / 1024; //Place Holder - not actually how response works
            digiToAdd[0].adc_tm1_ = -1; //NOT IMPLEMENTED
            digiToAdd[0].tot_     = totalClockCounts % 1024; //clock counts since last trigger clock (25ns clock)
            digiToAdd[0].toa_     = toa * (1024/25.); //conversion from ns to clock counts

            ecalDigis.addDigi( digiToAdd );
        }

        event.add("EcalDigis", ecalDigis );

        return;
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
