/**
 * @file RandomNumberSeedService.h
 * @brief Conditions object for random number seeds
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_RANDOMNUMBERSEEDSERVICE_H_
#define FRAMEWORK_RANDOMNUMBERSEEDSERVICE_H_

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/ConditionsObject.h"
#include "Framework/ConditionsObjectProvider.h"


namespace ldmx {

/** @class RandomNumberSeedService
 * System for consistent seeding of random number generators
 * 
 * The system can be configured in a number of different ways.  In general, seeds are constructed based on 
 * a master seed.  That master seed can be constructed in a number of ways, chosen in the python configuration.
 *  (a) The master seed can be the run number of the first run observed by the service ("Run")
 *  (b) The master seed can be based on the current time ("Time")
 *  (c) The master seed can be provided in the python configuration ("External")
 * 
 * Individual seeds are then constructed using the master seed and a simple hash based on the name of the seed.
 * Seeds can also be specified in the python file, in which case no autoseeding will be performed.
 */
class RandomNumberSeedService : public ConditionsObject, public ConditionsObjectProvider {
 public:
  /** Conditions object name */
  static constexpr const char* CONDITIONS_OBJECT_NAME="RandomNumberSeedService";
  
  RandomNumberSeedService(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process);

  /** Access a given seed by name */
  uint64_t getSeed(const std::string& name) const;

  /** Get a list of all the known seeds */
  std::vector<std::string> getSeedNames() const;

  /** Access the master seed */
  uint64_t getMasterSeed() const { return masterSeed_; }

  virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const std::string& condition_name, const EventHeader& context);  
  virtual void releaseConditionsObject(const ConditionsObject* co) { } // it us, never destroy it.

 private:
  /** intialized */
  bool initialized_{false};
  /** seeding mode */
  int seedMode_{0};

  /** Master seed */
  uint64_t masterSeed_{0};
  /** Seeds */
  mutable std::map<std::string,uint64_t> seeds_;
 
};

}
  


#endif // FRAMEWORK_RANDOMNUMBERSEEDSERVICE_H_
