/**
 * @file DigiCollection.h
 * @brief Class representing the collection of digitized signals from any subsystem
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENT_DIGICOLLECTION_H_
#define EVENT_DIGICOLLECTION_H_

// ROOT
#include "TObject.h" //for ClassDef

// STL
#include <stdint.h> //32bit words
#include <vector> //vector lists
#include <iostream> //Print method

namespace ldmx {

    /**
     * @class DigiCollection
     * @brief Collection of digitized signals from any subsystem
     *
     * @note This class represents the digitized signal information
     * in the form of a series of samples for each channel of readout.
     * Each channel is represented by an ID integer and each sample is a 32-bit word.
     * The number of samples for each digi is configurable, but is required to be
     * the same for all channels.
     *
     * Each digi corresponds to a one channel ID and numSamplesPerDigi_ samples.
     */
    class DigiCollection {

        public:

            /**
             * Class constructor.
             */
            DigiCollection() { }

            /**
             * Class destructor.
             *
             * Does nothing. STL vectors clean up themselves well.
             */
            virtual ~DigiCollection() { }

            /**
             * Clear the data in the object.
             *
             * Clears the vectors of channel IDs and samples, but does not change the number of samples per digi setting.
             */
            void Clear();

            /**
             * Print out the object.
             *
             * Prints out the lengths of the stored vectors and the number of samples per digi setting.
             */
            void Print() const;

            /**
             * Get number of samples per digi 
             */
            int getNumSamplesPerDigi() const { return numSamplesPerDigi_; }

            /**
             * Set number of samples for each digi
             */
            void setNumSamplesPerDigi( int n ) { numSamplesPerDigi_ = n; return; }

            /**
             * Add a signal measurement into digi collection.
             *
             * Vector must contain the correct number of samples.
             *  i.e. numSamplesPerDigi_
             * No checking is done!
             * Can be overwritten for a specific subsystem.
             *  e.g. The chip encoding can be done in a overwritten function of a derived class.
             */
            void addDigi( int channelID , std::vector< int32_t > newSamples );

            /**
             * Get Sample at the input indices
             *
             * No checking is done. 
             * digiIndex runs from 0 to getNumDigis()-1
             * sampleIndex runs from 0 to getNumSamplesPerDigi()-1
             */
            int32_t getSampleWord( unsigned int digiIndex , unsigned int sampleIndex ) const;

            /**
             * Get Channel ID at the input digiIndex
             *
             * digiIndex runs from 0 to getNumDigis()-1
             */
            int getChannelID( unsigned int digiIndex ) const { return channelIDs_.at( digiIndex ); }

            /**
             * Get Total number of Digis in this collection
             */
            unsigned int getNumDigis() const { return channelIDs_.size(); }

        private:

            /** list of channel IDs that we have digis for */
            std::vector< int > channelIDs_;

            /** list of samples that we have been given */
            std::vector< int32_t > samples_;

            /** number of samples for each digi */
            int numSamplesPerDigi_{1};

            /**
             * The ROOT class definition.
             */
            ClassDef(DigiCollection, 1);
    };

}

#endif /* EVENT_DIGICOLLECTION_H_ */
