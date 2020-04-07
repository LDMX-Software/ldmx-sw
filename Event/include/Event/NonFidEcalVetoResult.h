/**
 * @file NonFidEcalVetoResult.h
 * @brief Class used to encapsulate the results obtained from
 *        NonFidEcalVetoProcessor.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_NONFIDECALVETORESULT_H_
#define EVENT_NONFIDECALVETORESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <ostream>
#include <map>

//----------//
//   LDMX   //
//----------//
#include "Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include <TObject.h> //For ClassDef

namespace ldmx {

    class NonFidEcalVetoResult {

        public:

            /** Constructor */
            NonFidEcalVetoResult();

            /** Destructor */
            ~NonFidEcalVetoResult();

            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setVariables(
                    int nReadoutHits,
                    int deepestLayerHit,
                    int inside,
                    float summedDet,
                    float summedTightIso,
                    float maxCellDep,
                    float showerRMS,
                    float xStd,
                    float yStd,
                    float avgLayerHit,
                    float stdLayerHit,

                    std::vector<float> EcalLayerEdepReadout,
                    std::vector<double> recoilP,
                    std::vector<float> recoilPos,
                    std::vector<float> faceXY
            );

            /** Reset the object. */
            void Clear();

            /** Print the object */
            void Print(std::ostream& o) const;

            /** Checks if the event passes the Ecal veto. */
            std::vector<int> passesVeto() {
                return passesVeto_;
            }
            std::vector<float> getDisc() {
                return discValue_;
            }

            int getDeepestLayerHit() {
                return deepestLayerHit_;
            }

            int getNReadoutHits() {
                return nReadoutHits_;
            }

            float getSummedDet() {
                return summedDet_;
            }

            float getSummedTightIso() {
                return summedTightIso_;
            }

            float getMaxCellDep() {
                return maxCellDep_;
            }

            float getShowerRMS() {
                return showerRMS_;
            }

            float getXStd() {
                return xStd_;
            }

            float getYStd() {
                return yStd_;
            }

            float getAvgLayerHit() {
                return avgLayerHit_;
            }

            float getStdLayerHit() {
                return stdLayerHit_;
            }

            std::vector<float> getEcalLayerEdepReadout() {
                return ecalLayerEdepReadout_;
            }

            void setVetoResult(std::vector<int> passesVeto) {
                passesVeto_ = passesVeto;
            }
            void setDiscValue(std::vector<float> discValue) {
                discValue_ = discValue;
            }

            /** Return the momentum of the recoil at the front scoring plane. */
            std::vector<double> getRecoilMomentum() { return { recoilPx_, recoilPy_, recoilPz_ }; };

            /** Return the x position of the recoil at the front scoring plane. */
            double getRecoilX() { return recoilX_; };

            /** Return the y position of the recoil at the front scoring plane. */
            double getRecoilY() { return recoilY_; };

            double getFaceX() { return FaceX_; };

            double getFaceY() { return FaceY_; };

            int getInside() { return Inside_; };

        private:

            /** Flag indicating whether the event is vetoed by the Ecal. */
            std::vector<int> passesVeto_;

            int nReadoutHits_{0};
            int deepestLayerHit_{0};
            int Inside_{0};

            float summedDet_{0};
            float summedTightIso_{0};
            float maxCellDep_{0};
            float showerRMS_{0};
            float xStd_{0};
            float yStd_{0};
            float avgLayerHit_{0};
            float stdLayerHit_{0};


            std::vector<float> discValue_;

            /** px of recoil electron at the front scoring plane. */
            double recoilPx_{-9999};

            /** py of recoil electron at the front scoring plane. */
            double recoilPy_{-9999};

            /** py of recoil electron at the front scoring plane. */
            double recoilPz_{-9999};

            /** x position of recoil electron at the front scoring plane. */
            float recoilX_{-9999};

            /** y position of recoil electron at the front scoring plane. */
            float recoilY_{-9999};

            float FaceX_{-9999};

            float FaceY_{-9999};

            std::vector<float> ecalLayerEdepReadout_;

            ClassDef(NonFidEcalVetoResult, 3);
    };
}

#endif
