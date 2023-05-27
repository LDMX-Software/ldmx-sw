#ifndef WHITE_RABBIT_RESULT_H
#define WHITE_RABBIT_RESULT_H

#include <vector>
#include <stdint.h>

#include "TObject.h"

namespace beaminstrumentation {
  class WhiteRabbitResult {
  public:
    WhiteRabbitResult() = default;
    WhiteRabbitResult(uint32_t deltaTTrigger, uint32_t deltaTDownstreamHorizontal, uint32_t deltaTDownstreamVertical, uint32_t deltaTLowPressure, uint32_t deltaTHighPressure, uint64_t deltaTSpillStart, uint32_t spillNumber) : deltaTTrigger(deltaTTrigger), deltaTDownstreamHorizontal(deltaTDownstreamHorizontal), deltaTDownstreamVertical(deltaTDownstreamVertical), deltaTLowPressure(deltaTLowPressure), deltaTHighPressure(deltaTHighPressure), deltaTSpillStart(deltaTSpillStart), spillNumber(spillNumber) {};
    ~WhiteRabbitResult() = default;

    void Print(Option_t* option = "") const {};
    void Clear(Option_t* option = "") {};
    bool operator <(const WhiteRabbitResult &rhs) const {
      return true;
    }

  private:
    uint32_t deltaTTrigger; //deltaT between TS event and scintillator plate trigger
    uint32_t deltaTDownstreamHorizontal; //deltaT between TS event and downstream horizontal fiber tracker
    uint32_t deltaTDownstreamVertical; //deltaT between TS event and downstream vertical fiber tracker
    
    uint32_t deltaTUpstreamHorizontal; //deltaT between TS events and upstream horizontal fiber tracker
    uint32_t deltaTUpstreamVertical; //deltaT between TS events and upstream vertical fiber tracker

    uint32_t deltaTLowPressure; //deltaT between TS events and low pressure cherenkov signal
    uint32_t deltaTHighPressure; //deltaT between TS events and high pressure cherenkov signal

    uint64_t deltaTSpillStart; //deltaT between TS events and start of spill signal (as measured by the white rabbit, not the timestamp given by the TS' FPGA)
    uint32_t spillNumber;
    

    /*
     * Possible additions
     */
    //The offsets used in alignment
    //deltaTs to the second closest events, to see systematic effects and to judge the chance of misalignment
    //Spill information like:
    //Actual number of raw triggers in spill
    //Actual number of fiber tracker events in spill
    //Number of cherenkov hits etc. in spill

    ClassDef(WhiteRabbitResult, 1);

  };

} // namespace beaminstrumentation

#endif //WHITE_RABBIT_RESULT_H
