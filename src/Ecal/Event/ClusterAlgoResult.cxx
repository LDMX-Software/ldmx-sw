#include "Ecal/Event/ClusterAlgoResult.h"

ClassImp(ldmx::ClusterAlgoResult)

namespace ldmx {

    ClusterAlgoResult::ClusterAlgoResult() {
    }

    ClusterAlgoResult::~ClusterAlgoResult() {
        Clear();
    }

    void ClusterAlgoResult::Print() const {

        std::cout << "ClusterAlgoResult { " << "name: " << name_ << " }" << std::endl;

        for (int i = 0; i < variables_.GetSize(); ++i) {
            std::cout << "Element " << i << " : " << variables_[i] << std::endl;
        }
    }

    void ClusterAlgoResult::Clear() {

        name_ = "";

        for (int i = 0; i < variables_.GetSize(); ++i) {
            variables_[i] = 0;
        }
    }

    void ClusterAlgoResult::set(const TString& name, int nvar) {

        name_ = name;

        if (nvar > variables_.GetSize()) {
            variables_.Set(nvar);
        }
    }
    
    void ClusterAlgoResult::set(const TString& name, int nvar, int nweights) {

        name_ = name;

        if (nvar > variables_.GetSize()) {
            variables_.Set(nvar);
        }

        if (nweights > weights_.GetSize()) {
            weights_.Set(nweights);
        }
    }

    void ClusterAlgoResult::setAlgoVar(int element, double value) {
        if (element >= 0 && element < variables_.GetSize()) {
            variables_[element] = value;
        }
    }

    void ClusterAlgoResult::setWeight(int nCluster, double weight) {
        if (nCluster >= 0 && nCluster < weights_.GetSize()) {
            weights_[nCluster] = weight;
        }
    }
}
