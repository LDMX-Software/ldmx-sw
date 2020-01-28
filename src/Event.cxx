#include "Framework/Event.h"

namespace ldmx {

    Event::Event(const std::string& thePassName) : passName_(thePassName) { }

    Event::~Event() { }

    void Event::Print(int verbosity) const {
        for ( const auto &keyVal : passengers_ ) {
            if ( verbosity > 1 ) std::cout << keyVal.first << std::endl;
            std::visit( printPassenger(verbosity) , keyVal.second );
        }
    }

    void Event::addDrop( const std::string &exp ) {
        try {
            regexDropCollections_.emplace_back( exp );
        } catch ( std::regex_error & ) {
            EXCEPTION_RAISE(
                    "RegexErr",
                    "The drop rule expression '"
                    + exp
                    + "' is not a valid regular expression."
                    );
        }
    }

    std::vector<ProductTag> Event::searchProducts( const std::string& namematch, const std::string& passmatch, const std::string& typematch) const {
        std::vector<ProductTag> retval;

        std::regex reg_name(namematch.empty()?".*":namematch);
        std::regex reg_pass(passmatch.empty()?".*":passmatch);
        std::regex reg_type(typematch.empty()?".*":typematch);
  
        //all passed expressions are valid regular expressions 
        try {
      	    const std::vector<ProductTag>& products=getProducts();
      	    for ( const ProductTag &pt : products ) {
      	       if ( std::regex_match( pt.name() , reg_name ) and
                    std::regex_match( pt.passname() , reg_pass ) and
                    std::regex_match( pt.type() , reg_type )
                  ) retval.push_back(pt);
      	    } 
        } catch ( std::regex_error & ) {
            EXCEPTION_RAISE(
                    "RegexErr",
                    "One of '"
                    + namematch
                    + "', '"
                    + passmatch
                    + "', or '"
                    + typematch
                    + "' is non-empty and not a valid regular expression."
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
                std::string tname = dynamic_cast<TBranchElement*>(branches->At(i))->GetClassName();
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
            //Event Header not copied from input and hasn't been added yet, need to put it in
            add(EventConstants::EVENT_HEADER, eventHeader_);
        }
    }

    void Event::Clear() {
        // clear the event objects
        branchesFilled_.clear();
        for ( auto passenger : passengers_ ) {
            std::visit( clearPassenger() , passenger.second );
        }
    }

    void Event::onEndOfEvent() { }

    void Event::onEndOfFile() {
        passengers_.clear(); //reset event bus
        branches_.clear(); //reset branches
        outputTree_->ResetBranchAddresses(); //reset addresses for output branch
    }

    bool Event::shouldDrop(const std::string &collName) const {
        for ( const std::regex &exp : regexDropCollections_ ) {
            if ( std::regex_match( collName , exp ) ) return true;
        }
        return false;
    }

}

