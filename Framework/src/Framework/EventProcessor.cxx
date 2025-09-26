#include "Framework/EventProcessor.h"

// LDMX
#include "Framework/Process.h"
#include "Framework/RunHeader.h"
#include "TDirectory.h"

namespace framework {

EventProcessor::EventProcessor(const std::string &name, Process &process)
    : histograms_{[this]() -> TDirectory * {
        return this->getHistoDirectory();
      }},
      the_log_{logging::makeLogger(name)},
      process_{process},
      name_{name} {}

Conditions &EventProcessor::getConditions() const {
  return process_.getConditions();
}

const ldmx::EventHeader &EventProcessor::getEventHeader() const {
  return *(process_.getEventHeader());
}

TDirectory *EventProcessor::getHistoDirectory() {
  if (!histo_dir_) {
    histo_dir_ = process_.makeHistoDirectory(name_);
  }
  histo_dir_->cd();  // make this the current directory
  return histo_dir_;
}

void EventProcessor::setStorageHint(framework::StorageControl::Hint hint,
                                    const std::string &purposeString) {
  process_.getStorageController().addHint(name_, hint, purposeString);
}

int EventProcessor::getLogFrequency() const {
  return process_.getLogFrequency();
}

int EventProcessor::getRunNumber() const { return process_.getRunNumber(); }

void EventProcessor::createHistograms(
    const std::vector<framework::config::Parameters> &histos) {
  for (auto const &h : histos) {
    histograms_.create(h);
  }
}

DEFINE_FACTORY(EventProcessor);

Producer::Producer(const std::string &name, Process &process)
    : EventProcessor(name, process) {}

Analyzer::Analyzer(const std::string &name, Process &process)
    : EventProcessor(name, process) {}
}  // namespace framework
