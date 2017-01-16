#include "TTree.h"
#include "Framework/EventImpl.h"
#include "Framework/Exception.h"
#include <iostream>
#include "TBranchElement.h"
#include "TBranchClones.h"

namespace ldmxsw {

EventImpl::EventImpl(const std::string& thePassName) :
        passName_(thePassName) {
}

EventImpl::~EventImpl() {
    for (auto& x : objectsOwned_) {
        delete x.second;
    }
}

void EventImpl::add(const std::string& collectionName, TClonesArray* tca) {

  if (collectionName.find('_')!=std::string::npos) {
    EXCEPTION_RAISE("IllegalName","The product name '"+collectionName+"' is illegal as it contains an underscore.");
  }
  
    std::string branchName = makeBranchName(collectionName);

    std::map<std::string, TObject*>::iterator ito = objects_.find(branchName);
    if (ito == objects_.end()) { // create a new branch
        ito = objects_.insert(std::pair<std::string, TObject*>(branchName, tca)).first;
        if (outputTree_ != 0) {
            TBranch* aBranch = outputTree_->Branch(branchName.c_str(), tca, 100000, 3);
            newBranches_.push_back(aBranch);
        }
	branchNames_.push_back(branchName);
	knownLookups_.clear(); // have to invalidate this cache
    }
}

void EventImpl::add(const std::string& collectionName, TObject* to) {

  if (collectionName.find('_')!=std::string::npos) {
    EXCEPTION_RAISE("IllegalName","The product name '"+collectionName+"' is illegal as it contains an underscore.");
  }
  
    std::string branchName;
    if (collectionName=="EventHeader") branchName=collectionName;
    else branchName = makeBranchName(collectionName);

    std::map<std::string, TObject*>::iterator ito = objects_.find(branchName);

    if (ito == objects_.end()) { // create a new branch
        TObject* myCopy = to->Clone();
        ito = objects_.insert(std::pair<std::string, TObject*>(branchName, myCopy)).first;
        objectsOwned_.insert(std::pair<std::string, TObject*>(branchName, myCopy));
        if (outputTree_ != 0) {
            TBranch* aBranch = outputTree_->Branch(branchName.c_str(), myCopy);
            newBranches_.push_back(aBranch);
        }
	branchNames_.push_back(branchName);
	knownLookups_.clear(); // have to invalidate this cache
    }
    to->Copy(*ito->second);
}

  const TObject* EventImpl::getReal(const std::string& collectionName, const std::string& passName, bool mustExist) {

  std::string branchName;
  if (collectionName=="EventHeader") branchName=collectionName;
  else branchName = makeBranchName(collectionName, passName);

  if (passName.empty()) {
    auto ptr=knownLookups_.find(collectionName);
    if (ptr!=knownLookups_.end()) branchName=ptr->second;
    else {
      std::vector<std::vector<std::string>::const_iterator> matches;
      branchName=collectionName+"_";
      for (std::vector<std::string>::const_iterator ptr=branchNames_.begin(); ptr!=branchNames_.end(); ptr++) {
	if (!ptr->compare(0,branchName.size(),branchName)) matches.push_back(ptr);
      }
      if (matches.empty()) {
	if (!mustExist) return nullptr;
	EXCEPTION_RAISE("ProductNotFound","No product found for name '"+collectionName+"'");
      } else if (matches.size()>1) {
	std::string names;
	for (auto strs : matches) {
	  if (!names.empty()) names+=", ";
	  names+=*strs;
	}
	if (!mustExist) return nullptr;
	EXCEPTION_RAISE("ProductAmbiguous","Multiple products found for name '"+collectionName+"' without specified pass name ("+names+")");
      } else {
	branchName=*matches.front();
	knownLookups_[collectionName]=branchName;
      }
    }
  }

  
    if (inputTree_ == 0) {
        // check the objects map
        std::map<std::string, TObject*>::const_iterator ito = objects_.find(branchName);
        if (ito != objects_.end())
            return ito->second;
        else {
	  EXCEPTION_RAISE("ProductNotFound","No product found for name '"+collectionName+"' and pass '"+passName_+"'");
	  return 0;
	}
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
        TBranch* branch = inputTree_->GetBranch(branchName.c_str());
        if (branch == 0) {
	  EXCEPTION_RAISE("ProductNotFound","No product found for name '"+collectionName+"' and pass '"+passName_+"'");
	}

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
    outputTree_ = new TTree("LDMX_Events", "LDMX Events");

    eventHeader_=new event::EventHeader();
    add("EventHeader",eventHeader_);
    
    return outputTree_;
}

void EventImpl::setOutputTree(TTree* tree) {
    outputTree_ = tree;
}

void EventImpl::setInputTree(TTree* tree) {
    inputTree_ = tree;
    entries_ = inputTree_->GetEntries();
    eventHeader_=get<event::EventHeader*>("EventHeader");

    // find the names of all the existing branches
    TObjArray* branches=inputTree_->GetListOfBranches();
    for (int i=0; i<branches->GetEntriesFast(); i++) {
      branchNames_.push_back(branches->At(i)->GetName());
    }
}

bool EventImpl::nextEvent() {
    ientry_++;
    return true;
}

  void EventImpl::beforeFill() {
    if (inputTree_ == 0) add("EventHeader",eventHeader_);
  }
  void EventImpl::Clear() {
    // clear the event objects
    for (auto obj : objects_)
      obj.second->Clear("C");
  }
  void EventImpl::onEndOfEvent() {
  }
  
void EventImpl::onEndOfFile() {
}

}

