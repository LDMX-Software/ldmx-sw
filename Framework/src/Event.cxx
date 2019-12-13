#include "Framework/Event.h"

namespace ldmx {

    Event::Event(const std::string& thePassName) :
        passName_(thePassName) {
    }

    Event::~Event() { }

    std::vector<ProductTag> Event::searchProducts(
            const std::string& namematch, const std::string& passmatch, const std::string& typematch) const {
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
                //TODO: Improve the type-gettting algorithm
        		std::string tname=branches->At(i)->ClassName();
//        		if (tname=="TBranchElement")
//        		    tname=std::string("TClonesArray(")+((TBranchElement*)(branches->At(i)))->GetClonesName()+")";
    		    products_.push_back(ProductTag(iname,pname,tname));
    	    }
	    
            branchNames_.push_back(brname);
        }
    }

    bool Event::nextEvent() {
        ientry_++;
        eventHeader_ = getImpl<EventHeader>(EventConstants::EVENT_HEADER, "" , true);
        return true;
    }

    void Event::beforeFill() {
        if (inputTree_==0 && branchesFilled_.find(EventConstants::EVENT_HEADER)==branchesFilled_.end()) {
            add(EventConstants::EVENT_HEADER, eventHeader_);
        }
    }

    void Event::Clear() {
        // clear the event objects
        branchesFilled_.clear();

    }
    void Event::onEndOfEvent() {
        branchesFilled_.clear();
    }

    void Event::onEndOfFile() {
    }

}

