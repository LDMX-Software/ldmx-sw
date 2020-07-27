/**
 * @file EcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
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
        //  used  in actual digitization
        gain_             = ps.getParameter<double>("gain");
        pedestal_         = ps.getParameter<double>("pedestal");
        noiseRMS_         = ps.getParameter<double>("noiseRMS");
        readoutThreshold_ = ps.getParameter<double>("readoutThreshold");
        toaThreshold_     = ps.getParameter<double>("toaThreshold");
        totThreshold_     = ps.getParameter<double>("totThreshold");
        timingJitter_     = ps.getParameter<double>("timingJitter");
        clockCycle_       = ps.getParameter<double>("clockCycle");
        nADCs_            = ps.getParameter<int>("nADCs");
        iSOI_             = ps.getParameter<int>("iSOI");

        // physical constants
        //  used to calculate unit conversions
        MeV_ = ps.getParameter<double>("MeV");

        //Time -> clock counts conversion
        //  time [ns] * ( 2^10 / max time in ns ) = clock counts
        ns_ = 1024./clockCycle_;

        // geometry constants
        //  These are used in the noise generation so that we can randomly
        //  distribute the noise uniformly throughout the ECal channels.
        nEcalLayers_      = ps.getParameter<int>("nEcalLayers");
        nModulesPerLayer_ = ps.getParameter<int>("nModulesPerLayer");
        nCellsPerModule_  = ps.getParameter<int>("nCellsPerModule");
        nTotalChannels_   = nEcalLayers_*nModulesPerLayer_*nCellsPerModule_;

        // Configure generator that will produce noise hits in empty channels
        noiseGenerator_->setNoise(noiseRMS_); //rms noise in mV
        noiseGenerator_->setPedestal(gain_*pedestal_); //mean noise amplitude (if using Gaussian Model for the noise) in mV
        noiseGenerator_->setNoiseThreshold(readoutThreshold_); //threshold for readout in mV

        //The noise injector is used to place smearing on top
        //of energy depositions and hit times before doing
        //the digitization procedure.
        noiseInjector_ = std::make_unique<TRandom3>(ps.getParameter<int>("randomSeed"));

        // Configure the pulse shape function
        pulseFunc_ = TF1(
                "pulseFunc",
                "[0]/(1.0+exp([1]*(x-[2]+[3]-[4])))/(1.0+exp([5]*(x-[6]+[3]-[4])))",
                0.0,(double) nADCs_*clockCycle_
                );
        pulseFunc_.SetParameter( 1 , -0.345   );
        pulseFunc_.SetParameter( 2 , 70.6547  );
        pulseFunc_.SetParameter( 3 , 77.732   );
        pulseFunc_.SetParameter( 5 , 0.140068 );
        pulseFunc_.SetParameter( 6 , 87.7649  );

        //Option to make configuration histograms
        makeConfigHists_ = ps.getParameter<bool>("makeConfigHists");
    }

    void EcalDigiProducer::produce(Event& event) {

        //get simulated ecal hits from Geant4
        //  the class EcalHitIO in the SimApplication module handles the translation from G4CalorimeterHits to SimCalorimeterHits
        //  this class ensures that only one SimCalorimeterHit is generated per cell, but
        //  multiple "contributions" are still handled within SimCalorimeterHit 
        auto ecalSimHits{event.getCollection<SimCalorimeterHit>(EventConstants::ECAL_SIM_HITS)};

        //Empty collection to be filled
        HgcrocDigiCollection ecalDigis;
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

            std::vector<HgcrocDigiCollection::Sample> digiToAdd;
            if ( constructDigis( energyDepositions , simulatedTimes , digiToAdd ) ) {
                for ( auto& sample : digiToAdd ) sample.rawID_ = hitID;
                ecalDigis.addDigi( digiToAdd );
            }
        }

        //put noise into some empty channels
        EcalID detID;
        int numEmptyChannels = nTotalChannels_ - ecalDigis.getNumDigis(); //minus number of channels with a hit
        //noise generator gives us a list of noise amplitudes [mV] that randomly populate the empty
        //channels and are above the readout threshold
        auto noiseHitAmplitudes{noiseGenerator_->generateNoiseHits(numEmptyChannels)};
        for ( double noiseHit : noiseHitAmplitudes ) {

            //generate detector ID for noise hit
            //making sure that it is in an empty channel
            int noiseID;
            do {
                int layerID = noiseInjector_->Integer(nEcalLayers_);
                int moduleID= noiseInjector_->Integer(nModulesPerLayer_);
                int cellID  = noiseInjector_->Integer(nCellsPerModule_);
		        detID=EcalID(layerID, moduleID, cellID);
                noiseID = detID.raw();
            } while ( simHitIDs.find( noiseID ) != simHitIDs.end() );

            //get a time for this noise hit
            double hitTime = noiseInjector_->Uniform( clockCycle_ );

            //converting the amplitude to MeV and then using the same digitization emulation
            //as real hits is a worse-case scenario. In reality, these noise hits would
            //probably be randomly distributed throughout digi samples instead of forming
            //a coherent (although small) pulse shape.
            std::vector<HgcrocDigiCollection::Sample> digiToAdd;
            std::vector<double> noiseEnergies( 1 , noiseHit/MeV_ ), noiseTimes( 1 , hitTime );
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
            std::vector<HgcrocDigiCollection::Sample> &digiToAdd) {

            digiToAdd.clear(); //make sure it is clean
            digiToAdd.resize( nADCs_ ); //fill with required number of samples (default constructed)

            //First we emulate the ROC response by constructing
            //  a pulse from the timing/energy info and then measuring
            //  it at 25ns increments
            //total energy and average tiem of contribs inside timing window
            double energyInWindow = 0.0;
            double timeInWindow   = 0.0;
            for ( int iContrib = 0; iContrib < energies.size(); iContrib++ ) {

                if ( times.at(iContrib) < 0 or times.at(iContrib) > clockCycle_*nADCs_ ) {
                    //invalid contribution - outside time range or time is unset
                    continue;
                }

                energyInWindow += energies.at(iContrib);
                timeInWindow   += energies.at(iContrib) * times.at(iContrib);
            }
            if ( energyInWindow > 0. ) timeInWindow /= energyInWindow; //energy weighted average

            // put noise onto timing
            //TODO more physical way of simulating the timing jitter
            timeInWindow += noiseInjector_->Gaus( 0. , timingJitter_ );

            //set time in the window to zero if noise pushed it below zero
            //TODO better (more physical) method for handling this case?
            if ( timeInWindow   < 0. ) timeInWindow = 0.;

            //setup up pulse by changing the amplitude and timing parameters
            //  amplitude is gain times input voltage
            double signalAmplitude = energyInWindow*MeV_; //now this is the amplitude in mV
            pulseFunc_.SetParameter( 0 , signalAmplitude ); 
            pulseFunc_.SetParameter( 4 , timeInWindow ); //set time of peak to simulated hit time

            //measure pulse with option to include noise on top
            //  returns height of pulse in mV
            //  this : pointer to this object so lambda function has access to its members
            //  time : time [ns] to measure height of pulse func at
            //  withNoise : true if you want to include Gaussian noise on top
            auto measurePulse = [this](const double &time, bool withNoise) {
                auto signal = gain_*pedestal_ + pulseFunc_.Eval(time);
                if ( withNoise ) signal += noiseInjector_->Gaus( 0. , noiseRMS_ );
                return signal;
            };

            // choose readout mode
            if ( signalAmplitude < readoutThreshold_ ) {
                //below readout threshold -> skip this hit
                return false;
            } else if ( signalAmplitude < totThreshold_ ) {
                //below TOT threshold -> do ADC readout mode

                //measure time of arrival (TOA) using TOA threshold
                double toa(0.);
                // make sure pulse crosses TOA threshold
                if ( pulseFunc_.Eval(0.) < toaThreshold_ and signalAmplitude > toaThreshold_ ) 
                    toa = pulseFunc_.GetX(toaThreshold_, 0., timeInWindow);

                //measure ADCs
                for ( unsigned int iADC = 0; iADC < digiToAdd.size(); iADC++ ) {
                    double measTime = iADC*clockCycle_; // + offset;
                    digiToAdd[iADC].adc_t_   = measurePulse( measTime, true )/gain_;
                    digiToAdd[iADC].adc_tm1_ = iADC > 0 ? digiToAdd.at(iADC-1).adc_t_ : pedestal_; 
                    digiToAdd[iADC].toa_     = toa * ns_;
                    digiToAdd[iADC].tot_progress_ = false;
                    digiToAdd[iADC].tot_complete_ = false;
                }

            } else {
                // above TOT threshold -> do TOT readout mode
    
                //measure time of arrival (TOA) and time under threshold (TUT) from pulse
                //  TOA: earliest possible measure for crossing TOT threshold line
                //  TUT: latest possible measure for crossing TOT threshold line
                //TODO make the TOT measurement more realistic
                //  in reality, the pulse drastically changes shape when the chip goes
                //  into saturation. The charge draining after saturation slows down
                //  and makes the TOT <-> energy deposited conversion closer to linear
                
                double toa(0.); //default is earliest possible time
                // check if first half is just always above readout
                if ( pulseFunc_.Eval(0.) < totThreshold_ ) 
                    toa = pulseFunc_.GetX(totThreshold_, 0., timeInWindow);
    
                double tut(nADCs_*clockCycle_); //default is latest possible time
                // check if second half is just always above readout
                if ( pulseFunc_.Eval( nADCs_*clockCycle_ ) > totThreshold_ )
                    tut = pulseFunc_.GetX(totThreshold_, timeInWindow, nADCs_*clockCycle_);
    
                double tot = tut - toa;

                if ( makeConfigHists_ ) histograms_.fill( "tot_SimE" , tot , energyInWindow );
    
                /*
                std::cout << std::setw(6)
                    << energyInWindow << " MeV at " << timeInWindow << " ns --> "
                    << tot << " TOT " << toa << " TOA " << tut << " TUT" << std::endl;
                */
    
                for ( unsigned int iADC = 0; iADC < digiToAdd.size(); iADC++ ) {
                    if ( tot > clockCycle_ or tot < 0 ) {
                        //TOT still in progress or already completed
                        double measTime = iADC*clockCycle_; // + offset;
                        digiToAdd[iADC].adc_t_   = measurePulse( measTime, true )/gain_;
                        digiToAdd[iADC].adc_tm1_ = iADC > 0 ? digiToAdd.at(iADC-1).adc_t_ : pedestal_;
                        digiToAdd[iADC].toa_     = toa*ns_;
                        digiToAdd[iADC].tot_progress_ = true;
                        digiToAdd[iADC].tot_complete_ = false;
                    } else {
                        //TOT complete
                        digiToAdd[iADC].adc_tm1_ = iADC > 0 ? digiToAdd.at(iADC-1).adc_t_ : pedestal_;
                        digiToAdd[iADC].tot_     = tot*ns_;
                        digiToAdd[iADC].toa_     = toa*ns_;
                        digiToAdd[iADC].tot_progress_ = false;
                        digiToAdd[iADC].tot_complete_ = true;
                    }
                    tot -= clockCycle_; //decrement TOT
                }
    
                /*
                std::cout << std::setw(6) << totalClockCounts
                    << " TOT --> " << digiToAdd[iSOI_].adc_t_ << " Clocks and "
                    << digiToAdd[iSOI_].tot_ << " tot" << std::endl;
                */
    
            }

            return true;
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalDigiProducer);
