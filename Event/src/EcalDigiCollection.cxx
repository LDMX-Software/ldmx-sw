/**
 * @file EcalDigiCollection.cxx
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Event/EcalDigiCollection.h"

ClassImp(ldmx::EcalDigiCollection)

namespace ldmx {

    void EcalDigiCollection::Clear() {

        channelIDs_.clear();
        samples_.clear();

        return;
    }

    void EcalDigiCollection::Print() const {

        std::cout << "EcalDigiCollection { Num Channel IDs: " << channelIDs_.size()
            << ", Num Samples: " << samples_.size()
            << ", Samples Per Digi: " << numSamplesPerDigi_
            << ", Index for SOI: " << sampleOfInterest_
            << "}" << std::endl;

        return;
    }

    std::vector< EcalDigiSample > EcalDigiCollection::getDigi( unsigned int digiIndex ) const {
        
        std::vector< EcalDigiSample > digi;
        for ( unsigned int sampleIndex = 0; sampleIndex < this->getNumSamplesPerDigi(); sampleIndex++ ) {
    
            EcalDigiSample sample;
    
            sample.rawID_ = channelIDs_.at( digiIndex );
    
            int32_t word = samples_.at( digiIndex*numSamplesPerDigi_ + sampleIndex );
    
            //this is where the word --> measurements translation occurs
    
            bool firstFlag = ONE_BIT_MASK & ( word >> FIRSTFLAG_POS );
            bool seconFlag = ONE_BIT_MASK & ( word >> SECONFLAG_POS );
            int  firstMeas = TEN_BIT_MASK & ( word >> FIRSTMEAS_POS );
            int  seconMeas = TEN_BIT_MASK & ( word >> SECONMEAS_POS );
            int  lastMeas  = TEN_BIT_MASK & ( word );
    
            //the flags determine what the first and secon measurements should be interpreted as
            sample.tot_progress_ = firstFlag;
            sample.tot_complete_ = seconFlag;

            //the last measurement is always TOA (might be set to zero if hit was under TOA threshold)
            sample.toa_ = lastMeas;

            if ( not sample.tot_complete_ ) {
                //ADC Mode
                //  whether or not TOT is in progress, just output the ADC counts
                sample.adc_tm1_ = firstMeas;
                sample.adc_t_   = seconMeas;
            } else if ( not sample.tot_progress_ and sample.tot_complete_ ) {
                //TOT measurement completed, output it
                sample.adc_tm1_ = firstMeas;
                sample.tot_     = seconMeas;
            } else /* both true */ {
                //Calibration Mode
                sample.adc_ = firstMeas;
                sample.tot_ = seconMeas;
            }

            digi.push_back( sample );
        }

        return digi;
    }

    void EcalDigiCollection::addDigi( std::vector< EcalDigiSample > newSamples ) {

        if ( newSamples.size() != this->getNumSamplesPerDigi() ) {
            std::cerr << "[ WARN ] [ EcalDigiCollection ] Input list of samples has size '"
                << newSamples.size() << "' that does not match the number of samples per digi '"
                << this->getNumSamplesPerDigi() << "'!." << std::endl;
            return;
        }
        
        int channelID = newSamples.at(0).rawID_;
        channelIDs_.push_back( channelID );

        for ( auto const &sample : newSamples ) {
            
            int32_t word;

            //this is where the measurements --> word translation occurs

            //choose which measurements to put into first and second positions
            //  based off of the flags passed
            int firstMeas(0), seconMeas(0);
            if ( not sample.tot_complete_ ) {
                firstMeas = (sample.adc_tm1_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.adc_tm1_;
                seconMeas = (sample.adc_t_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.adc_t_;
            } else if ( not sample.tot_progress_ and sample.tot_complete_ ) {
                firstMeas = (sample.adc_tm1_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.adc_tm1_;
                seconMeas = (sample.tot_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.tot_;
            } else /* both flags true */ {
                firstMeas = (sample.adc_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.adc_;
                seconMeas = (sample.tot_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.tot_;
            }

            //check if over largest number possible ==> set to largest if over (don't want wrapping)
            //and then do bit shifting nonsense to code the measurements into the 32-bit word
            //set last measurement to TOA
            word = (sample.tot_progress_ << FIRSTFLAG_POS) 
                 + (sample.tot_complete_ << SECONFLAG_POS) 
                 + ( (( firstMeas > TEN_BIT_MASK ? TEN_BIT_MASK : firstMeas) & TEN_BIT_MASK) << FIRSTMEAS_POS ) 
                 + ( (( seconMeas > TEN_BIT_MASK ? TEN_BIT_MASK : seconMeas) & TEN_BIT_MASK) << SECONMEAS_POS ) 
                 + ( (( sample.toa_ > TEN_BIT_MASK ? TEN_BIT_MASK : sample.toa_ ) & TEN_BIT_MASK) )
            
            samples_.push_back( word );
        }


        return;
    }

} //ldmx
