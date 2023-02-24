#ifndef TRIGGER_TESTER_H_
#define TRIGGER_TESTER_H_

#include "Framework/EventProcessor.h"

namespace trigger {

/**
 * @class Tester
 * @brief Null algorithm test 
 */
class Tester : public framework::Producer {
 public:
  Tester(const std::string& name,
                           framework::Process& process);
  virtual void configure(framework::config::Parameters&);
  virtual void produce(framework::Event& event);

};
}  // namespace trigger

#endif // TRIGGER_TESTER_H_
