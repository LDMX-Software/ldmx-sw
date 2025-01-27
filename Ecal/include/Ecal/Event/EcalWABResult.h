/**
 * @file EcalWABResult.h
 * @brief Class used to encapsulate the results obtained from EcalWAbRecProcessor
 * @author Sanjit Masanam (UCSB)
 */

#ifndef EVENT_ECALWABRESULT_H_
#define EVENT_ECALWABRESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <map>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>  //For ClassDef

namespace ldmx {

    class EcalWABResult {
    public:
        
        EcalWABResult() = default;
        
        /**
         * Destructor.
         *
         * Currently, the destructor does nothing.
         */
        virtual ~EcalWABResult() = default;
        
        /**
         * Print the string representation of this object.
         *
         * This class is needed by ROOT when building the dictionary.
         */
        void Print() const;
        
        // To match the Framework Bus clear. It's doing nothing
        void Clear(){};

        void setVariables(
            double trueThetaElectron,
            double trueThetaPhoton,
            double truePhiElectron,
            double truePhiPhoton,
            double recThetaElectron,
            double recThetaPhoton,
            double recPhiElectron,
            double recPhiPhoton,
            double trueThetaDiffElectronPhoton,
            double truePhiDiffElectronPhoton,
            double recThetaDiffElectronPhoton,
            double recPhiDiffElectronPhoton,
            double trueRecThetaDiffElectron,
            double trueRecPhiDiffElectron,
            double trueRecThetaDiffPhoton,
            double trueRecPhiDiffPhoton
        ) {
            trueThetaElectron_ = trueThetaElectron;
            trueThetaPhoton_ = trueThetaPhoton;
            truePhiElectron_ = truePhiElectron;
            truePhiPhoton_ = truePhiPhoton;
            recThetaElectron_ = recThetaElectron;
            recThetaPhoton_ = recThetaPhoton;
            recPhiElectron_ = recPhiElectron;
            recPhiPhoton_ = recPhiPhoton;
            trueThetaDiffElectronPhoton_ = trueThetaDiffElectronPhoton;
            truePhiDiffElectronPhoton_ = truePhiDiffElectronPhoton;
            recThetaDiffElectronPhoton_ = recThetaDiffElectronPhoton;
            recPhiDiffElectronPhoton_ = recPhiDiffElectronPhoton;
            trueRecThetaDiffElectron_ = trueRecThetaDiffElectron;
            trueRecPhiDiffElectron_ = trueRecPhiDiffElectron;
            trueRecThetaDiffPhoton_ = trueRecThetaDiffPhoton;
            trueRecPhiDiffPhoton_ = trueRecPhiDiffPhoton;
        }
        
        void setTrueThetaElectron(double trueThetaElectron) { trueThetaElectron_ = trueThetaElectron; }
        double getTrueThetaElectron() const { return trueThetaElectron_; }

        void setTrueThetaPhoton(double trueThetaPhoton) { trueThetaPhoton_ = trueThetaPhoton; }
        double getTrueThetaPhoton() const { return trueThetaPhoton_; }

        void setTruePhiElectron(double truePhiElectron) { truePhiElectron_ = truePhiElectron; }
        double getTruePhiElectron() const { return truePhiElectron_; }

        void setTruePhiPhoton(double truePhiPhoton) { truePhiPhoton_ = truePhiPhoton; }
        double getTruePhiPhoton() const { return truePhiPhoton_; }

        void setRecThetaElectron(double recThetaElectron) { recThetaElectron_ = recThetaElectron; }
        double getRecThetaElectron() const { return recThetaElectron_; }

        void setRecThetaPhoton(double recThetaPhoton) { recThetaPhoton_ = recThetaPhoton; }
        double getRecThetaPhoton() const { return recThetaPhoton_; }

        void setRecPhiElectron(double recPhiElectron) { recPhiElectron_ = recPhiElectron; }
        double getRecPhiElectron() const { return recPhiElectron_; }

        void setRecPhiPhoton(double recPhiPhoton) { recPhiPhoton_ = recPhiPhoton; }
        double getRecPhiPhoton() const { return recPhiPhoton_; }

        void setTrueThetaDiffElectronPhoton(double trueThetaDiffElectronPhoton) { trueThetaDiffElectronPhoton_ = trueThetaDiffElectronPhoton; }
        double getTrueThetaDiffElectronPhoton() const { return trueThetaDiffElectronPhoton_; }

        void setTruePhiDiffElectronPhoton(double truePhiDiffElectronPhoton) { truePhiDiffElectronPhoton_ = truePhiDiffElectronPhoton; }
        double getTruePhiDiffElectronPhoton() const { return truePhiDiffElectronPhoton_; }

        void setRecThetaDiffElectronPhoton(double recThetaDiffElectronPhoton) { recThetaDiffElectronPhoton_ = recThetaDiffElectronPhoton; }
        double getRecThetaDiffElectronPhoton() const { return recThetaDiffElectronPhoton_; }

        void setRecPhiDiffElectronPhoton(double recPhiDiffElectronPhoton) { recPhiDiffElectronPhoton_ = recPhiDiffElectronPhoton; }
        double getRecPhiDiffElectronPhoton() const { return recPhiDiffElectronPhoton_; }

        void setTrueRecThetaDiffElectron(double trueRecThetaDiffElectron) { trueRecThetaDiffElectron_ = trueRecThetaDiffElectron; }
        double getTrueRecThetaDiffElectron() const { return trueRecThetaDiffElectron_; }

        void setTrueRecPhiDiffElectron(double trueRecPhiDiffElectron) { trueRecPhiDiffElectron_ = trueRecPhiDiffElectron; }
        double getTrueRecPhiDiffElectron() const { return trueRecPhiDiffElectron_; }

        void setTrueRecThetaDiffPhoton(double trueRecThetaDiffPhoton) { trueRecThetaDiffPhoton_ = trueRecThetaDiffPhoton; }
        double getTrueRecThetaDiffPhoton() const { return trueRecThetaDiffPhoton_; }

        void setTrueRecPhiDiffPhoton(double trueRecPhiDiffPhoton) { trueRecPhiDiffPhoton_ = trueRecPhiDiffPhoton; }
        double getTrueRecPhiDiffPhoton() const { return trueRecPhiDiffPhoton_; }
        
    protected:
        //Actual Kinematic Parameters
        double trueThetaElectron_;
        double trueThetaPhoton_;
        double truePhiElectron_;
        double truePhiPhoton_;
        double recThetaElectron_;
        double recThetaPhoton_;
        double recPhiElectron_;
        double recPhiPhoton_;
        double trueThetaDiffElectronPhoton_;
        double truePhiDiffElectronPhoton_;
        double recThetaDiffElectronPhoton_;
        double recPhiDiffElectronPhoton_;
        double trueRecThetaDiffElectron_;
        double trueRecPhiDiffElectron_;
        double trueRecThetaDiffPhoton_;
        double trueRecPhiDiffPhoton_;
        
        /// Class declaration needed by the ROOT dictionary.
        ClassDef(EcalWABResult, 1);
        
    };  // EcalWABResult

    typedef std::vector<ldmx::EcalWABResult> EcalWABResults;

}  // namespace ldmx

#endif // EVENT_ECALWABRESULT_H_