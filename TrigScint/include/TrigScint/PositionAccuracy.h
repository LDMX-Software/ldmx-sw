/**
 * @file PositionAccuracy.h
 * @brief Class that determines position difference between TrigScint tracks/clusters and found beamElectrons.
 * @author Erik Lundblad, Lund University
 */

#ifndef TRIGSCINT_POSITIONACCURACY_H
#define TRIGSCINT_POSITIONACCURACY_H

//---< Framework >---//
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

//---< TrigScint >---///
#include "TrigScint/Event/PositionDifference.h"
#include "TrigScint/Event/TrigScintCluster.h"

namespace trigscint {
/**
* Position accuracy of TS clusters and tracks processor
*
* This processor calculates the difference in position
* between found TS clusters and the beam electrons
* found in the BeamElectronLocator, as well as the position
* difference between TS tracks and the beam electrons.
*
* It takes the clusters, tracks and located beam electrons
* from any TS pad as input_collection
*
* It outputs the position difference as output_collections
*/

class PositionAccuracy : public framework::Producer {
    public:
        /**
        * Constructor.
        *
        * @param name Name for this instance of the class.
        * @param process The Process class associated with EventProcessor,
        * provided by the framework.
        */
        PositionAccuracy(const std::string &name, framework::Process &process)
            : Producer(name, process) {}

        /**
        * Configure the processor using the given user specified parameters.
        *
        * The user specified parameters that are available are defined
        * in the python configuration class. Look at the positionAccuracy.py
        * in Recon/python for the python structure.
        *
        * @param parameters Set of parameters used to configure this processor.
        */
        void configure(framework::config::Parameters &parameters) override;

        /**
        * Process the event and put new data products into it.
        *
        * @param event The event to process.
        */
        void produce(framework::Event &event) override;

    private:
        /**
         * Help function to pair horizontal and vertical clusters together
         * to new clusters with x,y and sx, sy.
         * @return matchedClusters. All possible cluster x,y and sx, sy combinations.
         */
        std::vector<ldmx::TrigScintCluster> matchXYClusters(std::vector<ldmx::TrigScintCluster> clusters);

        /*
         * Input collections:
         *  Element [0]: clusters from pad
         *  Element [1]: tracks from pad
         *  Element [2]: beam electrons from same pad
         */
        std::vector<std::string> input_collections_;

        /*
         * The pass name of the input collection
         */
        std::string inputPassName_;

        /*
         * Output collections of position differences
         *  Element [0]: clusters and beam electrons difference
         *  Element [1]: tracks and beam electrons difference
         */
        std::vector<std::string> output_collections_;

        /**
         * barID of first vertical bar in geometry
         */
        int vertBarStartIdx_{52};

        /**
         * Pad number
         */
        int padNumber_;

        /**
         * Pad position
         */
        std::vector<double> padPosition_;

        /*
         * Verbosity: whether to print detailed debug messages or not
         */
        bool verbose_{false};


};  // PositionAccuracy
}   // namespace trigscint

#endif  // TRIGSCINT_POSITIONACCURACY_H