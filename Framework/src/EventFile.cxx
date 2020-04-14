#include <ctime> 

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

        // Copy run headers from parent to output file.
        copyRunHeaders();

        // Create run header map.
        createRunMap();
        
        processStart_ = std::time(nullptr); 
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
                //tree_->AddFriend( parent_->tree_ );
        
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
            //copyAddresses( parentTree );

            //and reactivate any dropping rules
            for ( auto const& rule : reactivateRules_ )
                parent_->tree_->SetBranchStatus( rule.c_str() , 1 );
            
            //Reset the entry index with the new parent index
            ientry_ = parent_->ientry_;
        }

        //copy over run headers and recreate run map
        //TODO untested
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

        runHeader->setRunStart(processStart_); 
        runHeader->setRunEnd(std::time(nullptr)); 

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
                file_->Write();
                file_->Flush();
            }
        }
    }

    void EventFile::copyAddresses(TTree* parentTree) {
        //  Taken from TTree::CopyAddresses but removed warning when branch not found
        //  The outBranch may not exist for each inBranch because we may be using some drop/ignore rules
        //  Also removed option to 'undo' address copy
        TObjArray* branches = parentTree->GetListOfBranches();
        Int_t nbranches = branches->GetEntriesFast();
        for (Int_t i = 0; i < nbranches; ++i) {
            TBranch* branch = (TBranch*) branches->UncheckedAt(i);
            if (branch->TestBit(kDoNotProcess)) {
                //skip deactivated branches
                continue;
            }
            char* addr = branch->GetAddress();
            if (!addr) {
                if (branch->IsA() == TBranch::Class()) {
                    // If the branch was created using a leaflist, the branch itself may not have
                    // an address but the leaf might already.
                    TLeaf *firstleaf = (TLeaf*)branch->GetListOfLeaves()->At(0);
                    if (!firstleaf || firstleaf->GetValuePointer()) {
                        // Either there is no leaf (and thus no point in copying the address)
                        // or the leaf has an address but we can not copy it via the branche
                        // this will be copied via the next loop (over the leaf).
                        continue;
                    }
                }
                // Note: This may cause an object to be allocated.
                branch->SetAddress(0);
                addr = branch->GetAddress();
            }
            // FIXME: The GetBranch() function is braindead and may
            //        not find the branch!
            TBranch* br = tree_->GetBranch(branch->GetName());
            if (br) {
                br->SetAddress(addr);
                // The copy does not own any object allocated by SetAddress().
                if (br->InheritsFrom(TBranchElement::Class())) {
                    ((TBranchElement*) br)->ResetDeleteObject();
                }
            } //else //warning here in TTree::CopyAddresses
        } //loop through parentTree branches

        // Copy branch addresses starting from leaves.
        TObjArray* tleaves = tree_->GetListOfLeaves();
        Int_t ntleaves = tleaves->GetEntriesFast();
        for (Int_t i = 0; i < ntleaves; ++i) {
            TLeaf* tleaf = (TLeaf*) tleaves->UncheckedAt(i);
            TBranch* tbranch = tleaf->GetBranch();
            TBranch* branch = parentTree->GetBranch(tbranch->GetName());
            if (!branch) {
                continue;
            }
            TLeaf* leaf = branch->GetLeaf(tleaf->GetName());
            if (!leaf) {
                continue;
            }
            if (branch->TestBit(kDoNotProcess)) {
                continue;
            }
            TBranchElement *mother = dynamic_cast<TBranchElement*>(leaf->GetBranch()->GetMother());
            if (leaf->GetLeafCount() && (leaf->TestBit(TLeaf::kNewValue) || !leaf->GetValuePointer() || (mother && mother->IsObjectOwner())) && tleaf->GetLeafCount())
            {
                // If it is an array and it was allocated by the leaf itself,
                // let's make sure it is large enough for the incoming data.
                if (leaf->GetLeafCount()->GetMaximum() < tleaf->GetLeafCount()->GetMaximum()) {
                    leaf->GetLeafCount()->IncludeRange( tleaf->GetLeafCount() );
                    if (leaf->GetValuePointer()) {
                        if (leaf->IsA() == TLeafElement::Class() && mother)
                            mother->ResetAddress();
                        else
                            leaf->SetAddress(nullptr);
                    }
                }
            }
            if (!branch->GetAddress() && !leaf->GetValuePointer()) {
                // We should attempts to set the address of the branch.
                // something like:
                //(TBranchElement*)branch->GetMother()->SetAddress(0)
                //plus a few more subtilities (see TBranchElement::GetEntry).
                //but for now we go the simplest route:
                //
                // Note: This may result in the allocation of an object.
                branch->SetupAddresses();
            }
            if (branch->GetAddress()) {
                tree_->SetBranchAddress(branch->GetName(), (void*) branch->GetAddress());
                TBranch* br = tree_->GetBranch(branch->GetName());
                if (br) {
                    // The copy does not own any object allocated by SetAddress().
                    // FIXME: We do too much here, br may not be a top-level branch.
                    if (br->InheritsFrom(TBranchElement::Class())) {
                        ((TBranchElement*) br)->ResetDeleteObject();
                    }
                } //else //would be warning here from TTree::CopyAddresses
            } else {
                tleaf->SetAddress(leaf->GetValuePointer());
            }
        }

        //end copy of TTree::CopyAddresses
        return;
    }

}
