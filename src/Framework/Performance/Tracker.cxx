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
  storage_directory_->WriteObject(&absolute_start_, "absolute_start");
  storage_directory_->WriteObject(&absolute_end_, "absolute_end");
  storage_directory_->WriteObject(&begin_onProcessStart_, "begin_onProcessStart");
  storage_directory_->WriteObject(&end_onProcessStart_, "end_onProcessStart");
  storage_directory_->WriteObject(&begin_onProcessEnd_, "begin_onProcessEnd");
  storage_directory_->WriteObject(&end_onProcessEnd_, "end_onProcessEnd");
  storage_directory_->WriteObject(&begin_onFileOpen_, "begin_onFileOpen");
  storage_directory_->WriteObject(&end_onFileOpen_, "end_onFileOpen");
  storage_directory_->WriteObject(&begin_onFileClose_, "begin_onFileClose");
  storage_directory_->WriteObject(&end_onFileClose_, "end_onFileClose");
  storage_directory_->WriteObject(&begin_beforeNewRun_, "begin_beforeNewRun");
  storage_directory_->WriteObject(&end_beforeNewRun_, "end_beforeNewRun");
  storage_directory_->WriteObject(&begin_onNewRun_, "begin_onNewRun");
  storage_directory_->WriteObject(&end_onNewRun_, "end_onNewRun");
  event_data_->Write();
  for (auto& [name, m] : begin_process_) {
    delete m;
  }
  for (auto& [name, m] : end_process_) {
    delete m;
  }
}

void Tracker::absolute_start() {
  absolute_start_.sample();
}

void Tracker::absolute_end() {
  absolute_end_.sample();
}

void Tracker::begin_onProcessStart(const std::string& processor) {
  begin_onProcessStart_.emplace(processor, true);
}

void Tracker::end_onProcessStart(const std::string& processor) {
  end_onProcessStart_.emplace(processor, true);
}

void Tracker::begin_onProcessEnd(const std::string& processor) {
  begin_onProcessEnd_.emplace(processor, true);
}

void Tracker::end_onProcessEnd(const std::string& processor) {
  end_onProcessEnd_.emplace(processor, true);
}

void Tracker::begin_onFileOpen(const std::string& processor) {
  begin_onFileOpen_.emplace(processor, true);
}

void Tracker::end_onFileOpen(const std::string& processor) {
  end_onFileOpen_.emplace(processor, true);
}

void Tracker::begin_onFileClose(const std::string& processor) {
  begin_onFileClose_.emplace(processor, true);
}

void Tracker::end_onFileClose(const std::string& processor) {
  end_onFileClose_.emplace(processor, true);
}

void Tracker::begin_beforeNewRun(const std::string& processor) {
  begin_beforeNewRun_.emplace(processor, true);
}

void Tracker::end_beforeNewRun(const std::string& processor) {
  end_beforeNewRun_.emplace(processor, true);
}

void Tracker::begin_onNewRun(const std::string& processor) {
  begin_onNewRun_.emplace(processor, true);
}

void Tracker::end_onNewRun(const std::string& processor) {
  end_onNewRun_.emplace(processor, true);
}

void Tracker::begin_process(const std::string& processor) {
  if (begin_process_.find(processor) == begin_process_.end()) {
    auto meas = new Measurement(false);
    event_data_->Branch(("begin_"+processor).c_str(), meas);
    begin_process_.emplace(processor, meas);
  }
  begin_process_[processor]->sample();
}

void Tracker::end_process(const std::string& processor) {
  if (end_process_.find(processor) == end_process_.end()) {
    auto meas = new Measurement(false);
    event_data_->Branch(("end_"+processor).c_str(), &meas);
    end_process_.emplace(processor, meas);
  }
  end_process_[processor]->sample();
  if (processor == ALL) {
    event_data_->Fill();
  }
}

}
