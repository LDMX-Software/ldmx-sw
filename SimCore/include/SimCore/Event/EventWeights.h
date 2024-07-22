//
// Created by Wesley Ketchum on 4/27/24.
//

#ifndef SIM_CORE_EventWeights_H
#define SIM_CORE_EventWeights_H

#include "TObject.h"
#include <vector>
#include <map>

namespace ldmx {

    class EventWeights {
    public:

        //Enum to define
        enum VariationType {
            kINVALID = -1,
            kUNKNOWN = 0,

            kGENIE_GENERIC  = 1000,
            kGENIE_INukeTwkDial             = kGENIE_GENERIC + 100,
            kGENIE_INukeTwkDial_MFP_pi      = kGENIE_INukeTwkDial+1,
            kGENIE_INukeTwkDial_MFP_N       = kGENIE_INukeTwkDial+2,
            kGENIE_INukeTwkDial_FrCEx_pi    = kGENIE_INukeTwkDial+3,
            kGENIE_INukeTwkDial_FrInel_pi   = kGENIE_INukeTwkDial+4,
            kGENIE_INukeTwkDial_FrAbs_pi    = kGENIE_INukeTwkDial+5,
            kGENIE_INukeTwkDial_FrPiProd_pi = kGENIE_INukeTwkDial+6,
            kGENIE_INukeTwkDial_FrCEx_N    = kGENIE_INukeTwkDial+7,
            kGENIE_INukeTwkDial_FrInel_N   = kGENIE_INukeTwkDial+8,
            kGENIE_INukeTwkDial_FrAbs_N    = kGENIE_INukeTwkDial+9,
            kGENIE_INukeTwkDial_FrPiProd_N = kGENIE_INukeTwkDial+10,

            kGENIE_HadrNuclTwkDial          = kGENIE_GENERIC + 150,
            kGENIE_HadrNuclTwkDial_FormZone = kGENIE_HadrNuclTwkDial+1
        };

        EventWeights() = default;
        virtual ~EventWeights() = default;

        //constructor where variation map is declared
        EventWeights( std::map< VariationType, std::vector<double> > var_map )
        : variations_map_(var_map) {}

        void Clear();
        void Print();

        std::vector<double> getWeights() const { return weights_; }
        std::map< VariationType, std::vector<double> > getVariations() const { return variations_map_; }

        size_t getNumWeights() const { return weights_.size(); }
        size_t getNumVariationTypes() const { return variations_map_.size(); }

        double getNthWeight(size_t i_w) const { return weights_.at(i_w); }

        std::map< VariationType, double > getVariationsNthWeight(size_t i_w) const;

        void addWeight(double w) { weights_.push_back(w); }
        void setWeight(size_t i_w, double w) { weights_[i_w] = w; }

    private:
        //set of event weights (n weights per event)
        std::vector<double> weights_;

        //map of the variations used:  key is variation type, value is list of variation values (n per event)
        std::map< VariationType, std::vector<double> > variations_map_;

    public:

        inline static std::string variation_type_to_string(const VariationType& vtype) {
            switch (vtype) {
                case VariationType::kINVALID:
                    return "INVALID";
                case VariationType::kUNKNOWN:
                    return "UNKNOWN";
                case VariationType::kGENIE_GENERIC:
                    return "GENIE_GENERIC";
                case VariationType::kGENIE_INukeTwkDial:
                  return "GENIE_INukeTwkDial";
                case VariationType::kGENIE_INukeTwkDial_MFP_pi:
                  return "GENIE_INukeTwkDial_MFP_pi";
                case VariationType::kGENIE_INukeTwkDial_MFP_N:
                  return "GENIE_INukeTwkDial_MFP_N";
                case VariationType::kGENIE_INukeTwkDial_FrCEx_pi:
                  return "GENIE_INukeTwkDial_FrCEx_pi";
                case VariationType::kGENIE_INukeTwkDial_FrInel_pi:
                  return "GENIE_INukeTwkDial_FrInel_pi";
                case VariationType::kGENIE_INukeTwkDial_FrAbs_pi:
                  return "GENIE_INukeTwkDial_FrAbs_pi";
                case VariationType::kGENIE_INukeTwkDial_FrPiProd_pi:
                  return "GENIE_INukeTwkDial_FrPiProd_pi";
                case VariationType::kGENIE_INukeTwkDial_FrCEx_N:
                  return "GENIE_INukeTwkDial_FrCEx_N";
                case VariationType::kGENIE_INukeTwkDial_FrInel_N:
                  return "GENIE_INukeTwkDial_FrInel_N";
                case VariationType::kGENIE_INukeTwkDial_FrAbs_N:
                  return "GENIE_INukeTwkDial_FrAbs_N";
                case VariationType::kGENIE_INukeTwkDial_FrPiProd_N:
                  return "GENIE_INukeTwkDial_FrPiProd_N";
                case VariationType ::kGENIE_HadrNuclTwkDial:
                  return "GENIE_HadrNuclTwkDial";
                case VariationType ::kGENIE_HadrNuclTwkDial_FormZone:
                  return "GENIE_HadrNuclTwkDial_FormZone";
            }
            return "UNKNOWN";
        }
        inline static VariationType string_to_variation_type(const std::string& typestring) {

            if(typestring=="UNKNOWN")
                return VariationType::kUNKNOWN;
            else if(typestring=="INVALID")
                return VariationType::kINVALID;
            else if(typestring=="GENIE_GENERIC")
                return VariationType::kGENIE_GENERIC;
            else if(typestring=="GENIE_INukeTwkDial")
                return VariationType::kGENIE_INukeTwkDial;
            else if(typestring=="GENIE_INukeTwkDial_MFP_pi")
              return VariationType::kGENIE_INukeTwkDial_MFP_pi;
            else if(typestring=="GENIE_INukeTwkDial_MFP_N")
              return VariationType::kGENIE_INukeTwkDial_MFP_N;
            else if(typestring=="GENIE_INukeTwkDial_FrCEx_pi")
              return VariationType::kGENIE_INukeTwkDial_FrCEx_pi;
            else if(typestring=="GENIE_INukeTwkDial_FrInel_pi")
              return VariationType::kGENIE_INukeTwkDial_FrInel_pi;
            else if(typestring=="GENIE_INukeTwkDial_FrAbs_pi")
              return VariationType::kGENIE_INukeTwkDial_FrAbs_pi;
            else if(typestring=="GENIE_INukeTwkDial_FrPiProd_pi")
              return VariationType::kGENIE_INukeTwkDial_FrPiProd_pi;
            else if(typestring=="GENIE_INukeTwkDial_FrCEx_N")
              return VariationType::kGENIE_INukeTwkDial_FrCEx_N;
            else if(typestring=="GENIE_INukeTwkDial_FrInel_N")
              return VariationType::kGENIE_INukeTwkDial_FrInel_N;
            else if(typestring=="GENIE_INukeTwkDial_FrAbs_N")
              return VariationType::kGENIE_INukeTwkDial_FrAbs_N;
            else if(typestring=="GENIE_INukeTwkDial_FrPiProd_N")
              return VariationType::kGENIE_INukeTwkDial_FrPiProd_N;
            else if(typestring=="GENIE_HadrNuclTwkDial")
              return VariationType::kGENIE_HadrNuclTwkDial;
            else if(typestring=="GENIE_HadrNuclTwkDial_FormZone")
              return VariationType::kGENIE_HadrNuclTwkDial_FormZone;

            return VariationType::kUNKNOWN;
        }

    };
}

#endif //SIM_CORE_EventWeights_H
