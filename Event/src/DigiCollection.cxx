/**
 * @file DigiCollection.cxx
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Event/DigiCollection.h"

ClassImp(ldmx::DigiCollection)

namespace ldmx {

    void DigiCollection::Clear() {

        channelIDs_.clear();
        samples_.clear();

        return;
    }

    void DigiCollection::Print() const {

        printf( "Num Channel IDs:      %d\n" , channelIDs_.size() );
        printf( "Num Samples:          %d\n" , samples_.size() );
        printf( "Num Samples Per Digi: %d\n" , numSamplesPerDigi_ );

        return;
    }

    void DigiCollection::addDigi( int channelID , std::vector< int32_t > newSamples ) {

        channelIDs_.push_back( channelID );
        samples_.insert( samples_.end() , newSamples.begin() , newSamples.end() );

        return;
    }

    int32_t DigiCollection::getSampleWord( unsigned int digiIndex, unsigned int sampleIndex ) const {

        return samples_.at( digiIndex*numSamplesPerDigi_ + sampleIndex );

    }

}
