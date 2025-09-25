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
    auto name{h.get<std::string>("name")};
    auto x_label{h.get<std::string>("xlabel")};
    auto xbins{h.get<std::vector<double>>("xbins")};
    auto y_label{h.get<std::string>("ylabel")};
    auto ybins{h.get<std::vector<double>>("ybins", {})};
    if (ybins.empty()) {
      // assume 1D histogram
      histograms_.create(name, x_label, xbins);
    } else {
      // assume 2D histogram
      histograms_.create(name, x_label, xbins, y_label, ybins);
    }
  }
}

DEFINE_FACTORY(EventProcessor);

Producer::Producer(const std::string &name, Process &process)
    : EventProcessor(name, process) {}

Analyzer::Analyzer(const std::string &name, Process &process)
    : EventProcessor(name, process) {}
}  // namespace framework
