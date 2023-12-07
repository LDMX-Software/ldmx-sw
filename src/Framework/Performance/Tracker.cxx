#include "Framework/Performance/Tracker.h"

namespace framework::performance {

const std::string Tracker::ALL = "__ALL__";

Tracker::Tracker(TDirectory *storage_directory, const std::vector<std::string>& names)
  : storage_directory_{storage_directory} {

  /**
   * Create the event-by-event data TTree while within
   * the storage directory. This means the event data TTree
   * will be connected to this file automatically and will
   * be written there and full synchronized when that file
   * is closed
   */
  storage_directory_->cd();
  event_data_ = new TTree("by_event","by_event");

  /**
   * Copy the processor names passed to us
   * and include an extra name at the beginning
   * representing a timer encompasing all of the processors
   * in the sequence
   */
  names_.reserve(names.size()+1);
  names_.push_back(Tracker::ALL);
  for (const std::string& name : names) {
    names_.push_back(name);
  }

  /**
   * Allocate timers for each of the callbacks and each of the processors
   */
  processor_timers_.resize(7); // 7 different callbacks
  for (std::vector<Timer>& timer_set : processor_timers_) {
    timer_set.resize(names_.size());
  }

  /**
   * Attach the processor timers to the event-by-event data
   * TTree as branches
   *
   * @note This is where we connect the _address_ of our timer
   * to the TTree for serialization. This effectively means that
   * the vector of processor_timers_ should not be moved or changed
   * in size to avoid breaking this I/O connection.
   */
  event_data_->Branch("completed", &event_completed_);
  for (std::size_t i{0}; i < names_.size(); i++) {
    event_data_->Branch(
        names_[i].c_str(), 
        &(processor_timers_[to_index(Callback::process)][i])
    );
  }
}

Tracker::~Tracker() {
  storage_directory_->cd();
  absolute_.write(storage_directory_, "absolute");

  /**
   * Write the non-event callbacks in their own directories,
   * this data is then accessible as single data-points instead
   * of a TTree of entires
   */
  std::vector<Callback> non_event_callbacks = {
    Callback::onProcessStart, Callback::onProcessEnd,
    Callback::onFileOpen, Callback::onFileClose,
    Callback::beforeNewRun, Callback::onNewRun
  };
  for (auto& callback : non_event_callbacks) {
    TDirectory* callback_d = storage_directory_->mkdir(to_name(callback).c_str());
    for (std::size_t i_proc{0}; i_proc < names_.size(); i_proc++) {
      processor_timers_[to_index(callback)][i_proc].write(callback_d, names_[i_proc]);
    }
  }
}

void Tracker::absolute_start() {
  absolute_.start();
}

void Tracker::absolute_stop() {
  absolute_.stop();
}

void Tracker::start(Callback callback, std::size_t i_proc) {
  processor_timers_[to_index(callback)][i_proc].start();
}

void Tracker::stop(Callback callback, std::size_t i_proc) {
  processor_timers_[to_index(callback)][i_proc].stop();
}

void Tracker::end_event(bool completed) {
  event_completed_ = completed;
  event_data_->Fill();
  /**
   * Make sure to reset the timer _after_ the event data has been filled
   * so that if a future event is not completed (and some timers are not
   * started or ended), that can be reflected in the serialized data.
   */
  for (std::size_t i_proc{0}; i_proc < names_.size(); i_proc++) {
    processor_timers_[to_index(Callback::process)][i_proc].reset();
  }
}

}
