/**
 * @file PnWeightResult.cxx
 * @brief Class used to encapsulate the results obtained from
 *        PnWeightProcessor.
 * @author Alex Patterson, UCSB
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/PnWeightResult.h"

ClassImp(ldmx::PnWeightResult)

namespace ldmx {

    PnWeightResult::PnWeightResult() :
        TObject() {
    }

    PnWeightResult::~PnWeightResult() {
        Clear();
    }

    void PnWeightResult::setResult(double ke, double theta, double w, double fitW, double weight,
                                   double ke_hard,  double p_hard,  double pz_hard,  double w_hard,  double theta_hard, int A_hard,
                                   double ke_heavy, double p_heavy, double pz_heavy, double w_heavy, double theta_heavy, int A_heavy,
                                   double ke_dau,   double p_dau,   double pz_dau,   double w_dau,   double theta_dau, int pdg_dau
                                  ){
        ke_     = ke;  
        theta_  = theta; 
        w_      = w;
        fitW_   = fitW;
        weight_ = weight;

        ke_hard_    = ke_hard;
        p_hard_     = p_hard;
        pz_hard_    = pz_hard;
        w_hard_     = w_hard;
        theta_hard_ = theta_hard;
        A_hard_     = A_hard;

        ke_heavy_    = ke_heavy;
        p_heavy_     = p_heavy;
        pz_heavy_    = pz_heavy;
        w_heavy_     = w_heavy;
        theta_heavy_ = theta_heavy;
        A_heavy_     = A_heavy;

        ke_dau_    = ke_dau;
        p_dau_     = p_dau;
        pz_dau_    = pz_dau;
        w_dau_     = w_dau;
        theta_dau_ = theta_dau;
        pdg_dau_   = pdg_dau;
    }

    void PnWeightResult::Clear(Option_t *option) {
        TObject::Clear();

        ke_     = -10.; 
        theta_  = -10.;
        w_      = -10.;
        fitW_   = -10.;
        weight_ = 1.;

        ke_hard_    = -10.;
        p_hard_     = -10.;
        pz_hard_    = -10.;
        w_hard_     = -10.;
        theta_hard_ = -10.;
        A_hard_     = -10;

        ke_heavy_    = -10.;
        p_heavy_     = -10.;
        pz_heavy_    = -10.;
        w_heavy_     = -10.;
        theta_heavy_ = -10.;
        A_heavy_     = -10;

        ke_dau_    = -10.;
        p_dau_     = -10.;
        pz_dau_    = -10.;
        w_dau_     = -10.;
        theta_dau_ = -10.;
        pdg_dau_   = -10;
    }

    void PnWeightResult::Copy(TObject& object) const { 
        PnWeightResult& result = (PnWeightResult&) object; 

        result.ke_     = ke_; 
        result.theta_  = theta_;  
        result.w_      = w_; 
        result.fitW_   = fitW_;
        result.weight_ = weight_; 

        result.ke_hard_    = ke_hard_;
        result.p_hard_     = p_hard_;
        result.pz_hard_    = pz_hard_;
        result.w_hard_     = w_hard_;
        result.theta_hard_ = theta_hard_;
        result.A_hard_     = A_hard_;

        result.ke_heavy_    = ke_heavy_;
        result.p_heavy_     = p_heavy_;
        result.pz_heavy_    = pz_heavy_;
        result.w_heavy_     = w_heavy_;
        result.theta_heavy_ = theta_heavy_;
        result.A_heavy_     = A_heavy_;

        result.ke_dau_    = ke_dau_;
        result.p_dau_     = p_dau_;
        result.pz_dau_    = pz_dau_;
        result.w_dau_     = w_dau_;
        result.theta_dau_ = theta_dau_;
        result.pdg_dau_   = pdg_dau_;
    }

    void PnWeightResult::Print(Option_t *option) const {
        std::cout << "[ PnWeightResult ]:\n" 
                  << "\t W fit : "      << fitW_ << "\n"
                  << "\t PN daughter kinetic energy: " << ke_ << "\n"
                  << "\t W measured : " << w_ << "\n"
                  << "\t PN daughter polar angle: " << theta_ << "\n"
                  << "\t PN Weight : "    << weight_ << "\n" 
                  << std::endl;
    }
}
