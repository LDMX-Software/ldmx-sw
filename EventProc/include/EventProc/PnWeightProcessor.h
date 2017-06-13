/**
 * @file pnWeightProcessor.h
 * @brief Processor that calculates pnWeight based on photonNuclear track 
 *        properties.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_PNWEIGHTPROCESSOR_H_
#define EVENTPROC_PNWEIGHTPROCESSOR_H_

// LDMX
#include "Event/PnWeightResult.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    /**
     * @class pnWeightProcessor
     * @brief Calculates pnWeight based on photonNuclear track properties.
     *
     * @note
     * pnWeightProcessor calculates an event weight which is added to the 
     * collection as a pnWeight object. This weight is based on simParticles
     * arising from photonNuclear reactions, and is intended to correct
     * the simulation in the case of high-momentum, backwards-going nucleons
     * arising from those reactions.
     *   fit variable W_p = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p))
     *     where p_tot = sqrt(K^2 + 2*K*m)
     *           K = kinetic energy of nucleon at PN vertex
     *           p, p_z = momentum, z-component of nucleon at PN vertex
     * 
     */

    class PnWeightProcessor : public Producer {
        public:
            PnWeightProcessor(const std::string& name, Process& process) :
                Producer(name, process) {
            }

            virtual ~PnWeightProcessor() {}

            /**
            *  Read in user-specified parameters
            */
            virtual void configure(const ParameterSet& pSet);

            /**
            *  Run the weight calculation and create a pnWeightResult
            */
            virtual void produce(Event& event);

            /**
             * Calculate the fitted W_p defined as
             *     W_p(fit) = c1*exp(c2*(W_p(measured) - c3))
             * where
             *     c1 = 1.78032e+04
             *     c2 = -8.07561e-03
             *     c3 = 7.91244e+02
             * 
             * @param wp Measured W_p.
             * @return W_p(fit)
             */
            double calculateFitWp(double wp); 

            /**
             * Calculate the measured W_p defined as
             *     W_p(measured) = 0.5*(p_tot + K)*(1.12-0.5*(p_z/p)) 
             * where 
             *     p is the total momentum of the particle
             *     K is its kinetic energy
             *     p_z is the z component of the momentum
             * all defined at the vertex.
             *
             * @param particle SimParticle used to calculate W_p.
             * @return W_p 
             */
            double calculateWp(SimParticle* particle);

        private:
        
            bool verbose_{false}; 

            double wpThreshold_;

            PnWeightResult result_;
    };
}

#endif
