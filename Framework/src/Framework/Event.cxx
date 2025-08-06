#include "Framework/Event.h"

#include "TBranchElement.h"

namespace framework {

Event::Event(const std::string& thePassName) : pass_name_(thePassName) {}

Event::~Event() {
  for (regex_t& reg : regex_drop_collections_) {
    regfree(&reg);
  }
}

void Event::print() const {
  for (const ProductTag& tag : getProducts()) {
    std::cout << tag << std::endl;
  }
}

void Event::addDrop(const std::string& exp) {
  regex_t reg;
  if (!regcomp(&reg, exp.c_str(), REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
    regex_drop_collections_.push_back(reg);
  } else {
    EXCEPTION_RAISE("InvalidRegex", "The passed drop rule regex '" + exp +
                                        "' is not a valid regex.");
  }
}

/**
 * Construct an actual regex from the pass pattern (and full-string flag)
 *
 * If the pattern is the empty string, then we generate the match-all regex
 * `.*`.
 *
 * If the pattern is not empty and we want to match on full-strings, then
 * we prepend the pattern with `^` and append the pattern with `$` to inform
 * regex that the pattern should match the entire string.
 *
 * @param[in] pattern a regex pattern string
 * @param[in] full_string_match flag if we want full-string matches only (true)
 * or if we can include sub-strings (false)
 * @return generated regex structure, expecting user to call regfree on it when
 * done
 */
static regex_t constructRegex(const std::string& pattern,
                               bool full_string_match) {
  std::string pattern_regex{pattern};
  if (pattern_regex.empty())
    pattern_regex = ".*";
  else if (full_string_match)
    pattern_regex = "^" + pattern_regex + "$";

  regex_t reg;
  if (regcomp(&reg, pattern_regex.c_str(),
              REG_EXTENDED | REG_ICASE | REG_NOSUB)) {
    // use input value in exception since we expect our code above evolving
    // the regex to be accurate
    EXCEPTION_RAISE("InvalidRegex", "The passed regex '" + pattern +
                                        "' is not a valid regular expression.");
  }
  return reg;
}

std::vector<ProductTag> Event::searchProducts(const std::string& namematch,
                                              const std::string& passmatch,
                                              const std::string& typematch,
                                              bool full_string_match) const {
  std::vector<ProductTag> retval;
  regex_t reg_name{constructRegex(namematch, full_string_match)},
      reg_pass{constructRegex(passmatch, full_string_match)},
      reg_type{constructRegex(typematch, full_string_match)};

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

bool Event::exists(const std::string& name, const std::string& passName,
                   bool unique) const {
  static const bool require_full_string_match = true;
  auto matches = searchProducts(name, passName, "", require_full_string_match);
  if (unique)
    return (matches.size() == 1);
  else
    return (matches.size() > 0);
}

TTree* Event::createTree() {
  output_tree_ = new TTree("LDMX_Events", "LDMX Events");

  return output_tree_;
}

void Event::setOutputTree(TTree* tree) { output_tree_ = tree; }

void Event::setInputTree(TTree* tree) {
  input_tree_ = tree;

  // in some cases, setInputTree is called more than once,
  // so reset branch listing before starting
  products_.clear();
  known_lookups_.clear();  // reset caching of empty pass requests
  bus_.everybodyOff();

  // put in EventHeader (only one without pass name)
  products_.emplace_back(ldmx::EventHeader::BRANCH, "", "ldmx::EventHeader");

  // find the names of all the existing branches
  TObjArray* branches = input_tree_->GetListOfBranches();
  if (!branches) {
    EXCEPTION_RAISE(
        "BadInputFile",
        "Input tree doesn't have a list of branches but still made it to this "
        "point. This is bad and has never been seen before!");
    return;
  }
  for (int i = 0; i < branches->GetEntriesFast(); i++) {
    std::string brname;
    if (branches->At(i)) {
      brname = branches->At(i)->GetName();
    }
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
  event_header_ = getObject<ldmx::EventHeader>(ldmx::EventHeader::BRANCH, "");
  return true;
}

void Event::beforeFill() {
  if (input_tree_ == 0 && branches_filled_.find(ldmx::EventHeader::BRANCH) ==
                             branches_filled_.end()) {
    // Event Header not copied from input and hasn't been added yet, need to put
    // it in
    add(ldmx::EventHeader::BRANCH, event_header_);
  }
}

void Event::clear() {
  branches_filled_.clear();  // forget names of branches we filled
  bus_.clear();  // clear the event objects individually but leave them on bus
}

void Event::onEndOfEvent() {}

void Event::onEndOfFile() {
  if (output_tree_)
    output_tree_->ResetBranchAddresses();  // reset addresses for output branch
  if (input_tree_)
    input_tree_ = nullptr;  // detach old input_tree (owned by EventFile)
  known_lookups_.clear();   // reset caching of empty pass requests
  bus_.everybodyOff();     // delete buffer objects
}

bool Event::shouldDrop(const std::string& branch_name) const {
  for (const regex_t& exp : regex_drop_collections_) {
    if (!regexec(&exp, branch_name.c_str(), 0, 0, 0)) return true;
  }
  return false;
}

}  // namespace framework
