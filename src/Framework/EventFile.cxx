#include <ctime>

#include "TTreeReader.h"

// LDMX
#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "Framework/Exception/Exception.h"
#include "Framework/RunHeader.h"

namespace framework {

EventFile::EventFile(const framework::config::Parameters &params,
                     const std::string &filename, EventFile *parent,
                     bool isOutputFile, bool isSingleOutput, bool isLoopable)
    : fileName_(filename), parent_(parent), isOutputFile_(isOutputFile),
      isSingleOutput_(isSingleOutput), isLoopable_(isLoopable) {

  if (isOutputFile_) {
    // we are writting out so open the file and make sure it is writable
    file_ = new TFile(fileName_.c_str(), "RECREATE");
    if (!file_->IsOpen() or !file_->IsWritable()) {
      EXCEPTION_RAISE("FileError",
                      "Output file '" + fileName_ + "' is not writable.");
    }

    // set compression settings
    //  Check out the TFile constructor for explanation of how this integer is
    //  built Short Reference: setting = 100*algorithem + level algorithm = 0
    //  ==> use global default
    file_->SetCompressionSettings(
        params.getParameter<int>("compressionSetting", 9));

    if (parent_) {
      // output file when there are input files
      //  might be drop/keep rules, so we should have these rules to make sure
      //  it works

      // turn everything on
      //  hypothetically could turn everything off? Doesn't work for some
      //  reason?
      preCloneRules_.emplace_back("*", true);

      // except EventHeader (copies over to output)
      preCloneRules_.emplace_back("EventHeader*", true);

      // reactivate all branches so default behavior is drop
      reactivateRules_.push_back("*");
    }
  } else {
    // open file with only reading enabled
    file_ = new TFile(fileName_.c_str());
    // double check that file is open
    if (!file_->IsOpen()) {
      EXCEPTION_RAISE("FileError", "Input file '" + fileName_ +
                                       "' is not readable or does not exist.");
    }

    // Get the tree name from the configuration
    auto tree_name{params.getParameter<std::string>("tree_name")};
    tree_ = static_cast<TTree *>(file_->Get(tree_name.c_str()));
    if (!tree_) {
      EXCEPTION_RAISE("FileError", "File '" + fileName_ +
                                       "' does not have a TTree named '" +
                                       tree_name + "' in it.");
    }
    entries_ = tree_->GetEntriesFast();
  }

  importRunHeaders();
}

EventFile::EventFile(const framework::config::Parameters &params, 
    const std::string &filename, bool isLoopable)
    : EventFile(params, filename, nullptr, false, false, isLoopable) {}

EventFile::EventFile(const framework::config::Parameters &params,
                     const std::string &filename)
    : EventFile(params, filename, nullptr, false, false, false) {}

EventFile::EventFile(const framework::config::Parameters &params,
                     const std::string &filename, EventFile *parent,
                     bool isSingleOutput)
    : EventFile(params, filename, parent, true, isSingleOutput, false) {}

void EventFile::addDrop(const std::string &rule) {
  int offset;
  bool isKeep = false, isDrop = false, isIgnore = false;
  size_t i = rule.find("keep");
  if (i != std::string::npos) {
    offset = i + 4;
    isKeep = true;
  }
  i = rule.find("drop");
  if (i != std::string::npos) {
    offset = i + 4;
    isDrop = true;
  }
  i = rule.find("ignore");
  if (i != std::string::npos) {
    offset = i + 6;
    isIgnore = true;
  }

  // more than one of (keep,drop,ignore) was provided => not valid rule
  if (int(isKeep) + int(isDrop) + int(isIgnore) != 1)
    return;

  std::string srule = rule.substr(offset);
  for (i = srule.find_first_of(" \t\n\r"); i != std::string::npos;
       i = srule.find_first_of(" \t\n\r"))
    srule.erase(i, 1);

  // name of branch is not given
  if (srule.length() == 0)
    return;

  // add wild card at end for matching purposes
  if (srule.back() != '*')
    srule += ".*"; // add wildcard to back

  if (isKeep) {
    // turn both the input and output tree's on
    // root needs . removed otherwise it gets cranky
    srule.erase(std::remove(srule.begin(), srule.end(), '.'), srule.end());
    preCloneRules_.emplace_back(srule, true);
    // this branch will then be copied over into output tree and be active
  } else if (isIgnore) {
    // don't even read it from the input file
    // root needs . removed otherwise it gets cranky
    srule.erase(std::remove(srule.begin(), srule.end(), '.'), srule.end());
    preCloneRules_.emplace_back(srule, false);
    // these branches won't be copied over into output tree
  } else if (isDrop) {
    // drop means allowing it on reading but not writing
    // pass these regex to event bus so Event::add knows
    event_->addDrop(srule); // requires event_ to be set

    // root needs . removed otherwise it gets cranky
    srule.erase(std::remove(srule.begin(), srule.end(), '.'), srule.end());
    preCloneRules_.emplace_back(srule, false);
    // these branches won't be copied over into output tree
    // reactivate input branch after clone
    reactivateRules_.push_back(srule);
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
      if (!tree_ or !isSingleOutput_) {
        // clones parent_->tree_ to our tree_ keeping drop/keep rules in mind
        // clone tree (only copies over branches that are active on input tree)

        file_->cd();  // go into output file

        for (auto const &rulePair : preCloneRules_)
          parent_->tree_->SetBranchStatus(rulePair.first.c_str(),
                                          rulePair.second);
  
        tree_ = parent_->tree_->CloneTree(0);
  
        // reactivate any drop branches (drop) on input tree
        for (auto const &rule : reactivateRules_)
          parent_->tree_->SetBranchStatus(rule.c_str(), 1);
      }
      event_->setInputTree(parent_->tree_);
      event_->setOutputTree(tree_);
    } //we have a parent file
  } else {
    //later than first entry of file
    if (isOutputFile_) {
      event_->beforeFill();
      if (storeCurrentEvent) // we should store before moving on
        tree_->Fill();  // fill the clones...
    } // we are an output file

    // the event bus may not be defined
    //  for this file if we are input file and
    //  there is an output file during this run
    if (event_) {
      event_->Clear();
      event_->onEndOfEvent();
    } // event bus defined
  } // first or not first entry in this file

  if (parent_) {
    // we have a parent, follow their lead
    if (!parent_->nextEvent()) {
      return false;
    }
    ientry_ = parent_->ientry_;
    entries_++;
  } else if (isOutputFile_) {
    // we don't have a parent and we
    //  are an output file
    // Just increment the number of entries
    //  and the index of the current entry
    ientry_++;
    entries_++;
  } else {
    // we don't have a parent and
    //  we aren't an output file
    // try to load another entry from our tree
    if (ientry_ + 1 >= entries_) {
        if (isLoopable_) {
          // reset the event counter: reuse events from start of pileup tree
          ientry_ = -1; 
          if (event_) {
            // reset tree addresses before 
            //  those objects are de-allocated in onEndOfFile
            tree_->ResetBranchAddresses();
            tree_->Reset();
            // close up event bus since we are at end of a file
            event_->onEndOfFile();
            // re-open file like a new input
            event_->setInputTree(tree_);
          }
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

void EventFile::setupEvent(Event *evt) {
  event_ = evt;
  if (isOutputFile_) {
    //we are an output file
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
  } //output or input file
}

int EventFile::skipToEvent(int offset) {
  // make sure the event number exists,
  // -1 to account for stepping in nextEvent()
  offset = offset % entries_ - 1;
  for (int i_shift{0}; i_shift < offset; i_shift++) {
    if (!this->nextEvent()) return -1;
  }
  return ientry_;
}

void EventFile::updateParent(EventFile *parent) {
  parent_ = parent;

  // we can assume parent_->tree_ is valid
  //  because (for input files) the tree_ is imported
  //  from the file and then checked if its valid in the
  //  EventFile constructor

  // Enter output file
  file_->cd();

  // need to turn on/off the same branches as in the initial setup...
  for (auto const &rulePair : preCloneRules_)
    parent_->tree_->SetBranchStatus(rulePair.first.c_str(), rulePair.second);

  // Copy over addresses from the new parent
  parent_->tree_->CopyAddresses(tree_);

  // and reactivate any dropping rules
  for (auto const &rule : reactivateRules_)
    parent_->tree_->SetBranchStatus(rule.c_str(), 1);

  // Reset the entry index with the new parent index
  ientry_ = parent_->ientry_;

  // import run headers from new input file
  importRunHeaders();

  return;
}

void EventFile::close() {
  // MEMORY 'Conditional jump or move depends on uninitialised values' when
  // closing TFile and/or writing TTree
  //  TFile::Close --> TDirectoryFile::Close --> ~TTree
  // MEMORY 'Syscall param write(buf) points to uninitialised byte(s)'
  //  when writing TTree below
  //  From filling TTree in nextEvent

  // Before an output file, the Event tree needs to be written.
  if (isOutputFile_) {
    // make sure we are in output file before writing
    file_->cd();
    tree_->Write();
    // store the run map into the output tree

    // Check for the existence of the run tree in the file.
    // If it already exists, throw an exception.
    // TODO: Tree name shouldn't be hardcoded. Is this check really necessary?
    auto runTree{static_cast<TTree *>(file_->Get("LDMX_Run"))};
    if (runTree) {
      EXCEPTION_RAISE("RunTree",
                      "RunTree 'LDMX_Run' already exists in output file '" +
                          fileName_ + "'.");
    }

    runTree = new TTree("LDMX_Run", "LDMX run header");

    // create the branch on this tree
    ldmx::RunHeader *theHandle = nullptr;
    runTree->Branch("RunHeader", "ldmx::RunHeader", &theHandle, 32000, 3);

    // copy over the run headers into the tree
    for (auto &[num, header_pair] : runMap_) {
      theHandle = header_pair.second;
      runTree->Fill();
      if (header_pair.first)
        delete header_pair.second;
    }

    runTree->Write();
  }

  // Close the file
  file_->Close();
}

void EventFile::writeRunHeader(ldmx::RunHeader &runHeader) {
  int runNumber = runHeader.getRunNumber();

  if (runMap_.find(runNumber) != runMap_.end()) {
    EXCEPTION_RAISE("RunMap", "Run map already contains a run with number '" +
                                  std::to_string(runNumber) + "'.");
  }

  runMap_[runNumber] = std::make_pair(false, &runHeader);

  return;
}

ldmx::RunHeader &EventFile::getRunHeader(int runNumber) {
  if (runMap_.find(runNumber) != runMap_.end()) {
    return *(runMap_.at(runNumber).second);
  } else {
    EXCEPTION_RAISE("DataError", "No run header exists for " +
                                     std::to_string(runNumber) +
                                     " in the run map.");
  }
}

void EventFile::importRunHeaders() {
  // choose which file to import from
  auto theImportFile{file_}; // if this is an input file
  if (isOutputFile_ and parent_ and parent_->file_)
    theImportFile = parent_->file_;  // output file with input parent
  else if (isOutputFile_)
    return; // output file, no input parent to read from

  if (theImportFile) {
    // the file exist
    TTreeReader oldRunTree("LDMX_Run", theImportFile);
    TTreeReaderValue<ldmx::RunHeader> oldRunHeader(oldRunTree, "RunHeader");
    // TODO check that setup went correctly
    while (oldRunTree.Next()) {
      // copy input run tree into run map
      runMap_[oldRunHeader->getRunNumber()] =
          std::make_pair(true, new ldmx::RunHeader(*oldRunHeader));
    }
  }

  return;
}
} // namespace framework
