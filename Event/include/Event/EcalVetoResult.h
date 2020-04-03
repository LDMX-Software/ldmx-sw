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
#include <TObject.h> //For ClassDef

namespace ldmx {

    class EcalVetoResult {

        public:

            /** Constructor */
            EcalVetoResult();

            /** Destructor */
            ~EcalVetoResult();

            /**
             * Set the sim particle and 'is findable' flag.
             */
            void setVariables(
                    int nReadoutHits,
                    int deepestLayerHit,
                    float summedDet,
                    float summedTightIso,
                    float maxCellDep,
                    float showerRMS,
                    float xStd,
                    float yStd,
                    float avgLayerHit,
                    float stdLayerHit,
                    float ecalBackEnergy,

                    std::vector<float> electronContainmentEnergy,
                    std::vector<float> photonContainmentEnergy,
                    std::vector<float> outsideContainmentEnergy,
                    std::vector<int>   outsideContainmentNHits,
                    std::vector<float> outsideContainmentXStd,
                    std::vector<float> outsideContainmentYStd,

                    std::vector<float> EcalLayerEdepReadout,
                    std::vector<double> recoilP,
                    std::vector<float> recoilPos
            );

            /** Reset the object. */
            void Clear();

            /** Print the object */
            void Print() const;

            /** Checks if the event passes the Ecal veto. */
            bool passesVeto() const {
                return passesVeto_;
            }
            float getDisc() const {
                return discValue_;
            }

            int getDeepestLayerHit() const {
                return deepestLayerHit_;
            }

            int getNReadoutHits() const {
                return nReadoutHits_;
            }

            float getSummedDet() const {
                return summedDet_;
            }

            float getSummedTightIso() const {
                return summedTightIso_;
            }

            float getMaxCellDep() const {
                return maxCellDep_;
            }

            float getShowerRMS() const {
                return showerRMS_;
            }

            float getXStd() const {
                return xStd_;
            }

            float getYStd() const {
                return yStd_;
            }

            float getAvgLayerHit() const {
                return avgLayerHit_;
            }

            float getStdLayerHit() const {
                return stdLayerHit_;
            }

            float getEcalBackEnergy() const {
                return ecalBackEnergy_;
            }

            const std::vector<float>& getElectronContainmentEnergy() const {
                return electronContainmentEnergy_;
            }

            const std::vector<float>& getPhotonContainmentEnergy() const {
                return photonContainmentEnergy_;
            }

            const std::vector<float>& getOutsideContainmentEnergy() const {
                return outsideContainmentEnergy_;
            }

            const std::vector<int>& getOutsideContainmentNHits() const {
                return outsideContainmentNHits_;
            }

            const std::vector<float>& getOutsideContainmentXStd() const {
                return outsideContainmentXStd_;
            }

            const std::vector<float>& getOutsideContainmentYStd() const {
                return outsideContainmentYStd_;
            }

            const std::vector<float>& getEcalLayerEdepReadout() const {
                return ecalLayerEdepReadout_;
            }

            void setVetoResult(bool passesVeto) {
                passesVeto_ = passesVeto;
            }
            void setDiscValue(float discValue) {
                discValue_ = discValue;
            }

            /** Return the momentum of the recoil at the Ecal face. */
            std::vector<double> getRecoilMomentum() { return { recoilPx_, recoilPy_, recoilPz_ }; };

            /** Return the x position of the recoil at the Ecal face. */
            double getRecoilX() const { return recoilX_; };

            /** Return the y position of the recoil at the Ecal face. */
            double getRecoilY() const { return recoilY_; };

        private:

            /** Flag indicating whether the event is vetoed by the Ecal. */
            bool passesVeto_{false};

            int nReadoutHits_{0};
            int deepestLayerHit_{0};

            float summedDet_{0};
            float summedTightIso_{0};
            float maxCellDep_{0};
            float showerRMS_{0};
            float xStd_{0};
            float yStd_{0};
            float avgLayerHit_{0};
            float stdLayerHit_{0};
            float ecalBackEnergy_{0};

            std::vector<float> electronContainmentEnergy_;
            std::vector<float> photonContainmentEnergy_;
            std::vector<float> outsideContainmentEnergy_;
            std::vector<int>   outsideContainmentNHits_;
            std::vector<float> outsideContainmentXStd_;
            std::vector<float> outsideContainmentYStd_;

            float discValue_{0};

            /** px of recoil electron at the Ecal face. */
            double recoilPx_{-9999};

            /** py of recoil electron at the Ecal face. */
            double recoilPy_{-9999};

            /** py of recoil electron at the Ecal face. */
            double recoilPz_{-9999};

            /** x position of recoil electron at the Ecal face. */
            float recoilX_{-9999};

            /** y position of recoil electron at the Ecal face. */
            float recoilY_{-9999};

            std::vector<float> ecalLayerEdepReadout_;

            ClassDef(EcalVetoResult, 4);
    };
}

#endif
