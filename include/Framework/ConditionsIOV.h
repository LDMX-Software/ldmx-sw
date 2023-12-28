/**
 * @file ConditionsIOV.h
 * @brief Interval-of-validity object for conditions information
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_CONDITIONSIOV_H_
#define FRAMEWORK_CONDITIONSIOV_H_

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include <iostream>

#include "Framework/Exception/Exception.h"

namespace framework {
class ConditionsIOV;
}

std::ostream& operator<<(std::ostream&, const framework::ConditionsIOV& iov);

namespace ldmx {
class EventHeader;
}
namespace framework {

/**
 * @class ConditionsIOV
 *
 * @brief Class which defines the run/event/type range for which a given
 * condition is valid, including for all time
 */
class ConditionsIOV {
 public:
  /**
   * Constructor for null validity
   */
  ConditionsIOV()
      : firstRun_{0}, lastRun_{0}, validForData_{false}, validForMC_{false} {}

  /**
   * Constructor for a unlimited validity
   */
  ConditionsIOV(bool validForData, bool validForMC)
      : firstRun_(-1),
        lastRun_(-1),
        validForData_{validForData},
        validForMC_{validForMC} {}

  /**
   * Constructor for a run-limited validity
   * @arg firstRun should be -1 if valid from beginning of time
   * @arg lastRun should be -1 if valid to end of time
   */
  ConditionsIOV(int firstRun, int lastRun, bool validForData = true,
                bool validForMC = true)
      : firstRun_(firstRun),
        lastRun_(lastRun),
        validForData_{validForData},
        validForMC_{validForMC} {}

  /** Checks to see if this condition is valid for the given event using
   * information from the header */
  bool validForEvent(const ldmx::EventHeader& eh) const;

  /** Checks to see if this IOV overlaps with the given IOV */
  bool overlaps(const ConditionsIOV& iov) const;

  /**
   * Print the object to std::cout
   */
  void Print() const;

  /**
   * Print the object to a string
   */
  std::string ToString() const;

  /**
   * Stream the object contents to an output stream
   */
  void stream(std::ostream&) const;

 private:
  /** First run for which this condition is valid */
  int firstRun_;

  /** Last run for which this condition is valid or -1 for infinite validity */
  int lastRun_;

  /** Is this Condition valid for real data? */
  bool validForData_;

  /** Is this Condition valid for simulation? */
  bool validForMC_;
};
}  // namespace framework

#endif  // FRAMEWORK_CONDITIONSIOV_H_
