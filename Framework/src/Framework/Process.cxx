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

namespace framework {

Process::Process(const framework::config::Parameters &configuration)
    : conditions_{*this} {
  config_ = configuration;

  pass_name_ = configuration.getParameter<std::string>("passName", "");
  histoFilename_ = configuration.getParameter<std::string>("histogramFile", "");

  maxTries_ = configuration.getParameter<int>("maxTriesPerEvent", 1);
  eventLimit_ = configuration.getParameter<int>("maxEvents", -1);
  minEvents_ = configuration.getParameter<int>("minEvents", -1);
  totalEvents_ = configuration.getParameter<int>("totalEvents", -1);
  logFrequency_ = configuration.getParameter<int>("logFrequency", -1);
  compressionSetting_ =
      configuration.getParameter<int>("compressionSetting", 9);
  skipCorruptedInputFiles_ =
      configuration.getParameter<bool>("skipCorruptedInputFiles", false);

  inputFiles_ =
      configuration.getParameter<std::vector<std::string>>("inputFiles", {});
  outputFiles_ =
      configuration.getParameter<std::vector<std::string>>("outputFiles", {});
  dropKeepRules_ =
      configuration.getParameter<std::vector<std::string>>("keep", {});

  eventHeader_ = 0;

  // set up the logging for this run
  logging::open(
      configuration.getParameter<framework::config::Parameters>("logger", {}));

  auto run{configuration.getParameter<int>("run", -1)};
  if (run > 0) runForGeneration_ = run;

  auto libs{
      configuration.getParameter<std::vector<std::string>>("libraries", {})};
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

  storageController_.setDefaultKeep(
      configuration.getParameter<bool>("skimDefaultIsKeep", true));
  auto skim_rules{
      configuration.getParameter<std::vector<std::string>>("skimRules", {})};
  for (size_t i = 0; i < skim_rules.size(); i += 2) {
    storageController_.addRule(skim_rules[i], skim_rules[i + 1]);
  }

  auto sequence{
      configuration.getParameter<std::vector<framework::config::Parameters>>(
          "sequence", {})};
  if (sequence.empty() &&
      configuration.getParameter<bool>("testingMode", false)) {
    EXCEPTION_RAISE(
        "NoSeq",
        "No sequence has been defined. What should I be doing?\nUse "
        "p.sequence to tell me what processors to run.");
  }
  for (auto proc : sequence) {
    auto class_name{proc.getParameter<std::string>("className")};
    auto instance_name{proc.getParameter<std::string>("instanceName")};
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
        proc.getParameter<std::vector<framework::config::Parameters>>(
            "histograms", {})};
    if (!histograms.empty()) {
      ep.value()->getHistoDirectory();
      ep.value()->createHistograms(histograms);
    }
    ep.value()->configure(proc);
    sequence_.push_back(ep.value());
  }

  auto conditions_object_providers{
      configuration.getParameter<std::vector<framework::config::Parameters>>(
          "conditionsObjectProviders", {})};
  for (auto cop : conditions_object_providers) {
    auto class_name{cop.getParameter<std::string>("className")};
    auto object_name{cop.getParameter<std::string>("objectName")};
    auto tag_name{cop.getParameter<std::string>("tagName")};
    conditions_.createConditionsObjectProvider(class_name, object_name, tag_name,
                                               cop);
  }

  bool log_performance =
      configuration.getParameter<bool>("logPerformance", false);
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
  if (histoTFile_) {
    histoTFile_->Write();
    delete histoTFile_;
    histoTFile_ = 0;
  }
}

void Process::run() {
  if (performance_) performance_->absolute_start();

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
  eventHeader_ = the_event.getEventHeaderPtr();
  the_event.getEventHeader().setRun(runForGeneration_);

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
  if (inputFiles_.empty() && eventLimit_ > 0) {
    if (outputFiles_.empty()) {
      EXCEPTION_RAISE("InvalidConfig",
                      "No input files or output files were given.");
    } else if (outputFiles_.size() > 1) {
      ldmx_log(warn) << "Several output files given with no input files. "
                     << "Only the first output file '" << outputFiles_.at(0)
                     << "' will be used.";
    }
    std::string output_file_name = outputFiles_.at(0);

    // Configure the event file to create an output file with no parent. This
    // requires setting the parameters isOutputFile and isSingleOutput to true.
    EventFile out_file(config_, output_file_name, nullptr, true, true, false);
    onFileOpen(out_file);
    out_file.setupEvent(&the_event);

    for (auto rule : dropKeepRules_) out_file.addDrop(rule);

    ldmx::RunHeader run_header(runForGeneration_);
    run_header.setRunStart(std::time(nullptr));  // set run starting
    runHeader_ = &run_header;            // give handle to run header to process
    out_file.writeRunHeader(run_header);  // add run header to file

    newRun(run_header);

    int total_tries = 0;  // total number of tries for entire run
    int num_tries = 0;    // number of tries for the current event number
    int event_limit = eventLimit_;
    if (totalEvents_ > 0) {
      // Have a warning at the first event
      if (num_tries == 0)
        ldmx_log(warn) << "The totalEvents was set, so maxEvents and "
                          "maxTriesPerEvent will be ignored!";
      event_limit = totalEvents_;
    }
    while (n_events_processed < event_limit) {
      total_tries++;
      num_tries++;

      ldmx::EventHeader &eh = the_event.getEventHeader();
      eh.setRun(runForGeneration_);
      eh.setEventNumber(n_events_processed + 1);
      eh.setTimestamp(TTimeStamp());

      // reset the storage controller state
      storageController_.resetEventState();
      logging::Formatter::set(the_event.getEventNumber());

      bool completed = process(n_events_processed, num_tries, the_event);

      out_file.nextEvent(storageController_.keepEvent(completed));

      // reset try counter only on successfully completed events
      if (completed) num_tries = 0;

      // we use modulo here insetad of >= because we want to carry
      // the number of tries across the number of events processed boundary
      // totalEvents_ is set let's not exit until that's reached
      if (completed or (totalEvents_ < 0 and num_tries % maxTries_ == 0)) {
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
    if (outputFiles_.size() == 1) {
      single_output = true;
    } else if (!outputFiles_.empty() and
               outputFiles_.size() != inputFiles_.size()) {
      EXCEPTION_RAISE("Process",
                      "Unable to handle case of different number of input and "
                      "output files (other than zero/one ouput file).");
    }

    // next, loop through the files
    int ifile = 0;
    int was_run = -1;
    for (auto infilename : inputFiles_) {
      EventFile in_file(config_, infilename);
      if (in_file.isCorrupted()) {
        if (skipCorruptedInputFiles_) {
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
      if (!outputFiles_.empty()) {
        // setup new output file if either
        // 1) we are not in single output mode
        // 2) this is the first input file
        if (!single_output or ifile == 0) {
          // setup new output file
          out_file = new EventFile(config_, outputFiles_[ifile], &in_file,
                                  single_output);
          ifile++;

          // setup theEvent we will iterate over
          if (out_file) {
            out_file->setupEvent(&the_event);
            master_file = out_file;
          } else {
            EXCEPTION_RAISE("Process", "Unable to construct output file for " +
                                           outputFiles_[ifile]);
          }

          for (auto rule : dropKeepRules_) out_file->addDrop(rule);

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

      // In case we'd like to skip up to the event of minEvents_
      while (n_events_processed < (minEvents_ - 1) &&
             master_file->nextEvent(false)) {
        n_events_processed++;
      }

      bool event_completed = true;
      while (master_file->nextEvent(
                 storageController_.keepEvent(event_completed)) &&
             (eventLimit_ < 0 || (n_events_processed) < eventLimit_)) {
        // clean up for storage control calculation
        storageController_.resetEventState();
        logging::Formatter::set(the_event.getEventNumber());

        // notify for new run if necessary
        if (the_event.getEventHeader().getRun() != was_run) {
          was_run = the_event.getEventHeader().getRun();
          ldmx::RunHeader *rh{master_file->getRunHeaderPtr(was_run)};
          if (rh != nullptr) {
            runHeader_ = rh;
            ldmx_log(info) << "Got new run header from '"
                           << master_file->getFileName() << "'";
            newRun(*runHeader_);
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

      bool leave_early{false};
      if (eventLimit_ > 0 && n_events_processed == eventLimit_) {
        ldmx_log(info) << "Reached event limit of " << eventLimit_ << " events";
        leave_early = true;
      }

      if (eventLimit_ == 0 && n_events_processed > eventLimit_) {
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
  if (performance_) performance_->absolute_stop();
}

int Process::getRunNumber() const {
  return (eventHeader_) ? (eventHeader_->getRun()) : (runForGeneration_);
}

TDirectory *Process::makeHistoDirectory(const std::string &dirName) {
  auto owner{openHistoFile()};
  TDirectory *child = owner->mkdir((char *)dirName.c_str());
  if (child) child->cd();
  return child;
}

TDirectory *Process::openHistoFile() {
  TDirectory *owner{nullptr};

  if (histoFilename_.empty()) {
    // trying to write histograms/ntuples but no file defined
    EXCEPTION_RAISE(
        "NoHistFileName",
        "You did not provide the necessary histogram file name to "
        "put your histograms (or performance data) in.\n    Provide this "
        "name in the python configuration with 'p.histogramFile = "
        "\"myHistFile.root\"' where p is the Process object.");
  } else if (histoTFile_ == nullptr) {
    histoTFile_ = new TFile(histoFilename_.c_str(), "RECREATE");
    owner = histoTFile_;
  } else
    owner = histoTFile_;
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
  if ((logFrequency_ != -1) && ((n + 1) % logFrequency_ == 0) && (n_try < 2)) {
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
      performance_->end_event(false);
    }
    return false;
  }
  if (performance_) {
    performance_->stop(performance::Callback::process, 0);
    performance_->end_event(true);
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
