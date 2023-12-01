
#include "Framework/Performance/Callback.h"

namespace framework::performance {

std::size_t to_index(Callback c) {
  return static_cast<std::size_t>(c);
}

const char* to_name(Callback c) {
  switch (c) {
    case Callback::onProcessStart:
      return "onProcessStart";
      break;
    case Callback::onProcessEnd:
      return "onProcessEnd";
      break;
    case Callback::onFileOpen:
      return "onFileOpen";
      break;
    case Callback::onFileClose:
      return "onFileClose";
      break;
    case Callback::beforeNewRun:
      return "beforeNewRun";
      break;
    case Callback::onNewRun:
      return "onNewRun";
      break;
    case Callback::process:
      return "process";
      break;
    default:
      return "WTF";
  }
}

}
