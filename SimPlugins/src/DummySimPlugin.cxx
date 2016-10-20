#include "SimPlugins/DummySimPlugin.h"

extern "C" sim::DummySimPlugin* createDummySimPlugin() {
  return new sim::DummySimPlugin;
}

extern "C" void destroyDummySimPlugin(sim::DummySimPlugin* object) {
  delete object;
}
