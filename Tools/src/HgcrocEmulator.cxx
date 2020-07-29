
#include "Tools/HgcrocEmulator.h"

#include "DetDescr/EcalDetectorID.h"

#include <time.h>

namespace ldmx { 

    HgcrocEmulator::HgcrocEmulator(const Parameters& ps) {


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
        MeV_              = ps.getParameter<double>("MeV");
        nADCs_            = ps.getParameter<int>("nADCs");
        iSOI_             = ps.getParameter<int>("iSOI");

        //Time -> clock counts conversion
        //  time [ns] * ( 2^10 / max time in ns ) = clock counts
        ns_ = 1024./clockCycle_;

        // Configure generator that will produce noise hits in empty channels
        noiseGenerator_->setNoise(noiseRMS_); //rms noise in mV
        noiseGenerator_->setPedestal(gain_*pedestal_); //mean noise amplitude (if using Gaussian Model for the noise) in mV
        noiseGenerator_->setNoiseThreshold(readoutThreshold_); //threshold for readout in mV

        // Configure the pulse shape function
        pulseFunc_ = TF1(
                "pulseFunc",
                "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
                0.0,(double) nADCs_*clockCycle_
                );
        pulseFunc_.FixParameter( 1 , -0.345   );
        pulseFunc_.FixParameter( 2 , 70.6547  );
        pulseFunc_.FixParameter( 3 , 77.732   );
        pulseFunc_.FixParameter( 5 , 0.140068 );
        pulseFunc_.FixParameter( 6 , 87.7649  );

        //geometry stuff for noise simulation
        nEcalLayers_ = ps.getParameter<int>("nEcalLayers");
        nModulesPerLayer_ = ps.getParameter<int>("nModulesPerLayer");
        nCellsPerModule_  = ps.getParameter<int>("nCellsPerModule");
        
        nTotalChannels_ = nEcalLayers_*nModulesPerLayer_*nCellsPerModule_;

        ecal_ = true;

    }

    HgcrocDigiCollection HgcrocEmulator::digitize( const std::vector<SimCalorimeterHit> &simHits ) const {

        //Empty collection to be filled
        HgcrocDigiCollection digitizedHits;
        digitizedHits.setNumSamplesPerDigi( nADCs_ ); 
        digitizedHits.setSampleOfInterestIndex( iSOI_ );

        /******************************************************************************************
         * Chip Emulation
         *****************************************************************************************/

        std::set<int> simHitIDs; //set of hit IDs
        std::vector<HgcrocDigiCollection::Sample> digiToAdd; //object to be changing with each hit
        for (auto const& simHit : simHits ) {

            int hitID = simHit.getID();
            simHitIDs.insert( hitID );

            digiToAdd.clear(); //make sure it is clean
            digiToAdd.resize( nADCs_ ); //fill with required number of samples (default constructed)

            //sum all energy deposited and do an energy-weighted average to get the hit time
            //  exclude any hits with times outside the sampling region
            double energyInWindow = 0.0;
            double timeInWindow   = 0.0;
            for ( int iContrib = 0; iContrib < simHit.getNumberOfContribs(); iContrib++ ) {

                double time = simHit.getContrib( iContrib ).time;
                double edep = simHit.getContrib( iContrib ).edep;

                if ( time  < 0 or time > clockCycle_*nADCs_ ) {
                    //invalid contribution - outside time range or time is unset
                    continue;
                }

                energyInWindow += edep;
                timeInWindow   += edep * time;
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
            double pulsePeak = measurePulse( timeInWindow , false );
            if (verbose_) {
                std::cout << "Pulse: { "
                    << "Amplitude: " << signalAmplitude+gain_*pedestal_ << "mV, "
                    << "Beginning: " << measurePulse(0.,false) << "mV, "
                    << "Time: " << timeInWindow << "ns, "
                    << "Energy: " << energyInWindow << "MeV } -> ";
            }
            if ( pulsePeak < readoutThreshold_ ) {
                //below readout threshold -> skip this hit
                //don't add anything to the digitizedHits collection
                if (verbose_) std::cout << "Below Readout" << std::endl;
            } else if ( pulsePeak < totThreshold_ ) {
                //below TOT threshold -> do ADC readout mode
                if (verbose_) std::cout << "ADC Mode { ";

                //measure time of arrival (TOA) using TOA threshold
                double toa(0.);
                // make sure pulse crosses TOA threshold
                if ( measurePulse(0.,false) < toaThreshold_ and pulsePeak > toaThreshold_ ) {
                    toa = pulseFunc_.GetX(toaThreshold_-gain_*pedestal_, -nADCs_*clockCycle_, timeInWindow);
                }
                if (verbose_) std::cout << "TOA: " << toa << "ns, ";

                //measure ADCs
                for ( unsigned int iADC = 0; iADC < digiToAdd.size(); iADC++ ) {
                    double measTime = iADC*clockCycle_; // + offset;
                    digiToAdd[iADC].adc_t_   = measurePulse( measTime, true )/gain_;
                    digiToAdd[iADC].adc_tm1_ = iADC > 0 ? digiToAdd.at(iADC-1).adc_t_ : pedestal_; 
                    digiToAdd[iADC].toa_     = toa * ns_;
                    digiToAdd[iADC].rawID_   = simHit.getID();
                    digiToAdd[iADC].tot_progress_ = false;
                    digiToAdd[iADC].tot_complete_ = false;
                    if (verbose_) std::cout << " ADC " << iADC << ": " << digiToAdd[iADC].adc_t_*gain_ << "mV, ";
                }
                if (verbose_) std::cout << "}" << std::endl;

                digitizedHits.addDigi( digiToAdd );
            } else {
                // above TOT threshold -> do TOT readout mode
    
                //measure time of arrival (TOA) and time under threshold (TUT) from pulse
                //  TOA: earliest possible measure for crossing TOT threshold line
                //  TUT: latest possible measure for crossing TOT threshold line
                //TODO make the TOT measurement more realistic
                //  in reality, the pulse drastically changes shape when the chip goes
                //  into saturation. The charge draining after saturation slows down
                //  and makes the TOT <-> energy deposited conversion closer to linear
                if (verbose_) std::cout << "TOT Mode { ";

                double toa(0.); //default is earliest possible time
                // check if first half is just always above readout
                if ( measurePulse(0.,false) < totThreshold_ ) 
                    toa = pulseFunc_.GetX(totThreshold_-gain_*pedestal_, 0., timeInWindow);
    
                double tut(nADCs_*clockCycle_); //default is latest possible time
                // check if second half is just always above readout
                if ( pulseFunc_.Eval( nADCs_*clockCycle_ ) < totThreshold_ )
                    tut = pulseFunc_.GetX(totThreshold_-gain_*pedestal_, timeInWindow, nADCs_*clockCycle_);
    
                double tot = tut - toa;

                if (verbose_) std::cout << "TOA: " << toa << "ns, ";
                if (verbose_) std::cout << "TOT: " << tot << "ns} " << std::endl;

                for ( unsigned int iADC = 0; iADC < digiToAdd.size(); iADC++ ) {
                    if ( tot > clockCycle_ or tot < 0 ) {
                        //TOT still in progress or already completed
                        double measTime = iADC*clockCycle_; // + offset;
                        digiToAdd[iADC].adc_t_   = measurePulse( measTime, true )/gain_;
                        digiToAdd[iADC].tot_progress_ = true;
                        digiToAdd[iADC].tot_complete_ = false;
                    } else {
                        //TOT complete
                        digiToAdd[iADC].tot_     = tot*ns_;
                        digiToAdd[iADC].tot_progress_ = false;
                        digiToAdd[iADC].tot_complete_ = true;
                    }
                    digiToAdd[iADC].toa_     = toa*ns_;
                    digiToAdd[iADC].adc_tm1_ = iADC > 0 ? digiToAdd.at(iADC-1).adc_t_ : pedestal_;
                    digiToAdd[iADC].rawID_   = simHit.getID();
                    tot -= clockCycle_; //decrement TOT
                }
    
                digitizedHits.addDigi( digiToAdd );
            } //where is the amplitude of the hit w.r.t. readout and TOT thresholds

        } //loop over simulated calorimeter hits

        /******************************************************************************************
         * Noise Simulation
         *****************************************************************************************/

        //put noise into some empty channels
        int numEmptyChannels = nTotalChannels_ - digitizedHits.getNumDigis(); //minus number of channels with a hit
        //noise generator gives us a list of noise amplitudes [mV] that randomly populate the empty
        //channels and are above the readout threshold
        auto noiseHitAmplitudes{noiseGenerator_->generateNoiseHits(numEmptyChannels)};
        for ( double noiseHit : noiseHitAmplitudes ) {

            //get a time for this noise hit
            int noiseID = generateNoiseID();
            while ( simHitIDs.find(noiseID) != simHitIDs.end() ) noiseID = generateNoiseID();
            simHitIDs.insert(noiseID);

            double hitTime = noiseInjector_->Uniform( clockCycle_ );
            int hitSample  = noiseInjector_->Integer( nADCs_ );

            std::vector<HgcrocDigiCollection::Sample> digiToAdd;
            digiToAdd.resize( nADCs_ );
            for ( int iADC = 0; iADC < digiToAdd.size(); iADC++ ) {
                if ( iADC == hitSample ) {
                    //put in noise hit
                    digiToAdd[iADC].adc_t_ = noiseHit/gain_;
                    digiToAdd[iADC].toa_   = hitTime*ns_;
                } else {
                    //noisy channels
                    digiToAdd[iADC].adc_t_ = pedestal_;
                    digiToAdd[iADC].toa_   = 0;
                }
                digiToAdd[iADC].adc_tm1_ = iADC > 0 ? digiToAdd.at(iADC-1).adc_t_ : pedestal_; 
                digiToAdd[iADC].rawID_   = noiseID;
                digiToAdd[iADC].tot_progress_ = false;
                digiToAdd[iADC].tot_complete_ = false;
            }

            digitizedHits.addDigi( digiToAdd );
        }

        return std::move(digitizedHits);
    } //HgcrocEmulator::digitize

    int HgcrocEmulator::generateNoiseID() const {
        if ( ecal_ ) {
            EcalDetectorID detID;
            int layerID = noiseInjector_->Integer(nEcalLayers_);
            int moduleID= noiseInjector_->Integer(nModulesPerLayer_);
            int cellID  = noiseInjector_->Integer(nCellsPerModule_);
            detID.setFieldValue( 1 , layerID );
            detID.setFieldValue( 2 , moduleID );
            detID.setFieldValue( 3 , cellID );
            return detID.pack();
        } else {
            //TODO hcal noise id generation
            return 0;
        }
    }

} // ldmx
