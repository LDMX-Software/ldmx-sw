#include "EventDisplay/ISpyRecHits.h"

#include "Framework/ProductTag.h"

#include "SimCore/Event/SimCalorimeterHit.h"
#include "SimCore/Event/SimParticle.h"
#include "SimCore/Event/SimTrackerHit.h"
#include <vector>
#include <string>

std::vector<std::vector<double>> hits = {};

namespace ldmx {
    void analyze(const framework::Event& event){
        auto particle_map{event.getMap<int, SimParticle>("TaggerSimHits_test")};
        for (auto const& it : particle_map) {
            std::vector<double> vert = it.getVertex();
            hits.push_back(vert);
        }
        
        for(int i = 0; i < hits.size(); i++){
            for(int j = 0; j < hits[i].size(); j++){
                std::cout << hits[i][j] << ", " << std::endl;
            }
        }
        return;
    }
}
DECLARE_ANALYZER_NS(ldmx, ISpyRecHits)