#include "Event/ClusterAlgoResult.h"

ClassImp(ldmx::ClusterAlgoResult)

namespace ldmx {

    ClusterAlgoResult::ClusterAlgoResult() {
    }

    ClusterAlgoResult::~ClusterAlgoResult() {
        Clear();
    }

    void ClusterAlgoResult::Print(std::ostream& o) const {

        o << "ClusterAlgoResult { " << "name: " << name_ << " }\n";

        for (int i = 0; i < variables_.GetSize(); ++i) {
            o << "\tElement " << i << " : " << variables_[i];
            if ( i+1 < variables_.GetSize() ) o << '\n';
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
