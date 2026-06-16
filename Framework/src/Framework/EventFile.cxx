#include <regex.h>

#include <ctime>

#include "TTreeReader.h"

// LDMX
#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "Framework/Exception/Exception.h"
#include "Framework/RunHeader.h"

namespace framework {

EventFile::EventFile(const framework::config::Parameters& params,
                     const std::string& filename, EventFile* parent,
                     bool is_output_file, bool is_single_output,
                     bool is_loopable)
    : file_name_(filename),
      is_output_file_(is_output_file),
      is_single_output_(is_single_output),
      is_loopable_(is_loopable),
      parent_(parent) {
  if (is_output_file_) {
    // we are writting out so open the file and make sure it is writable
    file_ = new TFile(file_name_.c_str(), "RECREATE");
    if (!file_->IsOpen() or !file_->IsWritable()) {
      EXCEPTION_RAISE("FileError",
                      "Output file '" + file_name_ + "' is not writable.");
    }

    // set compression settings
    //  Check out the TFile constructor for explanation of how this integer is
    //  built Short Reference: setting = 100*algorithem + level algorithm = 0
    //  ==> use global default
    file_->SetCompressionSettings(params.get<int>("compression_setting", 9));

    if (parent_) {
      // output file when there are input files
      //  might be drop/keep rules, so we should have these rules to make sure
      //  it works

      // turn everything on
      //  hypothetically could turn everything off? Doesn't work for some
      //  reason?
      pre_clone_rules_.emplace_back("*", true);

      // except EventHeader (copies over to output)
      pre_clone_rules_.emplace_back("EventHeader*", true);

      // reactivate all branches so default behavior is drop
      reactivate_rules_.push_back("*");
    }
  } else {
    // open file with only reading enabled
    file_ = new TFile(file_name_.c_str());
    // double check that file is open
    if (!file_->IsOpen()) {
      EXCEPTION_RAISE("FileError", "Input file '" + file_name_ +
                                       "' is not readable or does not exist.");
    }

    bool skip_corrupted = params.get<bool>("skip_corrupted_input_files", false);

    // make sure file is not a zombie file
    // (i.e. process ended without closing or the file was corrupted some other
    // way)
    if (file_->IsZombie()) {
      if (not skip_corrupted) {
        EXCEPTION_RAISE("FileError", "Input file '" + file_name_ +
                                         "' is corrupted. Framework will not "
                                         "attempt to recover this file.");
      }
      return;
    }

    // Get the tree name from the configuration
    auto tree_name{params.get<std::string>("tree_name")};
    tree_ = static_cast<TTree*>(file_->Get(tree_name.c_str()));
    if (!tree_) {
      if (not skip_corrupted) {
        EXCEPTION_RAISE("FileError", "File '" + file_name_ +
                                         "' does not have a TTree named '" +
                                         tree_name + "' in it.");
      }
      return;
    }
    entries_ = tree_->GetEntriesFast();
  }

  importRunHeaders();
}

EventFile::EventFile(const framework::config::Parameters& params,
                     const std::string& filename, bool is_loopable)
    : EventFile(params, filename, nullptr, false, false, is_loopable) {}

EventFile::EventFile(const framework::config::Parameters& params,
                     const std::string& filename)
    : EventFile(params, filename, nullptr, false, false, false) {}

EventFile::EventFile(const framework::config::Parameters& params,
                     const std::string& filename, EventFile* parent,
                     bool is_single_output)
    : EventFile(params, filename, parent, true, is_single_output, false) {}

EventFile::~EventFile() {
  // Before an output file, the Event tree needs to be written.
  if (is_output_file_) {
    // make sure we are in output file before writing
    file_->cd();
    tree_->Write();
  }

  // detach objects that may/may not be out of scope already
  // by resetting branch addresses immediately before closing
  // this is a HACK that I'm embarrased by
  tree_->ResetBranchAddresses();
  // Close the file
  file_->Close();
}

bool EventFile::isCorrupted() const {
  if (is_output_file_) return file_->IsZombie();
  return (!tree_ or file_->IsZombie() or file_->GetNkeys() == 0);
}

void EventFile::addDrop(const std::string& rule) {
  int offset;
  bool is_keep = false, is_drop = false, is_ignore = false;
  // keywords must appear at the start of the rule string
  if (rule.find("keep") == 0) {
    offset = 4;
    is_keep = true;
  } else if (rule.find("drop") == 0) {
    offset = 4;
    is_drop = true;
  } else if (rule.find("ignore") == 0) {
    offset = 6;
    is_ignore = true;
  }

  // none of (keep,drop,ignore) was provided => not valid rule
  if (int(is_keep) + int(is_drop) + int(is_ignore) != 1) return;

  std::string srule = rule.substr(offset);
  size_t i;
  for (i = srule.find_first_of(" \t\n\r"); i != std::string::npos;
       i = srule.find_first_of(" \t\n\r"))
    srule.erase(i, 1);

  // name of branch is not given
  if (srule.length() == 0) return;

  // add wild card at end for matching purposes
  if (srule.back() != '*') srule += ".*";  // add wildcard to back

  // Guard: EventHeader must never be dropped or ignored
  if (is_drop or is_ignore) {
    regex_t guard_reg;
    if (regcomp(&guard_reg, srule.c_str(),
                REG_EXTENDED | REG_ICASE | REG_NOSUB) == 0) {
      bool matches_event_header =
          (regexec(&guard_reg, ldmx::EventHeader::BRANCH.c_str(), 0, 0, 0) ==
           0);
      regfree(&guard_reg);
      if (matches_event_header) {
        EXCEPTION_RAISE("BadRule",
                        "Drop/ignore rule '" + rule +
                            "' would affect EventHeader which is required by "
                            "the framework and cannot be removed.");
      }
    }
  }

  if (is_keep) {
    // turn both the input and output tree's on
    // root needs . removed otherwise it gets cranky
    srule.erase(std::remove(srule.begin(), srule.end(), '.'), srule.end());
    pre_clone_rules_.emplace_back(srule, true);
    // this branch will then be copied over into output tree and be active
  } else if (is_ignore) {
    // don't even read it from the input file
    // pass regex (with dots) to event bus so setInputTree skips these branches
    event_->addIgnore(srule);  // requires event_ to be set
    // root needs . removed otherwise it gets cranky
    srule.erase(std::remove(srule.begin(), srule.end(), '.'), srule.end());
    // warn if this rule drops all collections
    if (srule == "*")
      ldmx_log(fatal) << "Ignore rule '" << rule
                      << "' will hide all input collections from processors.";
    pre_clone_rules_.emplace_back(srule, false);
    // these branches won't be copied over into output tree
  } else if (is_drop) {
    // drop means allowing it on reading but not writing
    // pass these regex to event bus so Event::add knows
    event_->addDrop(srule);  // requires event_ to be set

    // root needs . removed otherwise it gets cranky
    srule.erase(std::remove(srule.begin(), srule.end(), '.'), srule.end());
    // warn if this rule drops all collections
    if (srule == "*")
      ldmx_log(fatal) << "Drop rule '" << rule
                      << "' will drop all collections from the output file.";
    pre_clone_rules_.emplace_back(srule, false);
    // these branches won't be copied over into output tree
    // reactivate input branch after clone
    reactivate_rules_.push_back(srule);
  }
}

bool EventFile::nextEvent(bool storeCurrentEvent) {
  if (ientry_ < 0) {
    // first entry of this file
    if (parent_) {
      // we have a parent file
      if (!parent_->tree_) {
        // this should _never_ happen
        EXCEPTION_RAISE("EventFile", "No event tree in the file");
      }
      // Only clone parent tree if either
      //  1) There is no tree setup yet (first input file)
      //  2) This is not single output (new input file --> new output file)
      if (!tree_ or !is_single_output_) {
        // clones parent_->tree_ to our tree_ keeping drop/keep rules in mind
        // clone tree (only copies over branches that are active on input tree)

        file_->cd();  // go into output file

        for (auto const& rule_pair : pre_clone_rules_)
          parent_->tree_->SetBranchStatus(rule_pair.first.c_str(),
                                          rule_pair.second);

        tree_ = parent_->tree_->CloneTree(0);

        // reactivate any drop branches (drop) on input tree
        for (auto const& rule : reactivate_rules_)
          parent_->tree_->SetBranchStatus(rule.c_str(), 1);
      }
      event_->setInputTree(parent_->tree_);
      event_->setOutputTree(tree_);
    }  // we have a parent file
  } else {
    // later than first entry of file
    if (is_output_file_) {
      event_->beforeFill();
      if (storeCurrentEvent)  // we should store before moving on
        tree_->Fill();        // fill the clones...
    }  // we are an output file

    // the event bus may not be defined
    //  for this file if we are input file and
    //  there is an output file during this run
    if (event_) {
      event_->clear();
      event_->onEndOfEvent();
    }  // event bus defined
  }  // first or not first entry in this file

  if (parent_) {
    // we have a parent, follow their lead
    if (!parent_->nextEvent()) {
      return false;
    }
    ientry_ = parent_->ientry_;
    entries_++;
  } else if (is_output_file_) {
    // we don't have a parent and we
    //  are an output file
    // Just increment the number of entries
    //  and the index_ of the current entry
    ientry_++;
    entries_++;
  } else {
    // we don't have a parent and
    //  we aren't an output file
    // try to load another entry from our tree
    if (ientry_ + 1 >= entries_) {
      if (is_loopable_) {
        // reset the event counter: reuse events from start of pileup tree
        ientry_ = -1;
      } else
        return false;
    }
    ientry_++;
    tree_->GetEntry(ientry_);
  }

  // if we have an event_
  //  make sure it is iterated as well
  return event_ ? event_->nextEvent() : true;
}

void EventFile::setupEvent(Event* evt) {
  event_ = evt;
  if (is_output_file_) {
    // we are an output file
    if (!tree_ && !parent_) {
      // we don't have a tree and we don't have a parent
      //  ==> *Production Mode* create a new tree
      tree_ = event_->createTree();
      ientry_ = 0;
      entries_ = 0;
    }

    if (parent_) {
      // we have a parent file so give
      //  the parent's tree to the event bus
      //  as the input tree
      event_->setInputTree(parent_->tree_);
    }

    // give our tree to the event as the output tree
    event_->setOutputTree(tree_);
  } else {
    // we are an input file
    //  so give our tree to the event as input tree
    event_->setInputTree(tree_);
  }  // output or input file
}

int EventFile::skipToEvent(int offset) {
  // make sure the event number exists
  ientry_ = offset % entries_ - 1;
  return ientry_;
}

void EventFile::updateParent(EventFile* parent) {
  parent_ = parent;

  // we can assume parent_->tree_ is valid
  //  because (for input files) the tree_ is imported
  //  from the file and then checked if its valid in the
  //  EventFile constructor

  // Enter output file
  file_->cd();

  // need to turn on/off the same branches as in the initial setup...
  for (auto const& rule_pair : pre_clone_rules_)
    parent_->tree_->SetBranchStatus(rule_pair.first.c_str(), rule_pair.second);

  // Copy over addresses from the new parent
  parent_->tree_->CopyAddresses(tree_);

  // and reactivate any dropping rules
  for (auto const& rule : reactivate_rules_)
    parent_->tree_->SetBranchStatus(rule.c_str(), 1);

  // Reset the entry index_ with the new parent index_
  ientry_ = parent_->ientry_;

  // import run headers from new input file
  importRunHeaders();

  return;
}

void EventFile::writeRunTree() {
  if (not is_output_file_) {
    EXCEPTION_RAISE("MisCall",
                    "Cannot write the run tree on an input event file.");
  }

  // store the run map into the output tree
  // Check for the existence of the run tree in the file.
  // If it already exists, throw an exception.
  // TODO: Tree name shouldn't be hardcoded. Is this check really necessary?
  auto run_tree{static_cast<TTree*>(file_->Get("LDMX_Run"))};
  if (run_tree) {
    EXCEPTION_RAISE("RunTree",
                    "RunTree 'LDMX_Run' already exists in output file '" +
                        file_name_ + "'.");
  }

  /**
   * ROOT requires us to be in the correct directory when we create
   * the output TTree for us to have it written into the correct
   * location.
   *
   * In order to allow downstream processors to getHistoDirectory()
   * and enter that directory for this histograms we need to enter
   * the output event file again here so that our run tree ends up
   * in the correct location.
   */
  file_->cd();
  run_tree = new TTree("LDMX_Run", "LDMX run header");

  // create the branch on this tree
  ldmx::RunHeader* the_handle = nullptr;
  run_tree->Branch("RunHeader", "ldmx::RunHeader", &the_handle, 32000, 3);

  // copy over the run headers into the tree
  for (auto& [num, header_pair] : run_map_) {
    the_handle = header_pair.second;
    run_tree->Fill();
    if (header_pair.first) delete header_pair.second;
  }

  run_tree->Write();
}

void EventFile::writeRunHeader(ldmx::RunHeader& run_header) {
  int run_number = run_header.getRunNumber();

  if (run_map_.find(run_number) != run_map_.end()) {
    EXCEPTION_RAISE("RunMap", "Run map already contains a run with number '" +
                                  std::to_string(run_number) + "'.");
  }

  run_map_[run_number] = std::make_pair(false, &run_header);

  return;
}

ldmx::RunHeader* EventFile::getRunHeaderPtr(int run_number) {
  if (run_map_.find(run_number) != run_map_.end()) {
    return run_map_.at(run_number).second;
  }
  return nullptr;
}

ldmx::RunHeader& EventFile::getRunHeader(int run_number) {
  ldmx::RunHeader* rh{this->getRunHeaderPtr(run_number)};
  if (rh != nullptr) {
    return *rh;
  }
  EXCEPTION_RAISE("RunHeader", "Unable to find header for run " +
                                   std::to_string(run_number));
}

void EventFile::importRunHeaders() {
  // choose which file to import from
  auto the_import_file{file_};  // if this is an input file
  if (is_output_file_ and parent_ and parent_->file_)
    the_import_file = parent_->file_;  // output file with input parent
  else if (is_output_file_)
    return;  // output file, no input parent to read from

  if (the_import_file) {
    // the file exist
    TTreeReader old_run_tree("LDMX_Run", the_import_file);
    TTreeReaderValue<ldmx::RunHeader> old_run_header(old_run_tree, "RunHeader");
    // TODO check that setup went correctly
    while (old_run_tree.Next()) {
      auto* old_run_header_ptr = old_run_header.Get();
      if (old_run_header_ptr != nullptr) {
        // copy input run tree into run map
        // We should consider moving to a shared_ptr instead of 'new'
        run_map_[old_run_header_ptr->getRunNumber()] =
            std::make_pair(true, new ldmx::RunHeader(*old_run_header_ptr));
      }
    }
  }

  return;
}
}  // namespace framework
