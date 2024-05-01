#pragma once

//---< SimCore >---//
#include "SimCore/Event/SimTrackerHit.h"
#include <set>

namespace tracking {
  namespace digitization {

    class SiElectrodeData {
      
   public: 

      SiElectrodeData();
              
      SiElectrodeData(int charge) {
        charge_ = charge;
      }

      ~SiElectrodeData() {
        sim_hits_.clear();
      }
      
      SiElectrodeData(int charge, const ldmx::SimTrackerHit& sim_hit) {

        charge_ = charge;
        sim_hits_.insert(sim_hit);
      }
            
      // TODO:: Maybe use the vector directly?
      SiElectrodeData(int charge,
                      const std::vector<ldmx::SimTrackerHit>& sim_hits) {
        
        charge_ = charge;
        
        for (auto& sim_hit : sim_hits)  {
          sim_hits_.insert(sim_hit);
        }
      }
      

      bool isValid() const {
        return (getCharge() != 0);
      }

      int getCharge() const {return charge_;}

      std::set<ldmx::SimTrackerHit> getSimulatedHits() const {
        return sim_hits_;
      }


      //TODO Change to operator overloading for cleaner code
      SiElectrodeData add(const SiElectrodeData& electrode_data);
      
      SiElectrodeData add(int charge, std::set<ldmx::SimTrackerHit> simulated_hits);
            
      SiElectrodeData addCharge(int charge);
            
      SiElectrodeData addSimulatedHit(const ldmx::SimTrackerHit hit);
      
      
   private: 
      
      int charge_{};
      std::set<ldmx::SimTrackerHit> sim_hits_;
      
    };
    
    
  } // digitization
} // tracking
