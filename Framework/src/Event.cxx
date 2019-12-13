#include "Framework/Event.h"

namespace ldmx {

    Event::Event(const std::string& thePassName) :
        passName_(thePassName) {
    }

    Event::~Event() {
        for (auto& x : objectsOwned_) {
            delete x.second;
        }
    }

    void Event::add(const std::string& collectionName, TClonesArray* tca) {

        if (collectionName.find('_') != std::string::npos) {
            EXCEPTION_RAISE("IllegalName", "The product name '" + collectionName + "' is illegal as it contains an underscore.");
        }

        std::string branchName = makeBranchName(collectionName);

        if (branchesFilled_.find(branchName) != branchesFilled_.end()) {
            EXCEPTION_RAISE("ProductExists", "A product named '" + collectionName + "' already exists in the event (has been loaded by a previous producer in this process.");
        }
        branchesFilled_.insert(branchName);

        std::map<std::string, TObject*>::iterator ito = objects_.find(branchName);
        if (ito == objects_.end()) { // create a new branch
            ito = objects_.insert(std::pair<std::string, TObject*>(branchName, tca)).first;
            if (outputTree_ != 0) {
                TBranch *outBranch = outputTree_->GetBranch( branchName.c_str() );
                if ( outBranch ) {
                    //branch already exists, just reset branch address
                    outBranch->SetAddress( &tca );
                } else {
                    //branch doesnt exist, make new one
                    outBranch = outputTree_->Branch(branchName.c_str(), tca, 100000, 3);
                }
                newBranches_.push_back(outBranch);
            }
	    std::string tcaContains(tca->GetName());
	    if (!tcaContains.empty()) tcaContains.pop_back();
	    products_.push_back(ProductTag(collectionName,passName_,"TClonesArray("+tcaContains+")"));
            branchNames_.push_back(branchName);
            knownLookups_.clear(); // have to invalidate this cache
        }
    }

    void Event::add(const std::string& collectionName, TObject* to) {

        if (collectionName.find('_')!=std::string::npos) {
            EXCEPTION_RAISE("IllegalName","The product name '"+collectionName+"' is illegal as it contains an underscore.");
        }

        std::string branchName;
        if (collectionName==EventConstants::EVENT_HEADER) branchName=collectionName;
        else branchName = makeBranchName(collectionName);

        if (branchesFilled_.find(branchName)!=branchesFilled_.end()) {
            EXCEPTION_RAISE("ProductExists","A product named '"+collectionName+"' already exists in the event (has been loaded by a previous producer in this process.");
        }
        branchesFilled_.insert(branchName);

        std::map<std::string, TObject*>::iterator ito = objects_.find(branchName);

        if (ito == objects_.end()) { // create a new branch
            TObject* myCopy = to->Clone();
            ito = objects_.insert(std::pair<std::string, TObject*>(branchName, myCopy)).first;
            objectsOwned_.insert(std::pair<std::string, TObject*>(branchName, myCopy));
            if (outputTree_ != 0) {
                //outputTree_ exists
                TBranch* outBranch = outputTree_->GetBranch( branchName.c_str() );
                if ( outBranch ) {
                    //branch already exists on output Tree
                    outBranch->SetAddress( &to );
                } else {
                    //branch doesn't exist on tree yet
                    outBranch = outputTree_->Branch(branchName.c_str(), myCopy);
                }
                newBranches_.push_back(outBranch);
            }
	    products_.push_back(ProductTag(collectionName,passName_,to->Class()->GetName()));
            branchNames_.push_back(branchName);
            knownLookups_.clear(); // have to invalidate this cache
        }
        to->Copy(*ito->second);
    }


    void Event::addToCollection(const std::string& name, const TObject& obj) {
        std::string branchName;
        if (name == EventConstants::EVENT_HEADER) return; // no adding to the event header...
        branchName = makeBranchName(name);

        auto location = objectsOwned_.find(branchName);
        if (location == objectsOwned_.end()) {
            //TCA not found in existing objects
            TClonesArray* tca = new TClonesArray(obj.ClassName(), 100);
            objectsOwned_[branchName] = tca;
            add(name, tca);
            //copy object into TCA
            TObject* to = tca->ConstructedAt(0);
            obj.Copy(*to);
        } else {
            TClonesArray* ptca = dynamic_cast<TClonesArray*>(location->second);
            if (ptca == 0) {
                EXCEPTION_RAISE("ProductProblem", "Attempted to add to the collection '" + name + "' which is not a TClonesArray.");
            }
            if (ptca->At(0)->Class() != obj.Class()) {
                EXCEPTION_RAISE("ProductProblem", "Attempted to add object of different class to the collection '" + name + "'");
            }
            TObject* to = ptca->ConstructedAt(ptca->GetEntriesFast());
            obj.Copy(*to);
        }
    }

    const TObject* Event::getReal(const std::string& collectionName, const std::string& passName, bool mustExist) const {

        std::string branchName;
        if (collectionName== EventConstants::EVENT_HEADER) branchName=collectionName;
        else branchName = makeBranchName(collectionName, passName);

        if (passName.empty() && collectionName!= EventConstants::EVENT_HEADER) {
            auto ptr=knownLookups_.find(collectionName);
            if (ptr!=knownLookups_.end()) branchName=ptr->second;
            else {
                std::vector<std::vector<std::string>::const_iterator> matches;
                branchName=collectionName+"_";
                for (std::vector<std::string>::const_iterator ptr=branchNames_.begin(); ptr!=branchNames_.end(); ptr++) {
                    if (!ptr->compare(0,branchName.size(),branchName)) matches.push_back(ptr);
                }
                if (matches.empty()) {
                    if (!mustExist)
                        return nullptr;
                    EXCEPTION_RAISE("ProductNotFound","No product found for name '"+collectionName+"'");
                } else if (matches.size()>1) {
                    std::string names;
                    for (auto strs : matches) {
                        if (!names.empty()) names+=", ";
                        names+=*strs;
                    }
                    if (!mustExist)
                        return nullptr;
                    EXCEPTION_RAISE("ProductAmbiguous","Multiple products found for name '"+collectionName+"' without specified pass name ("+names+")");
                } else {
                    branchName=*matches.front();
                    knownLookups_[collectionName]=branchName;
                }
            }
        }


        std::map<std::string, TBranch*>::const_iterator itb = branches_.find(branchName);

        // check the objects map
        std::map<std::string, TObject*>::const_iterator ito = objects_.find(branchName);
        if (ito != objects_.end()) {
            if (itb!=branches_.end())
                itb->second->GetEntry(ientry_);
            return ito->second;
        } else if (inputTree_ == 0) {
            EXCEPTION_RAISE("ProductNotFound", "No product found for name '" + collectionName + "' and pass '" + passName_ + "'");
        }

        // find the active branch and update if necessary
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
                EXCEPTION_RAISE("ProductNotFound", "No product found for name '" + collectionName + "' and pass '" + passName_ + "'");
            }
            // ooh, new branch!
            TObject* top(0);
            branch->SetAutoDelete(false);
            branch->SetStatus(1);
            branch->GetEntry((ientry_<0)?(0):(ientry_));
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

    std::vector<ProductTag> Event::searchProducts(const std::string& namematch, const std::string& passmatch, const std::string& typematch) const {
        std::vector<ProductTag> retval;

        regex_t reg_name, reg_pass, reg_type;
        char errbuf[1000];
        int rv;
  
        if (!regcomp(&reg_name,(namematch.empty()?(".*"):(namematch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
            if (!regcomp(&reg_pass,(passmatch.empty()?(".*"):(passmatch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
  	            if (!regcomp(&reg_type,(typematch.empty()?(".*"):(typematch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
  
  	                const std::vector<ProductTag>& products=getProducts();
  	                for (std::vector<ProductTag>::const_iterator i=products.begin(); i!=products.end(); i++) {
  	                    if (!regexec(&reg_name,i->name().c_str(),0,0,0) &&
  		                    !regexec(&reg_pass,i->passname().c_str(),0,0,0) &&
  		                    !regexec(&reg_type,i->type().c_str(),0,0,0))
  	                        retval.push_back(*i);
  	                } 
  	  
  	                regfree(&reg_type);
  	            } else { std::cout << "E3\n"; }
  	            regfree(&reg_pass);
            } else { std::cout << "E2\n"; }  
            regfree(&reg_name);
        } else { std::cout << "E1\n"; }

        return retval;
    }

    TTree* Event::createTree() {
        outputTree_ = new TTree("LDMX_Events", "LDMX Events");

        eventHeader_ = new EventHeader();

        return outputTree_;
    }

    void Event::setOutputTree(TTree* tree) {
        outputTree_ = tree;
    }

    void Event::setInputTree(TTree* tree) {
        inputTree_ = tree;
        entries_ = inputTree_->GetEntriesFast();
        branchNames_.clear();


	    products_.push_back(ProductTag(EventConstants::EVENT_HEADER,"","ldmx::EventHeader"));
	
        // find the names of all the existing branches
        TObjArray* branches = inputTree_->GetListOfBranches();
        for (int i = 0; i < branches->GetEntriesFast(); i++) {
    	    std::string brname=branches->At(i)->GetName();
    	    if (brname!=EventConstants::EVENT_HEADER) {
        		size_t j=brname.find("_");
        		std::string iname=brname.substr(0,j);
        		std::string pname=brname.substr(j+1);
        		std::string tname=branches->At(i)->ClassName();
        		if (tname=="TBranchElement")
        		    tname=std::string("TClonesArray(")+((TBranchElement*)(branches->At(i)))->GetClonesName()+")";
    		    products_.push_back(ProductTag(iname,pname,tname));
    	    }
	    
            branchNames_.push_back(brname);
        }
    }

    bool Event::nextEvent() {
        ientry_++;
        eventHeader_=get<EventHeader*>(EventConstants::EVENT_HEADER);
        return true;
    }

    void Event::beforeFill() {
        if (inputTree_==0 && branchesFilled_.find(EventConstants::EVENT_HEADER)==branchesFilled_.end()) {
    //        add(EventConstants::EVENT_HEADER, *eventHeader_);
        }
    }

    void Event::Clear() {
        // clear the event objects
        for (auto obj : objects_)
            obj.second->Clear("C");
        for ( auto coll : collections_ )
            boost::apply_visitor( clearCollection() , coll.second );
        branchesFilled_.clear();

    }
    void Event::onEndOfEvent() {
        branchesFilled_.clear();
    }

    void Event::onEndOfFile() {
    }

}

