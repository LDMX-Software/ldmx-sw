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
#include <map>

namespace ldmx {

    /**
     * @class SimParticle
     * @brief Represents MC particle information from a track in the simulation
     */
    class SimParticle: public TObject {

        public:

            /**
             * Enum for interesting process types.
             */
            enum ProcessType {
                unknown = 0,
                annihil,
                compt,
                conv,
                electronNuclear,
                eBrem,
                eIoni,
                msc,
                phot,
                photonNuclear,
                GammaToMuPair
                /* Only add additional processes to the end of this list! */
            };

            typedef std::map<std::string, ProcessType> ProcessTypeMap;

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
            double getEnergy() {
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
            double getTime() {
                return time_;
            }

            /**
             * Get the XYZ vertex of the particle's creation [mm].
             * @return The vertex of the particle.
             */
            std::vector<double> getVertex() {
                return {x_, y_, z_};
            }

            /**
             * Get the endpoint of the particle where it was destroyed
             * or left the detector [mm].
             * @return The endpoint of the particle
             */
            std::vector<double> getEndPoint() {
                return {endX_, endY_, endZ_};
            }

            /**
             * Get the XYZ momentum of the particle [MeV].
             * @return The momentum of the particle.
             */
            std::vector<double> getMomentum() {
                return {px_, py_, pz_};
            }

            /**
             * Get the mass of the particle [GeV].
             * @return The mass of the particle.
             */
            double getMass() {
                return mass_;
            }

            /**
             * The charge of the particle (units of electron charge).
             * @return The charge of the particle.
             */
            double getCharge() {
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
            void setEnergy(const double energy) {
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
            void setTime(const double time) {
                this->time_ = time;
            }

            /**
             * Set the vertex of the particle [mm].
             * @param x The vertex X position.
             * @param y The vertex Y position.
             * @param z The vertex Z position.
             */
            void setVertex(const double x, const double y, const double z) {
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
            void setEndPoint(const double endX, const double endY, const double endZ) {
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
            void setMomentum(const double px, const double py, const double pz) {
                this->px_ = px;
                this->py_ = py;
                this->pz_ = pz;
            }

            /**
             * Set the mass of the particle [GeV].
             * @param mass The mass of the particle.
             */
            void setMass(const double mass) {
                this->mass_ = mass;
            }

            /**
             * Set the charge of the particle.
             * @param charge The charge of the particle.
             */
            void setCharge(const double charge) {
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

            /**
             * Get the creator process type of this particle.
             * This corresponds to the value returned by <i>G4VProcess::GetProcessSubType()</i>
             * e.g. 121 for products of photonuclear reactions.
             * @return The creator process type of this particle.
             */
            int getProcessType() {
                return processType_;
            }

            /**
             * Set the creator process type of this particle.
             * This is set from the value of <i>G4VProcess::GetProcessSubType()</i>.
             * @param processType The creator process type of this particle.
             */
            void setProcessType(int processType) {
                processType_ = processType;
            }

            /**
             * Set the momentum at the particle's end point.
             * @param endpx The X momentum.
             * @param endpy The Y momentum.
             * @param endpz The Z momentum.
             */ 
            void setEndPointMomentum(const double endpx, const double endpy, const double endpz) {
                endpx_ = endpx;
                endpy_ = endpy;
                endpz_ = endpz;
            }

            /**
             * Get the momentum at the particle's end point.
             * @return The momentum at the particle's end point as a vector.
             */
            std::vector<double> getEndPointMomentum() {
                return {endpx_, endpy_, endpz_};
            }

            /**
             * Get the process type enum from a G4VProcess name.
             * @return The process type from the string.
             */
            static ProcessType findProcessType(const char* processName) {
                if (PROCESS_MAP.find(processName) != PROCESS_MAP.end()) {
                    return PROCESS_MAP[processName];
                } else {
                    return ProcessType::unknown;
                }
            }

        private:

            static ProcessTypeMap createProcessTypeMap();

        private:

            /** The energy of the particle. */
            double energy_{0};

            /** The PDG code of the particle. */
            int pdgID_{0};

            /** The generator status. */
            int genStatus_{-1};

            /** The global creation time. */
            double time_{0};

            /** The X vertex. */
            double x_{0};

            /** The Y vertex. */
            double y_{0};

            /** The Z vertex. */
            double z_{0};

            /** The X end point. */
            double endX_{0};

            /** The Y end point. */
            double endY_{0};

            /** The Z end point. */
            double endZ_{0};

            /** The X momentum.*/
            double px_{0};

            /** The Y momentum. */
            double py_{0};

            /** The Z momentum. */
            double pz_{0};

            /** The X momentum.*/
            double endpx_{0};

            /** The Y momentum. */
            double endpy_{0};

            /** The Z momentum. */
            double endpz_{0};

            /** The particle's mass. */
            double mass_{0};

            /** The particle's charge. */
            double charge_{0};

            /** The list of daughter particles. */
            TRefArray* daughters_;

            /** The list of parent particles. */
            TRefArray* parents_;

            /** Encoding of Geant4 process type. */
            int processType_{-1};

            static ProcessTypeMap PROCESS_MAP;

            /**
             * ROOT class definition.
             */
            ClassDef(SimParticle, 4);
    };

}

#endif
