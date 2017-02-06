/**
 * @file SimCalorimeterHit.h
 * @brief Class which stores simulated calorimeter hit information
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_SIMCALORIMETERHIT_H_
#define EVENT_SIMCALORIMETERHIT_H_

// ROOT
#include "TObject.h"
#include "TRefArray.h"

// LDMX
#include "Event/SimParticle.h"

namespace ldmx {

    /**
     * @class SimCalorimeterHit
     * @brief Stores simulated calorimeter hit information
     *
     * @note
     * This class represents simulated hit information from a calorimeter detector.
     * It provides access to the cell ID, energy deposition, cell position and time.
     * Additionally, individual depositions or steps from MC particles are tabulated
     * as contributions stored in vectors.  Contribution information includes a reference
     * to the relevant SimParticle, the PDG code of the actual particle which deposited
     * energy (may be different from the actual SimParticle), the time of the contribution
     * and the energy deposition.
     */
    class SimCalorimeterHit: public TObject {

        public:

            /**
             * @class Contrib
             * @brief Information about a contribution to the hit in the associated cell
             */
            struct Contrib {
                SimParticle* particle{nullptr};
                int pdgCode{0};
                float edep{0};
                float time{0};
            };

            /**
             * Class constructor.
             */
            SimCalorimeterHit();

            /**
             * Class destructor.
             */
            virtual ~SimCalorimeterHit();

            /**
             * Clear the data in the object.
             */
            void Clear(Option_t *option = "");

            /**
             * Print out the object.
             */
            void Print(Option_t *option = "") const;

            /**
             * Get the detector ID.
             * @return The detector ID.
             */
            int getID() {
                return id_;
            }

            /**
             * Set the detector ID.
             * @id The detector ID.
             */
            void setID(const int id) {
                this->id_ = id;
            }

            /**
             * Get the energy deposition of the hit [MeV].
             * @return The energy deposition of the hit.
             */
            float getEdep() {
                return edep_;
            }

            /**
             * Set the energy deposition of the hit [MeV].
             * @param edep The energy deposition of the hit.
             */
            void setEdep(const float edep) {
                this->edep_ = edep;
            }

            /**
             * Get the XYZ position of the hit [mm].
             * @return The XYZ position of the hit.
             */
            std::vector<float> getPosition() const {
                return {x_, y_, z_};
            }

            /**
             * Set the XYZ position of the hit [mm].
             * @param x The X position.
             * @param y The Y position.
             * @param z The Z position.
             */
            void setPosition(const float x, const float y, const float z) {
                this->x_ = x;
                this->y_ = y;
                this->z_ = z;
            }

            /**
             * Get the global time of the hit [ns].
             * @return The global time of the hit.
             */
            float getTime() {
                return time_;
            }

            /**
             * Set the time of the hit [ns].
             * @param time The time of the hit.
             */
            void setTime(const float time) {
                this->time_ = time;
            }

            /**
             * Get the number of hit contributions.
             * @return The number of hit contributions.
             */
            unsigned getNumberOfContribs() {
                return nContribs_;
            }

            /**
             * Add a hit contribution from a SimParticle.
             * @param simParticle The particle that made the contribution.
             * @param pdgCode The PDG code of the actual track.
             * @param edep The energy deposition of the hit [MeV].
             * @param time The time of the hit [ns].
             */
            void addContrib(SimParticle* simParticle, int pdgCode, float edep, float time);

            /**
             * Get a hit contribution by index.
             * @param i The index of the hit contribution.
             * @return The hit contribution at the index.
             */
            Contrib getContrib(int i);

            /**
             * Find the index of a hit contribution from a SimParticle and PDG code.
             * @param simParticle The sim particle that made the contribution.
             * @param pdgCode The PDG code of the contribution.
             * @return The index of the contribution or -1 if none exists.
             */
            int findContribIndex(SimParticle* simParticle, int pdgCode);

            /**
             * Update an existing hit contribution by incrementing its edep and setting the time
             * if the new time is less than the old one.
             * @param i The index of the contribution.
             * @param edep The additional energy contribution [MeV].
             * @param time The time of the contribution [ns].
             */
            void updateContrib(int i, float edep, float time);

        private:

            /**
             * The detector ID.
             */
            int id_{0};

            /**
             * The energy deposition.
             */
            float edep_{0};

            /**
             * The X position.
             */
            float x_{0};

            /**
             * The Y position.
             */
            float y_{0};

            /**
             * The Z position.
             */
            float z_{0};

            /**
             * The global time of the hit.
             */
            float time_{0};

            /**
             * The list of SimParticle objects contributing to the hit.
             */
            TRefArray* simParticleContribs_;

            /**
             * The list of PDG codes contributing to the hit.
             */
            std::vector<int> pdgCodeContribs_;

            /**
             * The list of energy depositions contributing to the hit.
             */
            std::vector<float> edepContribs_;

            /**
             * The list of times contributing to the hit.
             */
            std::vector<float> timeContribs_;

            /**
             * The number of hit contributions.
             */
            unsigned nContribs_{0};

            /**
             * ROOT class definition.
             */
            ClassDef(SimCalorimeterHit, 2)
    };

}

#endif
