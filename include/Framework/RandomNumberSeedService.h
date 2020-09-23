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

/** 
 * @class RandomNumberSeedService
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
  /// Conditions object name
  static const std::string CONDITIONS_OBJECT_NAME;
  
  /**
   * Create the random number seed service conditions object
   *
   * This is where we decide what seed mode we will be running in.
   *
   * @TODO allow loading of hand-provided seeds from python
   *
   * @param[in] name the name of this provider
   * @param[in] tagname the name of the tag generation of this condition
   * @param[in] parameters configuration parameters from python
   * @param[in] process reference to the running process object
   */
  RandomNumberSeedService(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process);

  /**
   * Configure the seed service when a new run starts
   *
   * If we are using the run number as the master seed,
   * then we get the run number and set the master seed to it.
   * 
   * No matter what, we put the master seed into the RunHeader
   * to be persisted into the output file.
   *
   * @param[in,out] header RunHeader for the new run that is starting
   */
  virtual void onNewRun(RunHeader& header);

  /** 
   * Access a given seed by name 
   *
   * Checks the cache for the input name.
   * If the input name is not in the cache,
   * it generates the seed for that name by combining
   * the master seed with a hash of the name.
   *
   * @param[in] name Name of seed
   * @return seed derived from master seed using the input name
   */
  uint64_t getSeed(const std::string& name) const;

  /** 
   * Get a list of all the known seeds 
   *
   * @returns vector of seed names that have already been requested (in cache)
   */
  std::vector<std::string> getSeedNames() const;

  /** 
   * Access the master seed 
   *
   * @returns master seed that is used to derive all other seeds
   */
  uint64_t getMasterSeed() const { return masterSeed_; }

  /**
   * Get the seed service as a conditions object
   *
   * This object is both the provider of the seed service and the conditions object itself,
   * so after checking if it has been initialized, we return a refernce to ourselves
   * with the unlimited interval of validity.
   *
   * @param[in] context EventHeader for the event context
   * @returns reference to ourselves and unlimited interval of validity
   */
  virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const EventHeader& context);  

  /**
   * This object is both the provider of the seed service and the conditions object
   * itself, so it does nothing when asked to release the object.
   *
   * @param[in] co ConditionsObject to release, unused
   */
  virtual void releaseConditionsObject(const ConditionsObject* co) { } // it is us, never destroy it.
  
  /**
   * Stream the configuration of this object to the input ostream
   *
   * @param[in,out] s output stream to print this object to
   */
  void stream(std::ostream& s) const;

 private:

  /// whether the master seed has been initialized
  bool initialized_{false};

  /// what mode of master seed are we using
  int seedMode_{0};

  /// what the master seed actually is
  uint64_t masterSeed_{0};

  /// cache of seeds by name
  mutable std::map<std::string,uint64_t> seeds_;
 
};

}

/**
 * Output streaming operator
 *
 * @see ldmx::RandomNumberSeedService::stream
 * @param[in] s output stream to print to
 * @param[in] o seed service to print
 * @returns modified output stream
 */
std::ostream& operator<<(std::ostream& s, const ldmx::RandomNumberSeedService& o);

#endif // FRAMEWORK_RANDOMNUMBERSEEDSERVICE_H_
