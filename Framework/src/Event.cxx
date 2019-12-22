#include "Framework/Event.h"

namespace ldmx {

    Event::Event(const std::string& thePassName) :
        passName_(thePassName) {
    }

    Event::~Event() { }

    void Event::Print(int verbosity) const {
        for ( const auto &keyVal : passengers_ ) {
            if ( verbosity > 0 ) std::cout << keyVal.first << std::endl;
            boost::apply_visitor( printPassenger(verbosity) , keyVal.second );
        }
    }

    std::vector<ProductTag> Event::searchProducts(
            const std::string& namematch, const std::string& passmatch, const std::string& typematch) const {
        std::vector<ProductTag> retval;

        regex_t reg_name, reg_pass, reg_type;
        char errbuf[1000];
        int rv;
  
        if (!regcomp(&reg_name,(namematch.empty()?(".*"):(namematch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
            if (!regcomp(&reg_pass,(passmatch.empty()?(".*"):(passmatch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
  	            if (!regcomp(&reg_type,(typematch.empty()?(".*"):(typematch.c_str())),REG_EXTENDED|REG_ICASE|REG_NOSUB)) {
                    //all passed expressions are valid regular expressions 
  	                const std::vector<ProductTag>& products=getProducts();
  	                for (std::vector<ProductTag>::const_iterator i=products.begin(); i!=products.end(); i++) {
  	                    if (!regexec(&reg_name,i->name().c_str(),0,0,0) &&
  		                    !regexec(&reg_pass,i->passname().c_str(),0,0,0) &&
  		                    !regexec(&reg_type,i->type().c_str(),0,0,0))
  	                        retval.push_back(*i);
  	                } 
  	  
  	                regfree(&reg_type);
  	            } else {
                    EXCEPTION_RAISE(
                            "InvalidRegex",
                            "The passed type regex '"
                            + typematch
                            + "' is not a valid regular expression"
                            );
                }
  	            regfree(&reg_pass);
            } else {
                EXCEPTION_RAISE(
                        "InvalidRegex",
                        "The passed name regex '"
                        + namematch
                        + "' is not a valid regular expression"
                        );
            }
            regfree(&reg_name);
        } else {
            EXCEPTION_RAISE(
                    "InvalidRegex",
                    "The passed pass regex '"
                    + passmatch
                    + "' is not a valid regular expression"
                    );
        }

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

        // in some cases, setInputTree is called more than once,
        // so reset branch listing before starting
        products_.clear();
        branchNames_.clear();

        // put in EventHeader (only one without pass name)
	    products_.emplace_back(EventConstants::EVENT_HEADER,"","ldmx::EventHeader");
	
        // find the names of all the existing branches
        TObjArray* branches = inputTree_->GetListOfBranches();
        for (int i = 0; i < branches->GetEntriesFast(); i++) {
    	    std::string brname=branches->At(i)->GetName();
    	    if (brname!=EventConstants::EVENT_HEADER) {
        		size_t j=brname.find("_");
        		std::string iname=brname.substr(0,j);
        		std::string pname=brname.substr(j+1);
                std::string tname = "";
                TBranchElement *tbe = dynamic_cast<TBranchElement*>(branches->At(i));
                if (tbe) tname = tbe->GetClassName();
                else tname = branches->At(i)->ClassName();
    		    products_.emplace_back(iname,pname,tname);
    	    }
            branchNames_.push_back(brname);
        }
    }

    bool Event::nextEvent() {
        ientry_++;
        eventHeader_ = getObject<EventHeader>(EventConstants::EVENT_HEADER);
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
        for ( auto passenger : passengers_ ) {
            boost::apply_visitor( clearPassenger() , passenger.second );
        }
    }
    void Event::onEndOfEvent() {
    }

    void Event::onEndOfFile() {
    }

}

