#include "Framework/Event.h"

#include "TBranchElement.h"

namespace framework {

Event::Event(const std::string& thePassName) : passName_(thePassName) {}

Event::~Event() {
  for (regex_t& reg : regexDropCollections_) {
    regfree(&reg);
  }
}

void Event::Print() const {
  for (const ProductTag& tag : getProducts()) {
    std::cout << tag << std::endl;
  }
}

void Event::addDrop(const std::string& exp) {
  regex_t reg;
  if (!regcomp(&reg, exp.c_str(), REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
    regexDropCollections_.push_back(reg);
  } else {
    EXCEPTION_RAISE("InvalidRegex", "The passed drop rule regex '" + exp +
                                        "' is not a valid regex.");
  }
}

std::vector<ProductTag> Event::searchProducts(
    const std::string& namematch, const std::string& passmatch,
    const std::string& typematch) const {
  std::vector<ProductTag> retval;

  regex_t reg_name, reg_pass, reg_type;

  if (!regcomp(&reg_name, (namematch.empty() ? (".*") : (namematch.c_str())),
               REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
    if (!regcomp(&reg_pass, (passmatch.empty() ? (".*") : (passmatch.c_str())),
                 REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
      if (!regcomp(&reg_type,
                   (typematch.empty() ? (".*") : (typematch.c_str())),
                   REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
        // all passed expressions are valid regular expressions
        const std::vector<ProductTag>& products = getProducts();
        for (std::vector<ProductTag>::const_iterator i = products.begin();
             i != products.end(); i++) {
          if (!regexec(&reg_name, i->name().c_str(), 0, 0, 0) &&
              !regexec(&reg_pass, i->passname().c_str(), 0, 0, 0) &&
              !regexec(&reg_type, i->type().c_str(), 0, 0, 0))
            retval.push_back(*i);
        }

        regfree(&reg_type);
      } else {
        EXCEPTION_RAISE("InvalidRegex",
                        "The passed type regex '" + typematch +
                            "' is not a valid regular expression");
      }
      regfree(&reg_pass);
    } else {
      EXCEPTION_RAISE("InvalidRegex",
                      "The passed name regex '" + namematch +
                          "' is not a valid regular expression");
    }
    regfree(&reg_name);
  } else {
    EXCEPTION_RAISE("RegexErr", "The passed passname regex '" + passmatch +
                                    "' is not a valid regular expression.");
  }

  return retval;
}

TTree* Event::createTree() {
  outputTree_ = new TTree("LDMX_Events", "LDMX Events");

  return outputTree_;
}

void Event::setOutputTree(TTree* tree) { outputTree_ = tree; }

void Event::setInputTree(TTree* tree) {
  inputTree_ = tree;
  entries_ = inputTree_->GetEntriesFast();

  // in some cases, setInputTree is called more than once,
  // so reset branch listing before starting
  products_.clear();
  knownLookups_.clear(); //reset caching of empty pass requests
  bus_.everybodyOff();

  // put in EventHeader (only one without pass name)
  products_.emplace_back(ldmx::EventHeader::BRANCH, "", "ldmx::EventHeader");

  // find the names of all the existing branches
  TObjArray* branches = inputTree_->GetListOfBranches();
  for (int i = 0; i < branches->GetEntriesFast(); i++) {
    std::string brname = branches->At(i)->GetName();
    if (brname != ldmx::EventHeader::BRANCH) {
      size_t j = brname.find("_");
      auto br = dynamic_cast<TBranchElement*>(branches->At(i));
      // can't determine type if branch isn't
      //  the higher-level TBranchElement type
      // Only occurs if the type on the bus is one of:
      //  bool, short, int, long, float, double (BSILFD)
      products_.emplace_back(
          brname.substr(0, j),   // collection name is before '_'
          brname.substr(j + 1),  // pass name is after
          br ? br->GetClassName() : "BSILFD");
    }
  }
}

bool Event::nextEvent(int ientry) {
  ientry_ = ientry;
  eventHeader_ = getObject<ldmx::EventHeader>(ldmx::EventHeader::BRANCH);
  return true;
}

void Event::beforeFill() {
  if (inputTree_ == 0 && branchesFilled_.find(ldmx::EventHeader::BRANCH) ==
                             branchesFilled_.end()) {
    // Event Header not copied from input and hasn't been added yet, need to put
    // it in
    add(ldmx::EventHeader::BRANCH, eventHeader_);
  }
}

void Event::Clear() {
  branchesFilled_.clear(); //forget names of branches we filled
  bus_.clear(); //clear the event objects individually but leave them on bus
}

void Event::onEndOfEvent() {}

void Event::onEndOfFile() {
  if (outputTree_)
    outputTree_->ResetBranchAddresses();  // reset addresses for output branch
  if (inputTree_)
    inputTree_ = nullptr;  // detach old inputTree (owned by EventFile)
  knownLookups_.clear();   // reset caching of empty pass requests
  bus_.everybodyOff();     // delete buffer objects
  ientry_ = -1;            // reset current entry and total entries
  entries_ = -1;
}

bool Event::shouldDrop(const std::string& branchName) const {
  for (const regex_t& exp : regexDropCollections_) {
    if (!regexec(&exp, branchName.c_str(), 0, 0, 0)) return true;
  }
  return false;
}

}  // namespace framework
