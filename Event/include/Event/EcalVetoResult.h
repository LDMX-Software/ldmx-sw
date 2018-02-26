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
                    
                    std::vector<float> EcalLayerEdepReadout,
                    std::vector<double> recoilP, 
                    std::vector<float> recoilPos
            );

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

            void setVetoResult(bool passesVeto) {
                passesVeto_ = passesVeto;
            }
            void setDiscValue(float discValue) {
                discValue_ = discValue;
            }

            /** Return the momentum of the recoil at the Ecal face. */
            std::vector<double> getRecoilMomentum() { return { recoilPx_, recoilPy_, recoilPz_ }; }; 

            /** Return the x position of the recoil at the Ecal face. */
            double getRecoilX() { return recoilX_; }; 

            /** Return the y position of the recoil at the Ecal face. */
            double getRecoilY() { return recoilY_; };

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

            ClassDef(EcalVetoResult, 3);
    };
}

#endif
