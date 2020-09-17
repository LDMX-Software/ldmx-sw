#include "Framework/RandomNumberSeedService.h"
#include "Event/EventHeader.h"
#include <time.h>

namespace ldmx {

static const int SEED_EXTERNAL = 2;
static const int SEED_RUN =  3;
static const int SEED_TIME = 4;

RandomNumberSeedService::RandomNumberSeedService(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) : ConditionsObject(CONDITIONS_OBJECT_NAME), ConditionsObjectProvider(CONDITIONS_OBJECT_NAME,tagname,parameters,process) {
  std::string seeding=parameters.getParameter<std::string>("seedMode","run");
  if (!strcasecmp(seeding.c_str(),"run")) {
    seedMode_=SEED_RUN;
  }
  if (!strcasecmp(seeding.c_str(),"external")) {
    seedMode_=SEED_EXTERNAL;
  }
  if (!strcasecmp(seeding.c_str(),"time")) {
    masterSeed_=time(0);
    seedMode_=SEED_TIME;
  }
  // need to also load any hand-provided seeds...
}

uint64_t RandomNumberSeedService::getSeed(const std::string& name) const {
  uint64_t seed(0);
  std::map<std::string,uint64_t>::const_iterator i=seeds_.find(name);
  if (i==seeds_.end()) {
    // hash is sum of characters shifted by position, mod 8
    for (size_t j=0; j<name.size(); j++) 
      seed+=(uint64_t(name[j])<<(j%8));
    seed+=masterSeed_;
    // break const here only to cache the seed
    seeds_[name]=seed;
  } else seed=i->second;
  return seed;
}

std::vector<std::string> RandomNumberSeedService::getSeedNames() const {
  std::vector<std::string> rv;
  for (auto i: seeds_) {
    rv.push_back(i.first);
  }
  return rv;
}

std::pair<const ConditionsObject*,ConditionsIOV> RandomNumberSeedService::getCondition(const EventHeader& context) {
  if (!initialized_) {
    if (seedMode_==SEED_RUN) {
      masterSeed_=context.getRun();
    }
    initialized_=true;
  }
  return std::pair<const ConditionsObject*,ConditionsIOV>(this,ConditionsIOV(true,true));
}
  
}
DECLARE_CONDITIONS_PROVIDER_NS(ldmx,RandomNumberSeedService)
