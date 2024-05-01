#include "Framework/RandomNumberSeedService.h"

#include <time.h>

#include "Framework/EventHeader.h"
#include "Framework/Process.h"
#include "Framework/RunHeader.h"

namespace framework {

const std::string RandomNumberSeedService::CONDITIONS_OBJECT_NAME =
    "RandomNumberSeedService";

static const int SEED_EXTERNAL = 2;
static const int SEED_RUN = 3;
static const int SEED_TIME = 4;

void RandomNumberSeedService::stream(std::ostream& s) const {
  s << "RandomNumberSeedService(";
  if (seedMode_ == SEED_RUN) s << "Seed on RUN";
  if (seedMode_ == SEED_TIME) s << "Seed on TIME";
  if (seedMode_ == SEED_EXTERNAL) s << "Seeded EXTERNALLY";
  s << ") Master seed = " << masterSeed_ << std::endl;
  for (auto i : seeds_) {
    s << " " << i.first << "=>" << i.second << std::endl;
  }
}

RandomNumberSeedService::RandomNumberSeedService(
    const std::string& name, const std::string& tagname,
    const framework::config::Parameters& parameters, Process& process)
    : ConditionsObject(CONDITIONS_OBJECT_NAME),
      ConditionsObjectProvider(CONDITIONS_OBJECT_NAME, tagname, parameters,
                               process) {
  auto seeding = parameters.getParameter<std::string>("seedMode", "run");
  if (!strcasecmp(seeding.c_str(), "run")) {
    seedMode_ = SEED_RUN;
  } else if (!strcasecmp(seeding.c_str(), "external")) {
    seedMode_ = SEED_EXTERNAL;
    masterSeed_ = parameters.getParameter<int>("masterSeed");
    initialized_ = true;
  } else if (!strcasecmp(seeding.c_str(), "time")) {
    masterSeed_ = time(0);
    seedMode_ = SEED_TIME;
    initialized_ = true;
  }
}

void RandomNumberSeedService::onNewRun(ldmx::RunHeader& rh) {
  if (seedMode_ == SEED_RUN) {
    masterSeed_ = rh.getRunNumber();
    initialized_ = true;
  }
  std::string key = "RandomNumberMasterSeed[" + process().getPassName() + "]";
  rh.setIntParameter(key, int(masterSeed_));
}

uint64_t RandomNumberSeedService::getSeed(const std::string& name) const {
  uint64_t seed(0);
  std::map<std::string, uint64_t>::const_iterator i = seeds_.find(name);
  if (i == seeds_.end()) {
    // hash is sum of characters shifted by position, mod 8
    for (size_t j = 0; j < name.size(); j++)
      seed += (uint64_t(name[j]) << (j % 8));
    seed += masterSeed_;
    // break const here only to cache the seed
    seeds_[name] = seed;
  } else
    seed = i->second;
  return seed;
}

std::vector<std::string> RandomNumberSeedService::getSeedNames() const {
  std::vector<std::string> rv;
  for (auto i : seeds_) {
    rv.push_back(i.first);
  }
  return rv;
}

std::pair<const ConditionsObject*, ConditionsIOV>
RandomNumberSeedService::getCondition(const ldmx::EventHeader& context) {
  if (!initialized_) {
    if (seedMode_ == SEED_RUN) {
      masterSeed_ = context.getRun();
    }
    initialized_ = true;
  }
  return std::pair<const ConditionsObject*, ConditionsIOV>(
      this, ConditionsIOV(true, true));
}

}  // namespace framework
DECLARE_CONDITIONS_PROVIDER_NS(framework, RandomNumberSeedService)
