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
            kGENIE_GENERIC = 1000
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
                case VariationType ::kUNKNOWN:
                    return "UNKNOWN";
                case VariationType ::kGENIE_GENERIC:
                    return "GENIE_GENERIC";
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

            return VariationType::kUNKNOWN;
        }

    };
}

#endif //SIM_CORE_EventWeights_H
