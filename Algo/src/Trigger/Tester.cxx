#include "Trigger/Tester.h"

namespace trigger {
Tester::Tester(const std::string& name,
               framework::Process& process)
    : Producer(name, process) {}

void Tester::configure(framework::config::Parameters& ps) {
  std::cout << " Tester::cfg " << std::endl;
}

void Tester::produce(framework::Event& event) {
  std::cout << " Tester::produce " << std::endl;
}
}  // namespace trigger
DECLARE_PRODUCER_NS(trigger, Tester);
