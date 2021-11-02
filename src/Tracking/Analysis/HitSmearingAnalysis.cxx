#include "Tracking/Analysis/HitSmearingAnalysis.h"

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"

namespace tracking {
  namespace analysis {

    void HitSmearingAnalysis::onProcessStart() {

      getHistoDirectory();

      histograms_.create("tagger_res_x_position","Residuals x position (hit-smeared_hit)",100,-1,1);
      histograms_.create("tagger_res_y_position","Residuals y position (hit-smeared_hit)",100,-1,1);
      
      histograms_.create("recoil_res_x_position","Residuals x position (hit-smeared_hit)",100,-1,1);
      histograms_.create("recoil_res_y_position","Residuals y position (hit-smeared_hit)",100,-1,1);
      
    }
    
    
    void HitSmearingAnalysis::analyze(const framework::Event& event) {

      std::cout<<"EVENT NR"<< event.getEventNumber()<<std::endl;
      
      auto tagger_sim_hits{
	event.getCollection<ldmx::SimTrackerHit>("TaggerSimHits")
      };

      auto smeared_tagger_sim_hits{
	event.getCollection<ldmx::SimTrackerHit>("SmearedTaggerSimHits")
      };
      
      for (unsigned int i_hit;i_hit<tagger_sim_hits.size(); i_hit++) {
	auto sim_hit_pos{tagger_sim_hits[i_hit].getPosition()};
	auto smeared_hit_pos{smeared_tagger_sim_hits[i_hit].getPosition()};

	histograms_.fill("tagger_res_x_position", sim_hit_pos[0] -  smeared_hit_pos[0]);
	histograms_.fill("tagger_res_y_position", sim_hit_pos[1] -  smeared_hit_pos[1]);
      }
      

      auto recoil_sim_hits{
	event.getCollection<ldmx::SimTrackerHit>("RecoilSimHits")
      };

      auto smeared_recoil_sim_hits{
	event.getCollection<ldmx::SimTrackerHit>("SmearedRecoilSimHits")
      };
      
      for (unsigned int i_hit;i_hit<recoil_sim_hits.size(); i_hit++) {
	auto sim_hit_pos{recoil_sim_hits[i_hit].getPosition()};
	auto smeared_hit_pos{smeared_recoil_sim_hits[i_hit].getPosition()};

	histograms_.fill("recoil_res_x_position", sim_hit_pos[0] -  smeared_hit_pos[0]);
	histograms_.fill("recoil_res_y_position", sim_hit_pos[1] -  smeared_hit_pos[1]);
      }
            
      return;
    }
    
  }  // namespace analysis
} // namespace tracking

DECLARE_ANALYZER_NS(tracking::analysis, HitSmearingAnalysis)
