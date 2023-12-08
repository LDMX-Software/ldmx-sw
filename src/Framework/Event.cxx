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

/**
 * Construct an actual regex from the pass pattern (and full-string flag)
 *
 * If the pattern is the empty string, then we generate the match-all regex `.*`.
 *
 * If the pattern is not empty and we want to match on full-strings, then
 * we prepend the pattern with `^` and append the pattern with `$` to inform
 * regex that the pattern should match the entire string.
 *
 * @param[in] pattern a regex pattern string
 * @param[in] full_string_match flag if we want full-string matches only (true)
 * or if we can include sub-strings (false)
 * @return generated regex structure, expecting user to call regfree on it when done
 */
static regex_t construct_regex(const std::string& pattern, bool full_string_match) {
  std::string pattern_regex{pattern};
  if (pattern_regex.empty()) pattern_regex = ".*";
  else if (full_string_match) pattern_regex = "^"+pattern_regex+"$";

  regex_t reg;
  if (regcomp(&reg, pattern_regex.c_str(), REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
    // use input value in exception since we expect our code above evolving
    // the regex to be accurate
    EXCEPTION_RAISE(
        "InvalidRegex",
        "The passed regex '"+pattern+"' is not a valid regular expression."
    );
  }
  return reg;
}

std::vector<ProductTag> Event::searchProducts(
    const std::string& namematch, const std::string& passmatch,
    const std::string& typematch, bool full_string_match) const {
  std::vector<ProductTag> retval;
  regex_t reg_name{construct_regex(namematch, full_string_match)},
          reg_pass{construct_regex(passmatch, full_string_match)},
          reg_type{construct_regex(typematch, full_string_match)};

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
  regfree(&reg_pass);
  regfree(&reg_name);

  return retval;
}

bool Event::exists(const std::string &name, const std::string &passName,
    bool unique) const {
  static const bool require_full_string_match = true;
  auto matches = searchProducts(name, passName, "", require_full_string_match);
  if (unique) return (matches.size() == 1);
  else return (matches.size() > 0);
}

TTree* Event::createTree() {
  outputTree_ = new TTree("LDMX_Events", "LDMX Events");

  return outputTree_;
}

void Event::setOutputTree(TTree* tree) { outputTree_ = tree; }

void Event::setInputTree(TTree* tree) {
  inputTree_ = tree;

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

bool Event::nextEvent() {
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
}

bool Event::shouldDrop(const std::string& branchName) const {
  for (const regex_t& exp : regexDropCollections_) {
    if (!regexec(&exp, branchName.c_str(), 0, 0, 0)) return true;
  }
  return false;
}

}  // namespace framework
