#include "TTree.h"
#include "Event/EventImpl.h"
#include <iostream>
#include "TBranchElement.h"
#include "TBranchClones.h"

namespace event {

const char* EventImpl::TREE_NAME{"LDMX_Events"};

EventImpl::EventImpl(const std::string& thePassName) :
        passName_(thePassName) {
}

EventImpl::~EventImpl() {
    for (auto& x : objectsOwned_) {
        delete x.second;
    }
}

void EventImpl::add(const std::string& collectionName, TClonesArray* tca) {

    std::string branchName = makeBranchName(collectionName);

    std::map<std::string, TObject*>::iterator ito = objects_.find(branchName);
    if (ito == objects_.end()) { // create a new branch
        ito = objects_.insert(std::pair<std::string, TObject*>(branchName, tca)).first;
        if (otree_ != 0) {
            TBranch* aBranch = otree_->Branch(branchName.c_str(), tca, 100000, 3);
            newBranches_.push_back(aBranch);
        }
    }
}

void EventImpl::add(const std::string& collectionName, TObject* to) {

    std::string branchName = makeBranchName(collectionName);

    std::map<std::string, TObject*>::iterator ito = objects_.find(branchName);

    if (ito == objects_.end()) { // create a new branch
        TObject* myCopy = to->Clone();
        ito = objects_.insert(std::pair<std::string, TObject*>(branchName, myCopy)).first;
        objectsOwned_.insert(std::pair<std::string, TObject*>(branchName, myCopy));
        if (otree_ != 0) {
            TBranch* aBranch = otree_->Branch(branchName.c_str(), myCopy);
            newBranches_.push_back(aBranch);
        }
    }
    to->Copy(*ito->second);
}

const TObject* EventImpl::get(const std::string& collectionName, const std::string& passName) {

    std::string branchName = makeBranchName(collectionName, passName);

    if (passName.empty()) {
        // TODO: logic to determine if there is one-and-only-one collection which matches this name
        //      assert(!passName.empty());
    }

    if (itree_ == 0) {
        // check the objects map
        std::map<std::string, TObject*>::const_iterator ito = objects_.find(branchName);
        if (ito != objects_.end())
            return ito->second;
        else
            return 0;
    }

    // find the active branch and update if necessary
    std::map<std::string, TBranch*>::const_iterator itb = branches_.find(branchName);
    if (itb != branches_.end()) {

        std::map<std::string, TObject*>::iterator ito = objects_.find(branchName);

        // update buffers if needed
        if (itb->second->GetReadEntry() != ientry_) {

            TBranchElement* tbe = dynamic_cast<TBranchElement*>(itb->second);
            if (!tbe)
                itb->second->SetAddress(ito->second);
            int nr = itb->second->GetEntry(ientry_, 1);
        }

        // check the objects map

        if (ito != objects_.end())
            return ito->second;

        // this case is hard to achieve
        return 0;
    } else {

        // ok, maybe we've not loaded this yet, look for a branch
        TBranch* branch = itree_->GetBranch(branchName.c_str());
        if (branch == 0)
            return 0;

        // ooh, new branch!
        TObject* top(0);
        branch->SetAutoDelete(false);
        branch->SetStatus(1);
        branch->GetEntry(ientry_);
        TBranchElement* tbe = dynamic_cast<TBranchElement*>(branch);
        if (tbe) {
            top = (TObject*) tbe->GetObject();
        } else {
            branch->SetAddress(&top);
        }

        branches_.insert(std::pair<std::string, TBranch*>(branchName, branch));
        objects_.insert(std::pair<std::string, TObject*>(branchName, top));

        return top;
    }
}

TTree* EventImpl::createTree() {
    otree_ = new TTree("LDMX_Events", "LDMX Events");
    // eventually, add branch for event info
    return otree_;
}

void EventImpl::setOTree(TTree* tree) {
    otree_ = tree;
}

void EventImpl::setITree(TTree* tree) {
    itree_ = tree;
    entries_ = itree_->GetEntries();
}

bool EventImpl::nextEvent() {
    ientry_++;
    return true;
}

void EventImpl::onEndOfEvent() {
    // clear the event objects
    for (auto obj : objectsOwned_)
        obj.second->Clear();
}

void EventImpl::onEndOfFile() {
}

}

