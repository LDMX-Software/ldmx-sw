#include "Framework/Performance/Tracker.h"

namespace framework::performance {

const std::string Tracker::ALL = "__ALL__";

Tracker::Tracker(TDirectory *storage_directory, const std::vector<std::string>& names)
  : storage_directory_{storage_directory} {
  storage_directory_->cd();
  event_data_ = new TTree("by_event","by_event");

  names_.reserve(names.size()+1);
  names_.push_back(Tracker::ALL);
  for (const std::string& name : names) {
    names_.push_back(name);
  }

  event_data_->Branch("completed", &event_completed_);
  event_times_.resize(names_.size());
  for (std::size_t i{0}; i < names_.size(); i++) {
    event_data_->Branch(names_[i].c_str(), &(event_times_[i]));
  }

  processor_timers_.resize(7);
  for (std::vector<Timer>& timer_set : processor_timers_) {
    timer_set.resize(names_.size());
  }
}

Tracker::~Tracker() {
  storage_directory_->cd();
  //event_data_->Write();
  absolute_.write(storage_directory_, "absolute");

  std::vector<Callback> non_event_callbacks = {
    Callback::onProcessStart, Callback::onProcessEnd,
    Callback::onFileOpen, Callback::onFileClose,
    Callback::beforeNewRun, Callback::onNewRun
  };
  for (auto& callback : non_event_callbacks) {
    TDirectory* callback_d = storage_directory_->mkdir(to_name(callback));
    for (std::size_t i_proc{0}; i_proc < names_.size(); i_proc++) {
      processor_timers_[to_index(callback)][i_proc].write(callback_d, names_[i_proc]);
    }
  }
}

void Tracker::absolute_start() {
  absolute_.start();
}

void Tracker::absolute_end() {
  absolute_.end();
}

void Tracker::start(Callback callback, std::size_t i_proc) {
  processor_timers_[to_index(callback)][i_proc].start();
}

void Tracker::end(Callback callback, std::size_t i_proc) {
  processor_timers_[to_index(callback)][i_proc].end();
}

void Tracker::end_event(bool completed) {
  event_completed_ = completed;
  for (std::size_t i_proc{0}; i_proc < names_.size(); i_proc++) {
    event_times_[i_proc] = processor_timers_[to_index(Callback::process)][i_proc].duration();
  }
  event_data_->Fill();
}

}
