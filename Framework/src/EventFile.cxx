#include <ctime> 

#include "TTreeReader.h"

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

        importRunHeaders();

        processStart_ = std::time(nullptr); 
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

        //turn everything off
        preCloneRules_.emplace_back( "*" , true );

        //except EventHeader (copies over to output)
        preCloneRules_.emplace_back( "EventHeader*" , true );

        //reactivate all branches so default behavior is drop
        reactivateRules_.push_back( "*" );

        if (isOutputFile_) {
            file_->SetCompressionLevel(compressionLevel);
        }

        importRunHeaders();
        
        processStart_ = std::time(nullptr); 
    }

    EventFile::~EventFile() {
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
            preCloneRules_.emplace_back( srule , true );
            //this branch will then be copied over into output tree and be active
        } else if( isIgnore ) {
            //don't even read it from the input file
            //root needs . removed otherwise it gets cranky
            srule.erase( std::remove( srule.begin(), srule.end(), '.' ) , srule.end() );
            preCloneRules_.emplace_back( srule , false );
            //these branches won't be copied over into output tree
        } else if ( isDrop ) {
            //drop means allowing it on reading but not writing
            // pass these regex to event bus so Event::add knows
            event_->addDrop( srule ); //requires event_ to be set

            //root needs . removed otherwise it gets cranky
            srule.erase( std::remove( srule.begin(), srule.end(), '.' ) , srule.end() );
            preCloneRules_.emplace_back( srule , false );
            //these branches won't be copied over into output tree
            //reactivate input branch after clone
            reactivateRules_.push_back( srule );
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
                //clones parent_->tree_ to our tree_ keeping drop/keep rules in mind
                //clone tree (only copies over branches that are active on input tree)
                
                file_->cd(); //go into output file

                for ( auto const& rulePair : preCloneRules_ )
                    parent_->tree_->SetBranchStatus( rulePair.first.c_str() , rulePair.second );

                tree_ = parent_->tree_->CloneTree(0);
        
                //reactivate any drop branches (drop) on input tree
                for ( auto const& rule : reactivateRules_ ) 
                    parent_->tree_->SetBranchStatus( rule.c_str(), 1 );

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

            //need to turn on/off the same branches as in the initial setup...
            for ( auto const& rulePair : preCloneRules_ )
                parent_->tree_->SetBranchStatus( rulePair.first.c_str() , rulePair.second );
            
            //Copy over addresses from the new parent
            parentTree->CopyAddresses( tree_ );

            //and reactivate any dropping rules
            for ( auto const& rule : reactivateRules_ )
                parent_->tree_->SetBranchStatus( rule.c_str() , 1 );
            
            //Reset the entry index with the new parent index
            ientry_ = parent_->ientry_;
        }

        //import run headers from new input file
        importRunHeaders();

        return;
    }

    void EventFile::close() {
        
        // Before an output file, the Event tree needs to be written. 
        if (isOutputFile_) {
            tree_->Write();
            //store the run map into the output tree

            // Check for the existence of the run tree in the file. 
            // If it already exists, throw an exception.
            // TODO: Tree name shouldn't be hardcoded. Is this check really necessary?
            auto runTree{static_cast<TTree*>(file_->Get("LDMX_Run"))};
            if (runTree) {
                EXCEPTION_RAISE(
                        "RunTree",
                        "RunTree 'LDMX_Run' already exists in output file '" + fileName_ + "'."
                        );
            }
                
            runTree = new TTree("LDMX_Run", "LDMX run header");
    
            //create the branch on this tree
            RunHeader *theHandle = nullptr;
            runTree->Branch("RunHeader", EventConstants::RUN_HEADER.c_str(), &theHandle, 32000, 3);

            //copy over the run headers into the tree
            for( auto& runTuple : runMap_ ) {
                theHandle = &(runTuple.second);
                runTree->Fill();
            }

            runTree->Write();
        }

        // Close the file
        file_->Close();
    }

    void EventFile::writeRunHeader(RunHeader& runHeader) {
    
        int runNumber = runHeader.getRunNumber();

        if ( runMap_.find(runNumber) != runMap_.end() ) {
            EXCEPTION_RAISE( "RunMap" , 
                    "Run map already contains a run with number '" + std::to_string(runNumber)
                    + "'." );
        }

        runHeader.setRunStart(processStart_);
        runHeader.setRunEnd(std::time(nullptr));

        runMap_[runNumber] = runHeader;

        return;
    }

    const RunHeader& EventFile::getRunHeader(int runNumber) {
        if (runMap_.find(runNumber) != runMap_.end()) {
            return runMap_[runNumber];
        } else {
            EXCEPTION_RAISE("DataError", "No run header exists for " + std::to_string(runNumber) + " in the run map.");
        }
    }

    void EventFile::importRunHeaders() {

        //choose which file to import from
        auto theImportFile{file_}; //if this is an input file
        if ( isOutputFile_ and parent_ and parent_->file_ ) 
            theImportFile = parent_->file_;

        if (theImportFile) {
            //the file exist
            TTreeReader oldRunTree( "LDMX_Run" , theImportFile );
            TTreeReaderValue<RunHeader> oldRunHeader( oldRunTree , "RunHeader" );
            //TODO check that setup went correctly
            while( oldRunTree.Next() ) {
                //copy input run tree into run map
                runMap_[ oldRunHeader->getRunNumber() ] = *oldRunHeader;
            }
        }

        return;
    }
}
