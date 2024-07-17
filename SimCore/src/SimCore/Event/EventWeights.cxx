//
// Created by Wesley Ketchum on 4/28/24.
//

#include "SimCore/Event/EventWeights.h"
#include <iostream>

namespace ldmx {

    void EventWeights::Clear() {
        weights_.clear();
        variations_map_.clear();
    }

    void EventWeights::Print() {
        std::cout << "Num weights: " << getNumWeights() << std::endl;
        for(size_t i_w=0; i_w<weights_.size(); ++i_w){
            std::cout << "\t" << i_w << ": weight=" << weights_[i_w];
            for(auto const& v : variations_map_) {
                std::cout << "\n\t var_type=" << v.first << ", var_value=" << v.second[i_w];
            }
            std::cout << std::endl;
        }
    }
}