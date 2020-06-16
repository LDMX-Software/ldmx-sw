#ifndef DQM_PHOTONUCLEARDQM_H
#define DQM_PHOTONUCLEARDQM_H

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

namespace ldmx { 

    // Forward declarations within the ldmx workspace
    class Event;
    class SimParticle;

    class PhotoNuclearDQM : public Analyzer { 
    
        public: 

            /// Constructor
            PhotoNuclearDQM(const std::string &name, Process &process);

            /// Destructor
            ~PhotoNuclearDQM();

            /** 
             * Configure this analyzer using the user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this 
             *                   analyzer.
             */
            void configure(Parameters& parameters) final override; 
 
            /**
             * Process the event and create the histogram summaries.
             *
             * @param event The event to analyze.
             */
            void analyze(const Event& event) final override;

            /// Method executed before processing of events begins. 
            void onProcessStart();

        private:

            /**
             * Print the particle tree.
             * 
             * @param[in] particleMap The map containing the SimParticles.
             */
            void printParticleTree(std::map< int, SimParticle > particleMap);

            /**
             * Print the daughters of a particle.
             *
             * @param[in] particleMap The map containing the SimParticles.
             * @param[in] particle The particle whose daughters will be printed.
             * @param[in] depth The tree depth.
             *
             * @return[out] A vector with the track IDs of particles that have 
             *      already been printed.
             */
            std::vector< int > printDaughters(std::map< int, SimParticle > particleMap, const SimParticle particle, int depth);  

            /** Method used to classify events. */
            int classifyEvent(const std::vector< const SimParticle* > daughters, double threshold); 

            /** Method used to classify events in a compact manner. */
            int classifyCompactEvent(const SimParticle* pnGamma, 
                                     const std::vector< const SimParticle* > daughters, double threshold); 

    };    
    
} // ldmx

#endif // _DQM_ECAL_PN_H_
