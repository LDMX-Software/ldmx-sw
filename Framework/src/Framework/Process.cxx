/**
 * @file Process.cxx
 * Implementation file for Process class
 */

#include "Framework/Process.h"

#include <dlfcn.h>

#include <iostream>
#include <set>

#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Logger.h"
#include "Framework/NtupleManager.h"
#include "Framework/RunHeader.h"
#include "TFile.h"
#include "TROOT.h"

// Preemption flag that can be set by signal handlers
// NOLINTNEXTLINE(readability-identifier-naming)
volatile std::sig_atomic_t preemption_received_ = 0;

namespace framework {

Process::Process(const framework::config::Parameters &configuration)
    : conditions_{*this} {
  config_ = configuration;

  pass_name_ = configuration.get<std::string>("pass_name", "");
  histo_filename_ = configuration.get<std::string>("histogram_file", "");

  max_tries_ = configuration.get<int>("max_tries_per_event", 1);
  event_limit_ = configuration.get<int>("max_events", -1);
  min_events_ = configuration.get<int>("min_events", -1);
  total_events_ = configuration.get<int>("total_events", -1);
  log_frequency_ = configuration.get<int>("log_frequency", -1);
  compression_setting_ = configuration.get<int>("compression_setting", 9);
  skip_corrupted_input_files_ =
      configuration.get<bool>("skip_corrupted_input_files", false);

  input_files_ = configuration.get<std::vector<std::string>>("input_files", {});
  output_files_ =
      configuration.get<std::vector<std::string>>("output_files", {});
  drop_keep_rules_ = configuration.get<std::vector<std::string>>("keep", {});

  event_header_ = 0;

  // set up the logging for this run
  logging::open(configuration.get<framework::config::Parameters>("logger", {}));

  auto run{configuration.get<int>("run", -1)};
  if (run > 0) run_for_generation_ = run;

  auto libs{configuration.get<std::vector<std::string>>("libraries", {})};
  std::set<std::string> libraries_loaded;
  for (const auto &lib : libs) {
    if (libraries_loaded.find(lib) != libraries_loaded.end()) {
      continue;
    }

    void *handle = dlopen(lib.c_str(), RTLD_NOW);
    if (handle == nullptr) {
      EXCEPTION_RAISE("LibraryLoadFailure",
                      "Error loading library '" + lib + "':" + dlerror());
    }

    libraries_loaded.insert(lib);
  }

  storage_controller_.setDefaultKeep(
      configuration.get<bool>("skim_default_is_keep", true));
  auto skim_rules{configuration.get<std::vector<std::string>>("skim_rules", {})};
  for (size_t i = 0; i < skim_rules.size(); i += 2) {
    storage_controller_.addRule(skim_rules[i], skim_rules[i + 1]);
  }

  auto sequence{configuration.get<std::vector<framework::config::Parameters>>(
      "sequence", {})};
  if (sequence.empty() && configuration.get<bool>("testing_mode", false)) {
    EXCEPTION_RAISE(
        "NoSeq",
        "No sequence has been defined. What should I be doing?\nUse "
        "p.sequence to tell me what processors to run.");
  }
  for (auto proc : sequence) {
    auto class_name{proc.get<std::string>("class_name")};
    auto instance_name{proc.get<std::string>("instance_name")};
    auto ep{
        EventProcessor::Factory::get().make(class_name, instance_name, *this)};
    if (not ep) {
      EXCEPTION_RAISE("UnableToCreate",
                      "The EventProcessor Factory was unable to create " +
                          instance_name + " of type " + class_name +
                          ". Did you inherit from framework::Producer or "
                          "framework::Analyzer? "
                          "Did you DECLARE_PRODUCER or DECLARE_ANALYZER in the "
                          "implementation (.cxx) file? "
                          "Did you use the class's full name (including "
                          "namespaces) in the Python configuration class? "
                          "Does the Python configuration class reference the "
                          "correct library it is a part of?");
    }
    auto histograms{
        proc.get<std::vector<framework::config::Parameters>>("histograms", {})};
    if (!histograms.empty()) {
      ep.value()->getHistoDirectory();
      ep.value()->createHistograms(histograms);
    }
    ep.value()->configure(proc);
    sequence_.push_back(ep.value());
  }

  auto conditions_object_providers{
      configuration.get<std::vector<framework::config::Parameters>>(
          "conditions_object_providers", {})};
  for (auto cop : conditions_object_providers) {
    auto class_name{cop.get<std::string>("class_name")};
    auto object_name{cop.get<std::string>("object_name")};
    auto tag_name{cop.get<std::string>("tag_name")};
    conditions_.createConditionsObjectProvider(class_name, object_name,
                                               tag_name, cop);
  }

  bool log_performance = configuration.get<bool>("log_performance", false);
  if (log_performance) {
    std::vector<std::string> names{sequence_.size()};
    for (std::size_t i{0}; i < sequence_.size(); i++) {
      names[i] = sequence_[i]->getName();
    }
    performance_ =
        new performance::Tracker(makeHistoDirectory("performance"), names);
  }
}

Process::~Process() {
  // need to delete the performance object so that it is
  // written before we close the histogram file below
  if (performance_) delete performance_;
  for (EventProcessor *ep : sequence_) {
    delete ep;
  }
  if (histo_t_file_) {
    histo_t_file_->Write();
    delete histo_t_file_;
    histo_t_file_ = 0;
  }
}

void Process::run() {
  if (performance_) performance_->absoluteStart();

  // Counter to keep track of the number of events that have been
  // procesed
  auto n_events_processed{0};

  // make sure the ntuple manager is in a blank state
  NtupleManager::getInstance().reset();

  // event bus for this process
  Event the_event(pass_name_);
  // the EventHeader object is created with the event bus as
  // one of its members, we obtain a pointer for the header
  // here so we can share it with the conditions system
  event_header_ = the_event.getEventHeaderPtr();
  the_event.getEventHeader().setRun(run_for_generation_);

  // Start by notifying everyone that modules processing is beginning
  std::size_t i_proc{0};
  if (performance_)
    performance_->start(performance::Callback::onProcessStart, 0);
  conditions_.onProcessStart();
  for (auto proc : sequence_) {
    i_proc++;
    if (performance_)
      performance_->start(performance::Callback::onProcessStart, i_proc);
    proc->onProcessStart();
    if (performance_)
      performance_->stop(performance::Callback::onProcessStart, i_proc);
  }
  if (performance_)
    performance_->stop(performance::Callback::onProcessStart, 0);

  // If we have no input files, but do have an event number, run for
  // that number of events and generate an output file.
  if (input_files_.empty() && event_limit_ > 0) {
    if (output_files_.empty()) {
      EXCEPTION_RAISE("InvalidConfig",
                      "No input files or output files were given.");
    } else if (output_files_.size() > 1) {
      ldmx_log(warn) << "Several output files given with no input files. "
                     << "Only the first output file '" << output_files_.at(0)
                     << "' will be used.";
    }
    std::string output_file_name = output_files_.at(0);

    // Configure the event file to create an output file with no parent. This
    // requires setting the parameters isOutputFile and isSingleOutput to true.
    EventFile out_file(config_, output_file_name, nullptr, true, true, false);
    onFileOpen(out_file);
    out_file.setupEvent(&the_event);

    for (auto rule : drop_keep_rules_) out_file.addDrop(rule);

    ldmx::RunHeader run_header(run_for_generation_);
    run_header.setRunStart(std::time(nullptr));  // set run starting
    run_header_ = &run_header;  // give handle to run header to process
    out_file.writeRunHeader(run_header);  // add run header to file

    newRun(run_header);

    int total_tries = 0;  // total number of tries for entire run
    int num_tries = 0;    // number of tries for the current event number
    int event_limit = event_limit_;
    if (total_events_ > 0) {
      // Have a warning at the first event
      if (num_tries == 0)
        ldmx_log(warn) << "The totalEvents was set, so maxEvents and "
                          "maxTriesPerEvent will be ignored!";
      event_limit = total_events_;
    }
    while (n_events_processed < event_limit) {
      // Check for preemption before processing each event
      if (preemption_received_) {
        ldmx_log(fatal)
            << "Preemption signal received, stopping event generation";
        break;
      }

      total_tries++;
      num_tries++;

      ldmx::EventHeader &eh = the_event.getEventHeader();
      eh.setRun(run_for_generation_);
      eh.setEventNumber(n_events_processed + 1);
      eh.setTimestamp(TTimeStamp());

      // reset the storage controller state
      storage_controller_.resetEventState();
      logging::Formatter::set(the_event.getEventNumber());

      bool completed = process(n_events_processed, num_tries, the_event);

      out_file.nextEvent(storage_controller_.keepEvent(completed));

      // reset try counter only on successfully completed events
      if (completed) num_tries = 0;

      // we use modulo here insetad of >= because we want to carry
      // the number of tries across the number of events processed boundary
      // total_events_ is set let's not exit until that's reached
      if (completed or (total_events_ < 0 and num_tries % max_tries_ == 0)) {
        n_events_processed++;                 // increment events made
        NtupleManager::getInstance().fill();  // fill ntuples
      }

      NtupleManager::getInstance().clear();
    }

    onFileClose(out_file);

    run_header.setRunEnd(std::time(nullptr));
    run_header.setNumTries(total_tries);
    out_file.writeRunTree();

    // Give a warning that this filter has very low efficiency
    if (n_events_processed < total_tries / 10000) {  // integer division is okay
      ldmx_log(warn)
          << "Less than 1 event out of every 10k events tried was accepted!";
      ldmx_log(warn)
          << "This could be an issue with your filtering and biasing procedure "
             "since this is incredibly inefficient.";
    }

  } else {
    // there are input files

    EventFile *out_file(0);

    bool single_output = false;
    if (output_files_.size() == 1) {
      single_output = true;
    } else if (!output_files_.empty() and
               output_files_.size() != input_files_.size()) {
      EXCEPTION_RAISE("Process",
                      "Unable to handle case of different number of input and "
                      "output files (other than zero/one ouput file).");
    }

    // next, loop through the files
    int ifile = 0;
    int was_run = -1;
    for (auto infilename : input_files_) {
      EventFile in_file(config_, infilename);
      if (in_file.isCorrupted()) {
        if (skip_corrupted_input_files_) {
          ldmx_log(warn) << "Input file '" << infilename
                         << "' was found to be corrupted. Skipping.";
          continue;
        } else {
          EXCEPTION_RAISE(
              "BadCode",
              "We should never get here. "
              "EventFile is corrupted but we aren't skipping corrupted inputs. "
              "EventFile should be throwing its own exceptions in this case.");
        }
      }

      ldmx_log(info) << "Opening file " << infilename;
      onFileOpen(in_file);

      // configure event file that will be iterated over
      EventFile *master_file;
      if (!output_files_.empty()) {
        // setup new output file if either
        // 1) we are not in single output mode
        // 2) this is the first input file
        if (!single_output or ifile == 0) {
          // setup new output file
          out_file = new EventFile(config_, output_files_[ifile], &in_file,
                                   single_output);
          ifile++;

          // setup theEvent we will iterate over
          if (out_file) {
            out_file->setupEvent(&the_event);
            master_file = out_file;
          } else {
            EXCEPTION_RAISE("Process", "Unable to construct output file for " +
                                           output_files_[ifile]);
          }

          for (auto rule : drop_keep_rules_) out_file->addDrop(rule);

        } else {
          // all other input files
          out_file->updateParent(&in_file);
          master_file = out_file;

        }  // check if in singleOutput mode

      } else {
        // empty output file list, use inputFile as master file
        in_file.setupEvent(&the_event);
        master_file = &in_file;
      }

      // In case we'd like to skip up to the event of min_events_
      while (n_events_processed < (min_events_ - 1) &&
             master_file->nextEvent(false)) {
        n_events_processed++;
      }

      bool event_completed = true;
      while (!preemption_received_ &&
             master_file->nextEvent(
                 storage_controller_.keepEvent(event_completed)) &&
             ((event_limit_ < 0) || (n_events_processed < event_limit_))) {
        // clean up for storage control calculation
        storage_controller_.resetEventState();
        logging::Formatter::set(the_event.getEventNumber());

        // notify for new run if necessary
        if (the_event.getEventHeader().getRun() != was_run) {
          was_run = the_event.getEventHeader().getRun();
          ldmx::RunHeader *rh{master_file->getRunHeaderPtr(was_run)};
          if (rh != nullptr) {
            run_header_ = rh;
            ldmx_log(info) << "Got new run header from '"
                           << master_file->getFileName() << "'";
            newRun(*run_header_);
          } else {
            ldmx_log(warn) << "Run header for run " << was_run
                           << " was not found!";
          }
        }

        event_completed = process(n_events_processed, 1, the_event);

        if (event_completed) NtupleManager::getInstance().fill();
        NtupleManager::getInstance().clear();

        n_events_processed++;
      }  // loop through events

      if (preemption_received_) {
        ldmx_log(fatal) << "Preemption signal received, stopping event "
                           "processing and closing files";
      }

      bool leave_early{false};
      if (event_limit_ > 0 && n_events_processed == event_limit_) {
        ldmx_log(info) << "Reached event limit of " << event_limit_
                       << " events";
        leave_early = true;
      }

      if (event_limit_ == 0 && n_events_processed > event_limit_) {
        ldmx_log(warn) << "Processing interrupted";
        leave_early = true;
      }

      ldmx_log(info) << "Closing file " << infilename;
      onFileClose(in_file);

      // Reset the event in case of multiple input files
      the_event.onEndOfFile();

      if (out_file and !single_output) {
        out_file->writeRunTree();
        delete out_file;
        out_file = nullptr;
      }

      if (leave_early) {
        break;
      }
    }  // loop through input files

    if (out_file) {
      // close outFile
      //  outFile would survive to here in single output mode
      out_file->writeRunTree();
      delete out_file;
      out_file = nullptr;
    }

  }  // are there input files? if-else tree

  // finally, notify everyone that we are stopping
  if (performance_) performance_->start(performance::Callback::onProcessEnd, 0);
  i_proc = 0;
  for (auto proc : sequence_) {
    i_proc++;
    if (performance_)
      performance_->start(performance::Callback::onProcessEnd, i_proc);
    proc->onProcessEnd();
    if (performance_)
      performance_->stop(performance::Callback::onProcessEnd, i_proc);
  }
  if (performance_) performance_->stop(performance::Callback::onProcessEnd, 0);

  // we're done so let's close up the logging
  logging::close();
  if (performance_) performance_->absoluteStop();
}

int Process::getRunNumber() const {
  return (event_header_) ? (event_header_->getRun()) : (run_for_generation_);
}

TDirectory *Process::makeHistoDirectory(const std::string &dirName) {
  auto owner{openHistoFile()};
  TDirectory *child = owner->mkdir((char *)dirName.c_str());
  if (child) child->cd();
  return child;
}

TDirectory *Process::openHistoFile() {
  TDirectory *owner{nullptr};

  if (histo_filename_.empty()) {
    // trying to write histograms/ntuples but no file defined
    EXCEPTION_RAISE(
        "NoHistFileName",
        "You did not provide the necessary histogram file name to "
        "put your histograms (or performance data) in.\n    Provide this "
        "name in the python configuration with 'p.histogramFile = "
        "\"myHistFile.root\"' where p is the Process object.");
  } else if (histo_t_file_ == nullptr) {
    histo_t_file_ = new TFile(histo_filename_.c_str(), "RECREATE");
    owner = histo_t_file_;
  } else {
    owner = histo_t_file_;
  }
  owner->cd();

  return owner;
}

void Process::newRun(ldmx::RunHeader &header) {
  // Producers are allowed to put parameters into
  // the run header through 'beforeNewRun' method

  // Put the version into the rh string param
  header.setStringParameter("Pass = " + pass_name_ + ", version",
                            LDMXSW_VERSION);
  if (performance_) performance_->start(performance::Callback::beforeNewRun, 0);
  std::size_t i_proc{0};
  for (auto proc : sequence_) {
    i_proc++;
    if (performance_)
      performance_->start(performance::Callback::beforeNewRun, i_proc);
    proc->beforeNewRun(header);
    if (performance_)
      performance_->stop(performance::Callback::beforeNewRun, i_proc);
  }
  if (performance_) performance_->stop(performance::Callback::beforeNewRun, 0);
  // now run header has been modified by Producers,
  // it is valid to read from for everyone else in 'onNewRun'
  if (performance_) performance_->start(performance::Callback::onNewRun, 0);
  conditions_.onNewRun(header);
  i_proc = 0;
  for (auto proc : sequence_) {
    i_proc++;
    if (performance_)
      performance_->start(performance::Callback::onNewRun, i_proc);
    proc->onNewRun(header);
    if (performance_)
      performance_->stop(performance::Callback::onNewRun, i_proc);
  }
  if (performance_) performance_->stop(performance::Callback::onNewRun, 0);
  ldmx_log(info) << header;
}

bool Process::process(int n, int n_try, Event &event) const {
  if ((log_frequency_ != -1) && ((n + 1) % log_frequency_ == 0) &&
      (n_try < 2)) {
    // only printout event counter if we've enabled log frequency, the event
    // matches the frequency and we are on the first try
    TTimeStamp t;
    ldmx_log(info) << "Processing " << n + 1 << " Run "
                   << event.getEventHeader().getRun() << " Event "
                   << event.getEventHeader().getEventNumber() << "  ("
                   << t.AsString("lc") << ")";
  }

  if (performance_) performance_->start(performance::Callback::process, 0);
  std::size_t i_proc{0};
  try {
    for (auto proc : sequence_) {
      i_proc++;
      if (performance_)
        performance_->start(performance::Callback::process, i_proc);
      proc->process(event);
      if (performance_)
        performance_->stop(performance::Callback::process, i_proc);
    }
  } catch (AbortEventException &) {
    if (performance_) {
      performance_->stop(performance::Callback::process, i_proc);
      performance_->stop(performance::Callback::process, 0);
      performance_->endEvent(false);
    }
    return false;
  }
  if (performance_) {
    performance_->stop(performance::Callback::process, 0);
    performance_->endEvent(true);
  }
  return true;
}

void Process::onFileOpen(EventFile &file) const {
  if (performance_) performance_->start(performance::Callback::onFileOpen, 0);
  std::size_t i_proc{0};
  for (auto proc : sequence_) {
    i_proc++;
    if (performance_)
      performance_->start(performance::Callback::onFileOpen, i_proc);
    proc->onFileOpen(file);
    if (performance_)
      performance_->stop(performance::Callback::onFileOpen, i_proc);
  }
  if (performance_) performance_->stop(performance::Callback::onFileOpen, 0);
}

void Process::onFileClose(EventFile &file) const {
  if (performance_) performance_->start(performance::Callback::onFileClose, 0);
  std::size_t i_proc{0};
  for (auto proc : sequence_) {
    i_proc++;
    if (performance_)
      performance_->start(performance::Callback::onFileClose, i_proc);
    proc->onFileClose(file);
    if (performance_)
      performance_->stop(performance::Callback::onFileClose, i_proc);
  }
  if (performance_) performance_->stop(performance::Callback::onFileClose, 0);
}

}  // namespace framework
