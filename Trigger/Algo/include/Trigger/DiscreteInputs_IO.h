#ifndef DISCRETEINPUTS_IO
#define DISCRETEINPUTS_IO

#include <vector>

#include "../../../Algo_HLS/Ecal/src/data.h"
#include "DiscreteInputs.h"

namespace trigger {

struct EventDump {
  uint64_t event_;
  std::vector<ldmx_int::EcalTP> ecal_tps_;

  EventDump() : event_(0), ecal_tps_() {}
  bool readFromFile(FILE *file) {
    if (!fread(&event_, sizeof(uint64_t), 1, file)) return false;
    ldmx_int::readManyFromFile(ecal_tps_, file);
    return true;
  }
  bool writeToFile(FILE *file) {
    fwrite(&event_, sizeof(uint64_t), 1, file);
    ldmx_int::writeManyToFile(ecal_tps_, file);
    return true;
  }
};

class DiscreteInputs {
 public:
  DiscreteInputs(const char *fileName) : file_(fopen(fileName, "rb")) {
    if (!file_) {
      std::cout << "ERROR: cannot read '" << fileName << "'" << std::endl;
    }
    assert(file_);
  }
  ~DiscreteInputs() { fclose(file_); }

  bool nextEvent() {
    if (feof(file_)) return false;
    if (!event_.readFromFile(file_)) return false;
    printf("Beginning of event %lu (%lu TPs) \n", event_.event_,
           event_.ecal_tps_.size());
    return true;
  }
  const EventDump &event() { return event_; }

 private:
  FILE *file_;
  EventDump event_;
};

}  // namespace trigger

#endif /* DISCRETEINPUTS_IO */
