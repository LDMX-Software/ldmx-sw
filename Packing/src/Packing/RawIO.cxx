
#include "Packing/RawIO.h"

namespace packing {

void RawIO::configure(framework::config::Parameters& ps) {
  raw_file_ = std::make_unique<rawdatafile::File>(ps.getParameter<framework::config::Parameters>("raw_file"));
}

void RawIO::beforeNewRun(ldmx::RunHeader& header) {
  raw_file_->writeRunHeader(header);
}

void RawIO::produce(framework::Event& event) {
  raw_file_->connect(event);
  if (not raw_file_->nextEvent()) {
    abortEvent();
  }
}

void RawIO::onProcessEnd() {
  raw_file_->close();
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, RawIO)
