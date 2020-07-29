
#include "Tools/HgcrocEmulator.h"

#include "DetDescr/EcalDetectorID.h"

#include <time.h>

namespace ldmx { 

    HgcrocEmulator::HgcrocEmulator(const Parameters& ps) {

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
        nADCs_            = ps.getParameter<int>("nADCs");
        iSOI_             = ps.getParameter<int>("iSOI");
        rateUpSlope_      = ps.getParameter<double>("rateUpSlope");
        timeUpSlope_      = ps.getParameter<double>("timeUpSlope");
        rateDnSlope_      = ps.getParameter<double>("rateDnSlope");
        timeDnSlope_      = ps.getParameter<double>("timeDnSlope");
        timePeak_         = ps.getParameter<double>("timePeak");

        //Time -> clock counts conversion
        //  time [ns] * ( 2^10 / max time in ns ) = clock counts
        ns_ = 1024./clockCycle_;

        // Configure the pulse shape function
        pulseFunc_ = TF1(
                "pulseFunc",
                "[0]*((1.0+exp([1]*(-[2]+[3])))*(1.0+exp([5]*(-[6]+[3]))))/((1.0+exp([1]*(x-[2]+[3]-[4])))*(1.0+exp([5]*(x-[6]+[3]-[4]))))",
                0.0,(double) nADCs_*clockCycle_
                );
        pulseFunc_.FixParameter( 1 , rateUpSlope_ );
        pulseFunc_.FixParameter( 2 , timeUpSlope_ );
        pulseFunc_.FixParameter( 3 , timePeak_    );
        pulseFunc_.FixParameter( 5 , rateDnSlope_ );
        pulseFunc_.FixParameter( 6 , timeDnSlope_ );

    }

    bool HgcrocEmulator::digitize(
            const std::vector<double> &voltages,
            const std::vector<double> &times,
            std::vector<HgcrocDigiCollection::Sample> &digiToAdd
    ) const {

        digiToAdd.clear(); //make sure it is clean
        digiToAdd.resize( nADCs_ ); //fill with required number of samples (default constructed)

        //sum all voltages and do a voltage-weighted average to get the hit time
        //  exclude any hits with times outside the sampling region
        double signalAmplitude = 0.0;
        double timeInWindow   = 0.0;
        for ( int iContrib = 0; iContrib < voltages.size(); iContrib++ ) {

            if ( times.at(iContrib)  < 0 or times.at(iContrib) > clockCycle_*nADCs_ ) {
                //invalid contribution - outside time range or time is unset
                continue;
            }

            signalAmplitude += voltages.at(iContrib);
            timeInWindow    += voltages.at(iContrib) * times.at(iContrib);
        }
        if ( signalAmplitude > 0. ) timeInWindow /= signalAmplitude; //voltage weighted average

        // put noise onto timing
        //TODO more physical way of simulating the timing jitter
        timeInWindow += noiseInjector_->Gaus( 0. , timingJitter_ );

        //set time in the window to zero if noise pushed it below zero
        //TODO better (more physical) method for handling this case?
        if ( timeInWindow   < 0. ) timeInWindow = 0.;

        //setup up pulse by changing the amplitude and timing parameters
        configurePulse( signalAmplitude , timeInWindow );

        // choose readout mode
        double pulsePeak = measurePulse( timeInWindow , false );
        if (verbose_) {
            std::cout << "Pulse: { "
                << "Amplitude: " << signalAmplitude+gain_*pedestal_ << "mV, "
                << "Beginning: " << measurePulse(0.,false) << "mV, "
                << "Time: " << timeInWindow << "ns } -> ";
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
                digiToAdd[iADC].tot_progress_ = false;
                digiToAdd[iADC].tot_complete_ = false;
                if (verbose_) std::cout << " ADC " << iADC << ": " << digiToAdd[iADC].adc_t_*gain_ << "mV, ";
            }
            if (verbose_) std::cout << "}" << std::endl;

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
                tot -= clockCycle_; //decrement TOT
            }

        } //where is the amplitude of the hit w.r.t. readout and TOT thresholds

        return true;
    } //HgcrocEmulator::digitize

} // ldmx
