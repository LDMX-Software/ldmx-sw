#include "Framework/Performance/Tracker.h"

namespace framework::performance {

const std::string Tracker::ALL = "__ALL__";

Tracker::Tracker(TDirectory *storage_directory)
  : storage_directory_{storage_directory} {
  storage_directory_->cd();
  event_data_ = new TTree("by_event","by_event");
}

Tracker::~Tracker() {
  storage_directory_->cd();
  event_data_->Write();
  storage_directory_->WriteObject(&absolute_, "absolute");

  std::vector<std::pair<std::string,std::map<std::string,Measurement>>> callbacks = {
    { "onProcessStart", onProcessStart_ },
    { "onProcessEnd", onProcessEnd_ },
    { "onFileOpen", onFileOpen_ },
    { "onFileClose", onFileClose_ },
    { "beforeNewRun", beforeNewRun_ },
    { "onNewRun", onNewRun_ }
  };
  for (auto& [name, measurements] : callbacks) {
    TDirectory* callback_d = storage_directory_->mkdir(name.c_str());
    for (const auto& [procname, measurement] : measurements) {
      callback_d->WriteObject(&measurement, procname.c_str());
    }
  }

  for (auto& [name, m] : process_) {
    delete m;
  }
}

void Tracker::absolute_start() {
  absolute_.start();
}

void Tracker::absolute_end() {
  absolute_.end();
}

void Tracker::begin_onProcessStart(const std::string& processor) {
  onProcessStart_.emplace(processor, true);
}

void Tracker::end_onProcessStart(const std::string& processor) {
  onProcessStart_[processor].end();
}

void Tracker::begin_onProcessEnd(const std::string& processor) {
  onProcessEnd_.emplace(processor, true);
}

void Tracker::end_onProcessEnd(const std::string& processor) {
  onProcessEnd_[processor].end();
}

void Tracker::begin_onFileOpen(const std::string& processor) {
  onFileOpen_.emplace(processor, true);
}

void Tracker::end_onFileOpen(const std::string& processor) {
  onFileOpen_[processor].end();
}

void Tracker::begin_onFileClose(const std::string& processor) {
  onFileClose_.emplace(processor, true);
}

void Tracker::end_onFileClose(const std::string& processor) {
  onFileClose_[processor].end();
}

void Tracker::begin_beforeNewRun(const std::string& processor) {
  beforeNewRun_.emplace(processor, true);
}

void Tracker::end_beforeNewRun(const std::string& processor) {
  beforeNewRun_[processor].end();
}

void Tracker::begin_onNewRun(const std::string& processor) {
  onNewRun_.emplace(processor, true);
}

void Tracker::end_onNewRun(const std::string& processor) {
  onNewRun_[processor].end();
}

void Tracker::begin_process(const std::string& processor) {
  auto meas_it{process_.find(processor)};
  if (meas_it == process_.end()) {
    auto meas = new Measurement(false);
    event_data_->Branch(("begin_"+processor).c_str(), meas);
    auto in = process_.emplace(processor, meas);
    meas_it = in.first;
  }
  meas_it->second->start();
}

void Tracker::end_process(const std::string& processor) {
  /**
   * We assume the callers of this tracker correctly call begin_*
   * before any end_* calls.
   */
  process_[processor]->end();
}

void Tracker::end_event() {
  event_data_->Fill();
  for (auto& [name, meas] : process_) meas->invalidate();
}

}
