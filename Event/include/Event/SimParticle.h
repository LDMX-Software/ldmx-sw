/**
 * @file SimParticle.h
 * @brief Class which implements an MC particle that stores information about tracks from the simulation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_SIMPARTICLE_H_
#define EVENT_SIMPARTICLE_H_

// ROOT
#include "TObject.h"
#include "TRefArray.h"

// STL
#include <vector>

namespace ldmx {

/**
 * @class SimParticle
 * @brief Represents MC particle information from a track in the simulation
 */
class SimParticle: public TObject {

    public:

        /**
         * Class constructor.
         */
        SimParticle();

        /**
         * Class destructor.
         */
        virtual ~SimParticle();

        /**
         * Clear the data in this object.
         */
        void Clear(Option_t *option = "");

        /**
         * Print out information of this object.
         */
        void Print(Option_t *option = "") const;

        /**
         * Get the energy of the particle [MeV].
         * @return The energy of the particle.
         */
        float getEnergy() {
            return energy_;
        }

        /**
         * Get the PDG code of the particle.
         * @return The PDG code of the particle.
         */
        int getPdgID() {
            return pdgID_;
        }

        /**
         * Get the generator status of the particle.
         * A non-zero status indicates that the particle originates from
         * an event generator source like an input LHE file.
         * @return The generator status.
         */
        int getGenStatus() {
            return genStatus_;
        }

        /**
         * Get the global time of the particle's creation [ns].
         * @return The global time of the particle's creation.
         */
        float getTime() {
            return time_;
        }

        /**
         * Get the XYZ vertex of the particle's creation [mm].
         * @return The vertex of the particle.
         */
        std::vector<float> getVertex() {
            return {x_, y_, z_};
        }

        /**
         * Get the endpoint of the particle where it was destroyed
         * or left the detector [mm].
         * @return The endpoint of the particle
         */
        std::vector<float> getEndPoint() {
            return {endX_, endY_, endZ_};
        }

        /**
         * Get the XYZ momentum of the particle [MeV].
         * @return The momentum of the particle.
         */
        std::vector<float> getMomentum() {
            return {px_, py_, pz_};
        }

        /**
         * Get the mass of the particle [GeV].
         * @return The mass of the particle.
         */
        float getMass() {
            return mass_;
        }

        /**
         * The charge of the particle (units of electron charge).
         * @return The charge of the particle.
         */
        float getCharge() {
            return charge_;
        }

        /**
         * Get the number of daughter particles.
         */
        int getDaughterCount() {
            return daughters_->GetEntriesFast();
        }

        /**
         * Get a daughter particle by index.
         * @param iDau The index of the daughter particle.
         */
        SimParticle* getDaughter(int iDau) {
            return (SimParticle*) daughters_->At(iDau);
        }

        /**
         * Get the number of parent particles.
         * @return The number of parent particles.
         */
        int getParentCount() {
            return parents_->GetEntriesFast();
        }

        /**
         * Get a parent particle by index.
         * @param iPar The index of the parent particle.
         */
        SimParticle* getParent(int iPar) {
            return (SimParticle*) parents_->At(iPar);
        }

        /**
         * Set the energy of the particle [MeV].
         * @param energy The energy of the particle.
         */
        void setEnergy(const float energy) {
            this->energy_ = energy;
        }

        /**
         * Set the PDG code of the hit.
         * @param pdgID The PDG code of the hit.
         */
        void setPdgID(const int pdgID) {
            this->pdgID_ = pdgID;
        }

        /**
         * Set the generator status of the hit.
         * @param genStatus The generator status of the hit.
         */
        void setGenStatus(const int genStatus) {
            this->genStatus_ = genStatus;
        }

        /**
         * Set the global time of the particle's creation [ns].
         * @param time The global time of the particle's creation.
         */
        void setTime(const float time) {
            this->time_ = time;
        }

        /**
         * Set the vertex of the particle [mm].
         * @param x The vertex X position.
         * @param y The vertex Y position.
         * @param z The vertex Z position.
         */
        void setVertex(const float x, const float y, const float z) {
            this->x_ = x;
            this->y_ = y;
            this->z_ = z;
        }

        /**
         * Set the end point of the particle [mm].
         * @param endX The X end point.
         * @param endY The Y end point.
         * @param endZ The Z end point.
         */
        void setEndPoint(const float endX, const float endY, const float endZ) {
            this->endX_ = endX;
            this->endY_ = endY;
            this->endZ_ = endZ;
        }

        /**
         * Set the momentum of the particle [MeV].
         * @param px The X momentum.
         * @param py The Y momentum.
         * @param pz The Z momentum.
         */
        void setMomentum(const float px, const float py, const float pz) {
            this->px_ = px;
            this->py_ = py;
            this->pz_ = pz;
        }

        /**
         * Set the mass of the particle [GeV].
         * @param mass The mass of the particle.
         */
        void setMass(const float mass) {
            this->mass_ = mass;
        }

        /**
         * Set the charge of the particle.
         * @param charge The charge of the particle.
         */
        void setCharge(const float charge) {
            this->charge_ = charge;
        }

        /**
         * Add a daughter particle.
         * @param daughter The daughter particle.
         */
        void addDaughter(SimParticle* daughter) {
            daughters_->Add(daughter);
        }

        /**
         * Add a parent particle.
         * @param parent The parent particle.
         */
        void addParent(SimParticle* parent) {
            parents_->Add(parent);
        }

    private:

        /**
         * The energy of the particle.
         */
        double energy_{0};

        /**
         * The PDG code of the particle.
         */
        int pdgID_{0};

        /**
         * The generator status.
         */
        int genStatus_{-1};

        /**
         * The global creation time.
         */
        float time_{0};

        /**
         * The X vertex.
         */
        float x_{0};

        /**
         * The Y vertex.
         */
        float y_{0};

        /**
         * The Z vertex.
         */
        float z_{0};

        /**
         * The X end point.
         */
        float endX_{0};

        /**
         * The Y end point.
         */
        float endY_{0};

        /**
         * The Z end point.
         */
        float endZ_{0};

        /**
         * The X momentum.
         */
        float px_{0};

        /**
         * The Y momentum.
         */
        float py_{0};

        /**
         * The Z momentum.
         */
        float pz_{0};

        /**
         * The particle's mass.
         */
        float mass_{0};

        /**
         * The particle's charge.
         */
        float charge_{0};

        /**
         * The list of daughter particles.
         */
        TRefArray* daughters_;

        /**
         * The list of parent particles.
         */
        TRefArray* parents_;

    /**
     * ROOT class definition.
     */
    ClassDef(SimParticle, 2);
};

}

#endif
