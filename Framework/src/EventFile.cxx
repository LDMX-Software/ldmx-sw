// LDMX
#include "Framework/EventFile.h"
#include "Framework/Event.h"
#include "Exception/Exception.h"
#include "Event/EventConstants.h"
#include "Event/RunHeader.h"

namespace ldmx {

    EventFile::EventFile(const std::string& filename, std::string treeName, bool isOutputFile, int compressionLevel) :
                fileName_(filename), isOutputFile_(isOutputFile) {

        if (isOutputFile_) {
            file_ = new TFile(filename.c_str(), "RECREATE");
            if (!file_->IsWritable()) {
                EXCEPTION_RAISE("FileError", "Output file '" + filename + "' is not writable");
            }

        } else {
            file_ = new TFile(filename.c_str());
        }

        if (!file_->IsOpen()) {
            EXCEPTION_RAISE("FileError", "File '" + filename + "' is not readable or does not exist.");
        }

        if (isOutputFile_) {
            file_->SetCompressionLevel(compressionLevel);
        }

        if (!isOutputFile_) {
            tree_ = (TTree*) (file_->Get(treeName.c_str()));
            entries_ = tree_->GetEntriesFast();
        }

        // Create run map from tree in this file.
        createRunMap();
    }

    EventFile::EventFile(const std::string& filename, bool isOutputFile, int compressionLevel) :
                EventFile(filename, EventConstants::EVENT_TREE_NAME, isOutputFile, compressionLevel) {
    }

    EventFile::EventFile(const std::string& filename, EventFile* cloneParent, bool isSingleOutput, int compressionLevel) :
                fileName_(filename), isOutputFile_(true), isSingleOutput_(isSingleOutput), parent_(cloneParent) {

        file_ = new TFile(filename.c_str(), "RECREATE");
        if (!file_->IsWritable()) {
            EXCEPTION_RAISE("FileError", "Output file '" + filename + "' is not writable");
        }

        if (!file_->IsOpen()) {
            EXCEPTION_RAISE("FileError", "File '" + filename + "' is not readable or does not exist.");
        }

        parent_->tree_->SetBranchStatus("*", 0); //turn everything off
        parent_->tree_->SetBranchStatus( (EventConstants::EVENT_HEADER + "*").c_str() , 1); //turn EventHeader on

        if (isOutputFile_) {
            file_->SetCompressionLevel(compressionLevel);
        }

        // Copy run headers from parent to output file.
        copyRunHeaders();

        // Create run header map.
        createRunMap();
    }

    EventFile::~EventFile() {
        for (auto entry : runMap_) {
            delete entry.second;
        }
        runMap_.clear();
    }

    void EventFile::addDrop(const std::string& rule) {

        int offset;
        bool isKeep=false,isDrop=false,isIgnore=false;
        size_t i = rule.find("keep");
        if( i != std::string::npos ){
            offset = i+4 ;
            isKeep = true;
        }
        i = rule.find("drop");
        if( i != std::string::npos ){
            offset = i+4 ;
            isDrop = true;
        }
        i = rule.find("ignore");
        if( i != std::string::npos ){
            offset = i+6 ;
            isIgnore = true;
        }

        //more than one of (keep,drop,ignore) was provided => not valid rule
        if( int(isKeep) + int(isDrop) + int(isIgnore) != 1 ) return;

        std::string srule = rule.substr(offset);
        for (i = srule.find_first_of(" \t\n\r"); i != std::string::npos; i = srule.find_first_of(" \t\n\r"))
            srule.erase(i, 1);

        //name of branch is not given
        if (srule.length() == 0) return;

        //add wild card at end for matching purposes
        if (srule.back() != '*') srule += ".*"; //add wildcard to back

        if( isKeep ) {
            //turn both the input and output tree's on
            //root needs . removed otherwise it gets cranky
            srule.erase( std::remove( srule.begin(), srule.end(), '.' ) , srule.end() );
            if ( parent_ ) parent_->tree_->SetBranchStatus(srule.c_str(),1);
            if ( tree_ ) tree_->SetBranchStatus(srule.c_str(),1);
        } else if( isIgnore ) {
            //don't even read it from the input file
            //root needs . removed otherwise it gets cranky
            srule.erase( std::remove( srule.begin(), srule.end(), '.' ) , srule.end() );
            if ( parent_ ) parent_->tree_->SetBranchStatus(srule.c_str(),0);
            if ( tree_ ) tree_->SetBranchStatus(srule.c_str(),0);
        } else if ( isDrop ) {
            //drop means allowing it on reading but not writing
            // pass these regex to event bus
            event_->addDrop( srule ); //requires event_ to be set

            //root needs . removed otherwise it gets cranky
            srule.erase( std::remove( srule.begin(), srule.end(), '.' ) , srule.end() );

            if ( parent_ ) {
                if ( not tree_ ) {
                    //deactivate this branch before clone
                    parent_->tree_->SetBranchStatus(srule.c_str(),0);
                    tree_ = parent_->tree_->CloneTree(0);
                }
                //reactivate the read-in branch
                parent_->tree_->SetBranchStatus(srule.c_str(),1);
            }

            //deactivate branch on output tree
            unsigned int f = 0; //look at this ROOT nonsense 
            // ==> the third parameter *must* be an address to an unsigned int
            // apparently Rene has never heard of pass by reference ¯\_(ツ)_/¯
            if ( tree_ ) tree_->SetBranchStatus(srule.c_str(),0,&f);//third parameter suppresses warning message
        }
    }

    bool EventFile::nextEvent(bool storeCurrentEvent) {

        if (ientry_ < 0 && parent_) {
            if (!parent_->tree_) {
                EXCEPTION_RAISE("EventFile", "No event tree in the file");
            }

            //Only clone parent tree if either
            //  1) There is no tree setup yet (first input file)
            //  2) This is not single output (new input file --> new output file)
            if ( !tree_ or !isSingleOutput_ ) {
                //TODO this may mean that collecitons aren't dropped after first file
                tree_ = parent_->tree_->CloneTree(0);
            }
            event_->setInputTree( parent_->tree_ );
            event_->setOutputTree( tree_ );
        }
        
        // close up the last event
        if (ientry_ >= 0) {
            if (isOutputFile_) {
                event_->beforeFill();
                if (storeCurrentEvent) tree_->Fill(); // fill the clones...
            }
            if (event_) {
                event_->Clear();
                event_->onEndOfEvent();
            }
        }

        if (parent_) {
            if (!parent_->nextEvent()) {
                return false;
            }
            parent_->tree_->GetEntry(parent_->ientry_);
            ientry_ = parent_->ientry_;
            event_->nextEvent();
            entries_++;
            return true;

        } else {

            // if we are reading, move the pointer
            if (!isOutputFile_) {

                if (ientry_ + 1 >= entries_) {
                    return false;
                }

                ientry_++;
                tree_->LoadTree(ientry_);

                if (event_) {
                    event_->nextEvent();
                }
                return true;

            } else {
                ientry_++;
                entries_++;
                return true;
            }
        }
        return false;
    }

    void EventFile::setupEvent(Event* evt) {
        event_ = evt;
        if (isOutputFile_) {
            if (!tree_ && !parent_) {
                tree_ = event_->createTree();
                ientry_ = 0;
                entries_ = 0;
            }

            if (parent_) {
                event_->setInputTree(parent_->tree_);
            }
        } else {
            event_->setInputTree(tree_);
        }
    }

    void EventFile::updateParent(EventFile* parent) { 
        
        parent_ = parent; 
        
        TTree* parentTree = (TTree *)parent_->file_->Get("LDMX_Events");
        if ( parentTree ) {
            
            //Enter output file
            file_->cd();
            
            //Copy over addresses from the new parent
            //  Taken from TTree::CopyAddresses but removed warning when branch not found
            //  The outBranch may not exist for each inBranch because we may be using some drop/ignore rules
            TObjArray *branches = parentTree->GetListOfBranches();
            int nbranches = branches->GetEntriesFast();
            for ( int iBr = 0; iBr < nbranches; iBr++ ) {
                TBranch *inBranch = (TBranch *)branches->At(iBr);
                if ( inBranch->TestBit(kDoNotProcess) ) {
                    continue; //parent tree has this inBranch turned off ==> skip
                }
                char* addr = inBranch->GetAddress();
                TBranch *outBranch = tree_->GetBranch( inBranch->GetName() );
                if ( outBranch ) {
                    //if outTree has this branch -> reset address
                    outBranch->SetAddress( addr );
                    if ( outBranch->InheritsFrom(TBranchElement::Class()) ) {
                        ((TBranchElement*)outBranch)->ResetDeleteObject();
                    } //reset object if more complicated
                } //if outBranch was found
            } //loop through parentTree branches

            //Reset the entry index with the new parent index
            ientry_ = parent_->ientry_;
        }

        //copy over run headers and recreate run map
        copyRunHeaders();
        createRunMap();

        return;
    }

    void EventFile::close() {
        
        // Before an output file, the Event tree needs to be written. 
        if (isOutputFile_) tree_->Write();

        // Close the file
        file_->Close();
    }

    void EventFile::writeRunHeader(RunHeader* runHeader) {
    
        // Before writing the event header, make sure the file is an output
        // file.  TODO: Is checking whether the file is writable good enough?  
        if (!isOutputFile_) {
            EXCEPTION_RAISE("FileError", "Output file '" + fileName_ + "' is not writable.");
        }
        
        // Check for the existence of the run tree in the file.  If it doesn't
        // exists, create it. 
        // TODO: Tree name shouldn't be hardcoded. Is this check really necessary?
        auto runTree{static_cast<TTree*>(file_->Get("LDMX_Run"))};
        if (!runTree) runTree = new TTree("LDMX_Run", "LDMX run header");

        // Check for the existence of the "RunHeader" branch in the tree.  If
        // it doesn't exists, crate it. 
        // TODO: Is this check really necessary?
        auto runBranch{runTree->GetBranch("RunHeader")};
        if (!runBranch) 
            runTree->Branch("RunHeader", EventConstants::RUN_HEADER.c_str(), &runHeader, 32000, 3);

        // Fill the tree and write it to the file. 
        runTree->Fill();
        runTree->Write();
    }

    const RunHeader& EventFile::getRunHeader(int runNumber) {
        if (runMap_.find(runNumber) != runMap_.end()) {
            return *(runMap_[runNumber]);
        } else {
            EXCEPTION_RAISE("DataError", "No run header exists for " + std::to_string(runNumber) + " in the input file.");
        }
    }

    void EventFile::createRunMap() {
        TTree* runTree = (TTree*) file_->Get("LDMX_Run");
        if (runTree) {
            RunHeader* aRunHeader = nullptr;
            runTree->SetBranchAddress("RunHeader", &aRunHeader);
            for (int iEntry = 0; iEntry < runTree->GetEntriesFast(); iEntry++) {
                runTree->GetEntry(iEntry);
                RunHeader* newRunHeader = new RunHeader();
                *newRunHeader = *aRunHeader; //copy over run header
                runMap_[newRunHeader->getRunNumber()] = newRunHeader;
            }
            runTree->ResetBranchAddresses();
        }
    }

    void EventFile::copyRunHeaders() {
        if (parent_ && parent_->file_) {
            TTree* oldtree = (TTree*)parent_->file_->Get("LDMX_Run");
            if (oldtree && !file_->Get("LDMX_Run")) {
                oldtree->SetBranchStatus("RunHeader", 1);
                file_->cd();
                TTree* newtree = oldtree->CloneTree();
                if ( newtree ) {
                    file_->Write();
                    file_->Flush();
                }
            }
        }
    }

}
