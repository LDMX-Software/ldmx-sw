
#include "Framework/Performance/Callback.h"
#include "Framework/Exception/Exception.h"

namespace framework::performance {

std::string to_name(Callback c) {
  switch (c) {
    case Callback::onProcessStart:
      return "onProcessStart";
    case Callback::onProcessEnd:
      return "onProcessEnd";
    case Callback::onFileOpen:
      return "onFileOpen";
    case Callback::onFileClose:
      return "onFileClose";
    case Callback::beforeNewRun:
      return "beforeNewRun";
    case Callback::onNewRun:
      return "onNewRun";
    case Callback::process:
      return "process";
  }
  EXCEPTION_RAISE(
      "BadCode",
      "Somehow we got a Callback enum (value: "+std::to_string(to_index(c))+
      ") that doesn't match one of the listed possibilities\n"
      "Did a new Callback get added to the performance tracker and the to_name function wasn't updated?"
  );
}

}
