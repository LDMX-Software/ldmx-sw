#ifndef DISCRETEINPUTS_IO
#define DISCRETEINPUTS_IO

#include <vector>

#include "../../../Algo_HLS/Ecal/src/data.h"
#include "DiscreteInputs.h"

namespace trigger {

struct EventDump {
  uint64_t event;
  std::vector<ldmx_int::EcalTP> EcalTPs;

 EventDump() : event(0), EcalTPs(){}
  bool readFromFile(FILE *file) {
    if (!fread(&event, sizeof(uint64_t), 1, file)) return false;
    ldmx_int::readManyFromFile(EcalTPs, file);
    return true;
  }
  bool writeToFile(FILE *file) {
    fwrite(&event, sizeof(uint64_t), 1, file);
    ldmx_int::writeManyToFile(EcalTPs, file);
    return true;
  }
};
    
class DiscreteInputs {
public:
DiscreteInputs(const char *fileName) : file_(fopen(fileName,"rb")) {
    if (!file_) { std::cout << "ERROR: cannot read '" << fileName << "'" << std::endl; }
    assert(file_);
  }
  ~DiscreteInputs() { fclose(file_); }

  bool nextEvent() {
    if (feof(file_)) return false;
    if (!event_.readFromFile(file_)) return false;
    printf("Beginning of event %lu (%d TPs) \n", event_.event, event_.EcalTPs.size());
    return true;
  }
  const EventDump & event() { return event_; }
    
private:    
  FILE *file_;
  EventDump event_;
    
};

}

#endif /* DISCRETEINPUTS_IO */
