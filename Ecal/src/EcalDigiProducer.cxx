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

        //noise generator by default uses a Gausian model for noise
        //  i.e. It assumes the noise is distributed around a mean (setPedestal)
        //  with a certain RMS (setNoise) and then calculates
        //  how many hits should be generated for a given number of empty
        //  channels and a minimum readout value (setNoiseThreshold)
        noiseGenerator_ = std::make_unique<NoiseGenerator>();

        //The noise injector is used to place smearing on top
        //of energy depositions and hit times before doing
        //the digitization procedure.
        noiseInjector_ = std::make_unique<TRandom3>(time(nullptr));
    }

    EcalDigiProducer::~EcalDigiProducer() { }

    void EcalDigiProducer::configure(Parameters& ps) {

        gain_            = ps.getParameter<double>("gain");
        pedestal_        = ps.getParameter<double>("pedestal");
        noiseIntercept_  = ps.getParameter<double>("noiseIntercept");
        noiseSlope_      = ps.getParameter<double>("noiseSlope");
        padCapacitance_  = ps.getParameter<double>("padCapacitance");
        nADCs_           = ps.getParameter<int>("nADCs");
        iSOI_            = ps.getParameter<int>("iSOI");

        // Calculate the noise RMS based on the properties of the readout pad
        noiseRMS_ = this->calculateNoise(padCapacitance_, noiseIntercept_, noiseSlope_);  

        // Convert the noise RMS in electrons to energy
        noiseRMS_ = noiseRMS_*(MIP_SI_RESPONSE/ELECTRONS_PER_MIP); 

        // Calculate the readout threhsold
        //  the input readoutThreshold is assumed to be multiples of the noiseRMS
        readoutThreshold_ = ps.getParameter<double>("readoutThreshold")*noiseRMS_;

        noiseGenerator_->setNoise(noiseRMS_); //rms noise in MeV
        noiseGenerator_->setPedestal(0); //mean noise amplitude (if using Gaussian Model for the noise) in MeV
        noiseGenerator_->setNoiseThreshold(readoutThreshold_); //threshold for readout in MeV

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
        auto ecalSimHits{event.getCollection<SimCalorimeterHit>(EventConstants::ECAL_SIM_HITS)};

        //Empty collection to be filled
        EcalDigiCollection ecalDigis;
        ecalDigis.setNumSamplesPerDigi( nADCs_ ); 
        ecalDigis.setSampleOfInterestIndex( iSOI_ );

        std::set<int> simHitIDs;
        for (auto const& simHit : ecalSimHits ) {

            std::vector<double> energyDepositions, simulatedTimes;
            for ( int iContrib = 0; iContrib < simHit.getNumberOfContribs(); iContrib++ ) {
                energyDepositions.push_back( simHit.getContrib( iContrib ).edep );
                simulatedTimes.push_back( simHit.getContrib( iContrib ).time );
            }

            int hitID = simHit.getID();
            simHitIDs.insert( hitID );

            std::vector<EcalDigiSample> digiToAdd;
            if ( constructDigis( energyDepositions , simulatedTimes , digiToAdd ) ) {
                for ( auto& sample : digiToAdd ) sample.rawID_ = hitID;
                ecalDigis.addDigi( digiToAdd );
            }
        }

        //put noise into some empty channels
        int numEmptyChannels = TOTAL_NUM_CHANNELS - ecalDigis.getNumDigis();
        EcalDetectorID detID;
        auto noiseHitAmplitudes{noiseGenerator_->generateNoiseHits(numEmptyChannels)};
        for ( double noiseHit : noiseHitAmplitudes ) {

            //generate detector ID for noise hit
            //making sure that it is in an empty channel
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

            //get a time for this noise hit
            double hitTime = noiseInjector_->Uniform( EcalDigiProducer::CLOCK_CYCLE );

            std::vector<EcalDigiSample> digiToAdd;
            std::vector<double> noiseEnergies( 1 , noiseHit ), noiseTimes( 1 , hitTime );
            if ( constructDigis( noiseEnergies , noiseTimes , digiToAdd ) ) {
                for ( auto& sample : digiToAdd ) sample.rawID_ = noiseID;
                ecalDigis.addDigi( digiToAdd );
            }

        }

        event.add("EcalDigis", ecalDigis );

        return;
    } //produce

    bool EcalDigiProducer::constructDigis(
            const std::vector<double> &energies, 
            const std::vector<double> &times, 
            std::vector<EcalDigiSample> &digiToAdd) {

            digiToAdd.clear(); //make sure it is clean
            digiToAdd.resize( nADCs_ ); //fill with required number of samples (default constructed)

            //First we emulate the ROC response by constructing
            //  a pulse from the timing/energy info and then measuring
            //  it at 25ns increments
            //total energy and average tiem of contribs inside timing window
            double energyInWindow = 0.0;
            double timeInWindow   = 0.0;
            for ( int iContrib = 0; iContrib < energies.size(); iContrib++ ) {

                if ( times.at(iContrib) < 0 or times.at(iContrib) > EcalDigiProducer::CLOCK_CYCLE*nADCs_ ) {
                    //invalid contribution - outside time range or time is unset
                    continue;
                }

                energyInWindow += energies.at(iContrib);
                timeInWindow   += energies.at(iContrib) * times.at(iContrib);
            }
            if ( energyInWindow > 0. ) timeInWindow /= energyInWindow; //energy weighted average

            //put noise onto pulse parameters
            //TODO this is putting noise ontop of noise, is this a problem? (I don't think so, but maybe)
            energyInWindow += noiseInjector_->Gaus( 0.0 , noiseRMS_/gain_ );
            //this 100. was chosen arbitrarily
            //TODO choose the width of the timing jitter more realistically --> python parameter?
            timeInWindow   += noiseInjector_->Gaus( 0.0 , EcalDigiProducer::CLOCK_CYCLE / 100. ); 

            //set time in the window to zero if noise pushed it below zero
            //TODO better (more physical) method for handling this case?
            if ( timeInWindow   < 0. ) timeInWindow = 0.;

            //setup up pulse by changing the amplitude and timing parameters
            pulseFunc_.SetParameter( 0 , gain_*energyInWindow ); //set amplitude to gain * energy
            pulseFunc_.SetParameter( 4 , timeInWindow ); //set time of peak to simulated hit time

            //skip the hit if the peak of the pulse after noise is less than readoutThreshold_
            //TODO do ADC counts in this case instead of skipping it
            if ( pulseFunc_.Eval( timeInWindow ) < readoutThreshold_  ) return false; 

            //measure time of arrival (TOA) and time under threshold (TUT) from pulse
            //  TOA: earliest possible measure for crossing threshold line
            //  TUT: latest possible measure for crossing threshold line
            
            double toa(0.);
            // check if first half is just always above readout
            if ( pulseFunc_.Eval( -nADCs_*EcalDigiProducer::CLOCK_CYCLE ) > readoutThreshold_ ) 
                toa = -nADCs_*EcalDigiProducer::CLOCK_CYCLE; //always above --> toa is beginning of readout interval
            else
                toa = pulseFunc_.GetX(readoutThreshold_, -nADCs_*EcalDigiProducer::CLOCK_CYCLE, timeInWindow);

            double tut(0.);
            // check if second half is just always above readout
            if ( pulseFunc_.Eval( nADCs_*EcalDigiProducer::CLOCK_CYCLE ) > readoutThreshold_ )
                tut = nADCs_*EcalDigiProducer::CLOCK_CYCLE; //always above --> tut is end of readout interval
            else
                tut = pulseFunc_.GetX(readoutThreshold_, timeInWindow, nADCs_*EcalDigiProducer::CLOCK_CYCLE);

            double tot = tut - toa;

            /*
            std::cout << std::setw(6)
                << energyInWindow << " MeV at " << timeInWindow << " ns --> "
                << tot << " TOT " << toa << " TOA " << tut << " TUT" << std::endl;
            */

            //conversion from ns to clock counts (converting to int implicitly)
            int totalClockCounts = tot*(1024/EcalDigiProducer::CLOCK_CYCLE); 

            //TODO: should expand digi response to multiple samples instead of just the SOI
            //TODO currently using adc_t_ to count number of samples that clock is over threshold
            //  this is NOT how it is actually done on the chip
            digiToAdd[iSOI_].adc_t_   = totalClockCounts / 1024; //Place Holder - not actually how response works
            digiToAdd[iSOI_].adc_tm1_ = -1; //NOT IMPLEMENTED
            digiToAdd[iSOI_].tot_     = totalClockCounts % 1024; //clock counts since last trigger clock (25ns clock)
            digiToAdd[iSOI_].toa_     = toa * (1024/EcalDigiProducer::CLOCK_CYCLE); //conversion from ns to clock counts

            /*
            std::cout << std::setw(6) << totalClockCounts
                << " TOT --> " << digiToAdd[iSOI_].adc_t_ << " Clocks and "
                << digiToAdd[iSOI_].tot_ << " tot" << std::endl;
            */

            if ( makeConfigHists_ ) {
                tot_SimE_->Fill( totalClockCounts , energyInWindow );
            }

            return true;
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
