#include "Framework/Performance/Tracker.h"

namespace framework::performance {

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

}
