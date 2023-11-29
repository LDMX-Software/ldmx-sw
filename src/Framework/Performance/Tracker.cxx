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
  storage_directory_->WriteObject(&absolute_start_, "absolute_start");
  storage_directory_->WriteObject(&absolute_end_, "absolute_end");

  TDirectory* onProcessStart_d = storage_directory_->mkdir("onProcessStart");
  TDirectory* begin = onProcessStart_d->mkdir("begin");
  for (const auto& [name, meas] : begin_onProcessStart_) {
    begin->WriteObject(&meas, name.c_str());
  }
  TDirectory* end = onProcessStart_d->mkdir("end");
  for (const auto& [name, meas] : end_onProcessStart_) {
    end->WriteObject(&meas, name.c_str());
  }

  TDirectory* onProcessEnd_d = storage_directory_->mkdir("onProcessEnd");
  begin = onProcessEnd_d->mkdir("begin");
  for (const auto& [name, meas] : begin_onProcessEnd_) {
    begin->WriteObject(&meas, name.c_str());
  }
  end = onProcessEnd_d->mkdir("end");
  for (const auto& [name, meas] : end_onProcessEnd_) {
    end->WriteObject(&meas, name.c_str());
  }

  TDirectory* onFileOpen_d = storage_directory_->mkdir("onFileOpen");
  begin = onFileOpen_d->mkdir("begin");
  for (const auto& [name, meas] : begin_onFileOpen_) {
    begin->WriteObject(&meas, name.c_str());
  }
  end = onFileOpen_d->mkdir("end");
  for (const auto& [name, meas] : end_onFileOpen_) {
    end->WriteObject(&meas, name.c_str());
  }

  TDirectory* onFileClose_d = storage_directory_->mkdir("onFileClose");
  begin = onFileClose_d->mkdir("begin");
  for (const auto& [name, meas] : begin_onFileClose_) {
    begin->WriteObject(&meas, name.c_str());
  }
  end = onFileClose_d->mkdir("end");
  for (const auto& [name, meas] : end_onFileClose_) {
    end->WriteObject(&meas, name.c_str());
  }

  TDirectory* beforeNewRun_d = storage_directory_->mkdir("beforeNewRun");
  begin = beforeNewRun_d->mkdir("begin");
  for (const auto& [name, meas] : begin_beforeNewRun_) {
    begin->WriteObject(&meas, name.c_str());
  }
  end = beforeNewRun_d->mkdir("end");
  for (const auto& [name, meas] : end_beforeNewRun_) {
    end->WriteObject(&meas, name.c_str());
  }

  TDirectory* onNewRun_d = storage_directory_->mkdir("onNewRun");
  begin = onNewRun_d->mkdir("begin");
  for (const auto& [name, meas] : begin_onNewRun_) {
    begin->WriteObject(&meas, name.c_str());
  }
  end = onNewRun_d->mkdir("end");
  for (const auto& [name, meas] : end_onNewRun_) {
    end->WriteObject(&meas, name.c_str());
  }

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
  auto meas_it{begin_process_.find(processor)};
  if (meas_it == begin_process_.end()) {
    auto meas = new Measurement(false);
    event_data_->Branch(("begin_"+processor).c_str(), meas);
    auto in = begin_process_.emplace(processor, meas);
    meas_it = in.first;
  }
  meas_it->second->sample();
}

void Tracker::end_process(const std::string& processor) {
  auto meas_it{end_process_.find(processor)};
  if (meas_it == end_process_.end()) {
    auto meas = new Measurement(false);
    event_data_->Branch(("end_"+processor).c_str(), meas);
    auto in = end_process_.emplace(processor, meas);
    meas_it = in.first;
  }
  meas_it->second->sample();
}

void Tracker::end_event() {
  event_data_->Fill();
}

}
