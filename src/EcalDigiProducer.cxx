/**
 * @file EcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 */

#include "Ecal/EcalDigiProducer.h"

namespace ldmx {

    EcalDigiProducer::EcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {

        //noise generator by default uses a Gausian model for noise
        //  i.e. It assumes the noise is distributed around a mean (setPedestal)
        //  with a certain RMS (setNoise) and then calculates
        //  how many hits should be generated for a given number of empty
        //  channels and a minimum readout value (setNoiseThreshold)
        noiseGenerator_ = std::make_unique<NoiseGenerator>();
    }

    EcalDigiProducer::~EcalDigiProducer() { }

    void EcalDigiProducer::configure(Parameters& ps) {

        //settings of readout chip
        gain_              = ps.getParameter<double>("gain");
        pedestal_          = ps.getParameter<double>("pedestal");
        noiseRMS_          = ps.getParameter<double>("noiseRMS");
        readoutThreshold_  = ps.getParameter<double>("readoutThreshold");
        nADCs_             = ps.getParameter<int>("nADCs");
        iSOI_              = ps.getParameter<int>("iSOI");
        CLOCK_CYCLE        = ps.getParameter<double>("CLOCK_CYCLE");

        // physical constants
        //  use in the MIP <-> electrons <-> energy conversions
        ELECTRONS_PER_MIP  = ps.getParameter<double>("ELECTRONS_PER_MIP");
        MIP_SI_RESPONSE    = ps.getParameter<double>("MIP_SI_RESPONSE");

        // geometry constants
        //  These are used in the noise generation so that we can randomly
        //  distribute the noise uniformly throughout the ECal channels.
        NUM_ECAL_LAYERS           = ps.getParaemter<int>("NUM_ECAL_LAYERS");
        NUM_HEX_MODULES_PER_LAYER = ps.getParameter<int>("NUM_HEX_MODULES_PER_LAYER");
        CELLS_PER_HEX_MODULE      = ps.getParameter<int>("CELLS_PER_HEX_MODULE");

        // Configure generator that will produce noise hits in empty channels
        noiseGenerator_->setNoise(noiseRMS_); //rms noise in MeV
        noiseGenerator_->setPedestal(0); //mean noise amplitude (if using Gaussian Model for the noise) in MeV
        noiseGenerator_->setNoiseThreshold(readoutThreshold_); //threshold for readout in MeV

        //The noise injector is used to place smearing on top
        //of energy depositions and hit times before doing
        //the digitization procedure.
        noiseInjector_ = std::make_unique<TRandom3>(ps.getParameter<int>("randomSeed"));

        // Configure the pulse shape function
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
        EcalID detID;
        auto noiseHitAmplitudes{noiseGenerator_->generateNoiseHits(numEmptyChannels)};
        for ( double noiseHit : noiseHitAmplitudes ) {

            //generate detector ID for noise hit
            //making sure that it is in an empty channel
            int noiseID;
            do {
                int layerID = noiseInjector_->Integer(NUM_ECAL_LAYERS);
                int moduleID= noiseInjector_->Integer(NUM_HEX_MODULES_PER_LAYER);
                int cellID  = noiseInjector_->Integer(CELLS_PER_HEX_MODULE);
		        detID=EcalID(layerID, moduleID, cellID);
                noiseID = detID.raw();
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
            energyInWindow += noiseInjector_->Gaus( 0.0 , noiseRMS_ );
            timeInWindow   += noiseInjector_->Gaus( 0.0 , timingJitter_ );

            //set time in the window to zero if noise pushed it below zero
            //TODO better (more physical) method for handling this case?
            if ( timeInWindow   < 0. ) timeInWindow = 0.;

            //setup up pulse by changing the amplitude and timing parameters
            //  amplitude is gain times number of electrons generated
            //  number of electrons is calculated by converting energy to MIPs and then MIPs to electrons
            pulseFunc_.SetParameter( 0 , gain_*(ELECTRONS_PER_MIP/MIP_SI_RESPONSE)*energyInWindow ); 
            pulseFunc_.SetParameter( 4 , timeInWindow ); //set time of peak to simulated hit time

            // choose readout mode
            if ( energyInWindow < readoutThreshold_ ) {
                //below readout threshold -> skip this hit
                return false;
            } else if ( energyInWindow < totThreshold_ ) {
                //below TOT threshold -> do ADC readout mode

                //measure time of arrival (TOA) using TOA threshold
                double toa(0.);
                // check if first half is just always above readout or if doesn't go above TOA threshold
                if ( pulseFunc_.Eval(0.) > toaThreshold_ or energyInWindow < toaThreshold_ ) 
                    toa = 0.; //toa is beginning of readout interval
                else
                    toa = pulseFunc_.GetX(toaThreshold_, 0., timeInWindow);

                //measure ADCs
                for ( unsigned int iADC = 0; iADC < digiToAdd.size(); iADC++ ) {
                    double measTime = iADC*CLOCK_CYCLE; // + offset;
                    digiToAdd[iADC].adc_t_   = pulseFunc_.Eval( measTime );
                    digiToAdd[iADC].adc_tm1_ = -1; //TODO set this up
                    digiToAdd[iADC].toa_     = toa * (1024/CLOCK_CYCLE);
                    digiToAdd[iADC].tot_     = 0;
                    digiToAdd[iADC].tot_progress_ = false;
                    digiToAdd[iADC].tot_complete_ = false;
                }

            } else {
                // above TOT threshold -> do TOT readout mode
    
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
                digitoAdd[iSOI_].tot_progress_ = false;
                digitoAdd[iSOI_].tot_complete_ = true ;
    
                /*
                std::cout << std::setw(6) << totalClockCounts
                    << " TOT --> " << digiToAdd[iSOI_].adc_t_ << " Clocks and "
                    << digiToAdd[iSOI_].tot_ << " tot" << std::endl;
                */
    
                if ( makeConfigHists_ ) {
                    tot_SimE_->Fill( totalClockCounts , energyInWindow );
                }
            }

            return true;
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
