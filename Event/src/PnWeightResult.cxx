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

    void PnWeightResult::setResult(
                           double fitWp, double ke, double wp, double theta, double weight,
                           double fitWp_max, double ke_max, double wp_max, double theta_max, double weight_max,
                           double fitWp_p, double ke_p, double wp_p, double theta_p, double weight_p
                                  ){
        fitWp_ = fitWp;
        ke_ = ke;  
        wp_ = wp;
        theta_ = theta; 
        weight_ = weight;

        fitWp_max_ = fitWp_max;
        ke_max_ = ke_max;
        wp_max_ = wp_max;
        theta_max_ = theta_max;
        weight_max_ = weight_max;

        fitWp_p_ = fitWp_p;
        ke_p_ = ke_p;
        wp_p_ = wp_p;
        theta_p_ = theta_p;
        weight_p_ = weight_p;
    }

    void PnWeightResult::Clear(Option_t *option) {
        TObject::Clear();

        fitWp_ = 0.0;
        ke_ = 0; 
        wp_ = 0.0;
        theta_ = 0.0;
        weight_ = 0.0;

        fitWp_max_ = 0.0;
        ke_max_ = 0.0;
        wp_max_ = 0.0;
        theta_max_ = 0.0;
        weight_max_ = 0.0;

        fitWp_p_ = 0.0;
        ke_p_ = 0.0;
        wp_p_ = 0.0;
        theta_p_ = 0.0;
        weight_p_ = 0.0;
    }

    void PnWeightResult::Copy(TObject& object) const { 
        PnWeightResult& result = (PnWeightResult&) object; 

        result.fitWp_ = fitWp_;
        result.ke_ = ke_; 
        result.theta_ = theta_;  
        result.wp_ = wp_; 
        result.weight_ = weight_; 

        result.fitWp_max_ = fitWp_max_;
        result.ke_max_ = ke_max_;
        result.wp_max_ = wp_max_;
        result.theta_max_ = theta_max_;
        result.weight_max_ = weight_max_;

        result.fitWp_p_ = fitWp_p_;
        result.ke_p_ = ke_p_;
        result.wp_p_ = wp_p_;
        result.theta_p_ = theta_p_;
        result.weight_p_ = weight_p_;
    }

    void PnWeightResult::Print(Option_t *option) const {
        std::cout << "[ PnWeightResult ]:\n" 
                  << "\t W_p fit : "      << fitWp_ << "\n"
                  << "\t PN daughter kinetic energy: " << ke_ << "\n"
                  << "\t W_p measured : " << wp_ << "\n"
                  << "\t PN daughter polar angle: " << theta_ << "\n"
                  << "\t PN Weight : "    << weight_ << "\n" 
                  << std::endl;
    }
}
