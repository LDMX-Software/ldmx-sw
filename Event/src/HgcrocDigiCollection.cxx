/**
 * @file HgcrocDigiCollection.cxx
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Event/HgcrocDigiCollection.h"

ClassImp(ldmx::HgcrocDigiCollection)

namespace ldmx {

    int32_t HgcrocDigiCollection::Sample::encode() const {

        //choose which measurements to put into first and second positions
        //  based off of the flags passed
        int firstMeas(0), seconMeas(0);
        if ( not tot_complete_ ) {
            firstMeas = adc_tm1_;
            seconMeas = adc_t_;
        } else if ( not tot_progress_ and tot_complete_ ) {
            firstMeas = adc_tm1_;
            //the 12 bit internal tot measurement needs to 
            //  be packed into a 10 bit int
            if ( tot_ < 512 ) seconMeas = tot_;
            else seconMeas = 512 + tot_/8; //lose some precision, but can go higher
        } else /* both flags true */ {
            firstMeas = adc_t_;
            seconMeas = tot_;
        }

        //check if over largest number possible ==> set to largest if over (don't want wrapping)
        //and then do bit shifting nonsense to code the measurements into the 32-bit word
        //set last measurement to TOA
        int32_t word = (tot_progress_ << FIRSTFLAG_POS) 
             + (tot_complete_ << SECONFLAG_POS) 
             + ( (( firstMeas > TEN_BIT_MASK ? TEN_BIT_MASK : firstMeas) & TEN_BIT_MASK) << FIRSTMEAS_POS ) 
             + ( (( seconMeas > TEN_BIT_MASK ? TEN_BIT_MASK : seconMeas) & TEN_BIT_MASK) << SECONMEAS_POS ) 
             + ( (( toa_      > TEN_BIT_MASK ? TEN_BIT_MASK : toa_     ) & TEN_BIT_MASK) );

        return std::move(word);
    }

    void HgcrocDigiCollection::Sample::decode(int32_t word) {

        //this is where the word --> measurements translation occurs

        bool firstFlag = ONE_BIT_MASK & ( word >> FIRSTFLAG_POS );
        bool seconFlag = ONE_BIT_MASK & ( word >> SECONFLAG_POS );
        int  firstMeas = TEN_BIT_MASK & ( word >> FIRSTMEAS_POS );
        int  seconMeas = TEN_BIT_MASK & ( word >> SECONMEAS_POS );
        int  lastMeas  = TEN_BIT_MASK & ( word );

        //the flags determine what the first and secon measurements should be interpreted as
        tot_progress_ = firstFlag;
        tot_complete_ = seconFlag;

        //the last measurement is always TOA (might be set to zero if hit was under TOA threshold)
        toa_ = lastMeas;

        if ( not tot_complete_ ) {
            //ADC Mode
            //  whether or not TOT is in progress, just output the ADC counts
            adc_tm1_ = firstMeas;
            adc_t_   = seconMeas;
        } else if ( not tot_progress_ and tot_complete_ ) {
            //TOT measurement completed, output it
            adc_tm1_ = firstMeas;
            //The tot measurement needs to be converted from a packed 10-bit integer
            //  to an unpacked 12-bit integer
            if ( seconMeas < 512 ) tot_ = seconMeas;
            else tot_ = 8*(seconMeas-512);
        } else /* both true */ {
            //Calibration Mode
            adc_t_ = firstMeas;
            //The tot measurement needs to be converted from a packed 10-bit integer
            //  to an unpacked 12-bit integer
            if ( seconMeas < 512 ) tot_ = seconMeas;
            else tot_ = 8*(seconMeas-512);
        }

        return;
    }

    void HgcrocDigiCollection::Clear() {

        channelIDs_.clear();
        samples_.clear();

        return;
    }

    void HgcrocDigiCollection::Print() const {

        std::cout << "HgcrocDigiCollection { Num Channel IDs: " << channelIDs_.size()
            << ", Num Samples: " << samples_.size()
            << ", Samples Per Digi: " << numSamplesPerDigi_
            << ", Index for SOI: " << sampleOfInterest_
            << "}" << std::endl;

        return;
    }

    HgcrocDigiCollection::HgcrocDigi HgcrocDigiCollection::getDigi( unsigned int digiIndex ) const {
        
        HgcrocDigiCollection::HgcrocDigi digi;
        digi.id_ = channelIDs_.at( digiIndex );

        for ( unsigned int sampleIndex = 0; sampleIndex < this->getNumSamplesPerDigi(); sampleIndex++ )
            digi.samples_.emplace_back( samples_.at( digiIndex*numSamplesPerDigi_ + sampleIndex ) );

        return digi;
    }

    void HgcrocDigiCollection::addDigi( const HgcrocDigiCollection::HgcrocDigi &digi ) {

        if ( digi.samples_.size() != this->getNumSamplesPerDigi() ) {
            std::cerr << "[ WARN ] [ HgcrocDigiCollection ] Input list of samples has size '"
                << digi.samples_.size() << "' that does not match the number of samples per digi '"
                << this->getNumSamplesPerDigi() << "'!." << std::endl;
            return;
        }
        
        channelIDs_.push_back( digi.id_ );

        for ( auto const &s : digi.samples_ )
            samples_.push_back( s.encode() );

        return;
    }

} //ldmx
