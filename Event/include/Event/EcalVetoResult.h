/**
 * @file EcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from 
 *        EcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_ECALVETORESULT_H_
#define EVENT_ECALVETORESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>

//----------//
//   LDMX   //
//----------//
#include "Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

namespace ldmx {

    class EcalVetoResult : public TObject {

        public:

            /** Constructor */
            EcalVetoResult();

            /** Destructor */
            ~EcalVetoResult();

            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setVariables(int nReadoutHits, int nLooseIsoHits, int nTightIsoHits, float summedDet, int summedOuter,
                    float backSummedDet, float summedLooseIso, float maxLooseIsoDep, float summedTightIso, float maxTightIsoDep,
                    float maxCellDep, float showerRMS, std::vector<float> EcalLayerEdepReadout, std::vector<std::pair<int, float>> looseMipTracks,
                    std::vector<std::pair<int, float>> mediumMipTracks, std::vector<std::pair<int, float>> tightMipTracks);

            /** Reset the object. */
            void Clear(Option_t *option = "");

            /**
             * Copy this object. 
             *
             * @param object The target object. 
             */
            void Copy(TObject& object) const;

            /** Print the object */
            void Print(Option_t *option = "") const;

            /** Checks if the event passes the Ecal veto. */
            bool passesVeto() {
                return passesVeto_;
            }
            float getDisc() {
                return discValue_;
            }
            int getNReadoutHits() {
                return nReadoutHits_;
            }

            int getNLooseIsoHits() {
                return nLooseIsoHits_;
            }

            int getNTightIsoHits() {
                return nTightIsoHits_;
            }

            int nLooseMipTracks() {
                return nLooseMipTracks_;
            }

            int nMediumMipTracks() {
                return nMediumMipTracks_;
            }

            int nTightMipTracks() {
                return nTightMipTracks_;
            }

            float getSummedDet() {
                return summedDet_;
            }

            float getSummedOuter() {
                return summedOuter_;
            }

            float getBackSummedDep() {
                return backSummedDet_;
            }

            float getSummedLooseIso() {
                return summedLooseIso_;
            }

            float getMaxLooseIsoDep() {
                return maxLooseIsoDep_;
            }

            float getSummedTightIso() {
                return summedTightIso_;
            }

            float getMaxTightIsoDep() {
                return maxTightIsoDep_;
            }
            float getMaxCellDep() {
                return maxCellDep_;
            }
            float getShowerRMS() {
                return showerRMS_;
            }
            std::vector<float> getEcalLayerEdepReadout() {
                return ecalLayerEdepReadout_;
            }

            std::vector<std::pair<int, float>> getLooseMipTracks() {
                return looseMipTracks_;
            }

            std::vector<std::pair<int, float>> getMediumMipTracks() {
                return mediumMipTracks_;
            }

            std::vector<std::pair<int, float>> getTightMipTracks() {
                return tightMipTracks_;
            }

            void setVetoResult(bool passesVeto) {
                passesVeto_ = passesVeto;
            }
            void setDiscValue(float discValue) {
                discValue_ = discValue;
            }
        private:

            /** Flag indicating whether the event is vetoed by the Ecal. */
            bool passesVeto_{false};

            int nReadoutHits_{0};
            int nLooseIsoHits_{0};
            int nTightIsoHits_{0};
            int nLooseMipTracks_{0};
            int nMediumMipTracks_{0};
            int nTightMipTracks_{0};

            float summedDet_{0};
            float summedOuter_{0};
            float backSummedDet_{0};
            float summedLooseIso_{0};
            float maxLooseIsoDep_{0};
            float summedTightIso_{0};
            float maxTightIsoDep_{0};
            float maxCellDep_{0};
            float showerRMS_{0};
            float discValue_{0};
            std::vector<float> ecalLayerEdepReadout_;
            std::vector<std::pair<int, float>> looseMipTracks_;
            std::vector<std::pair<int, float>> mediumMipTracks_;
            std::vector<std::pair<int, float>> tightMipTracks_;

        ClassDef(EcalVetoResult, 2);

    };
}

#endif
