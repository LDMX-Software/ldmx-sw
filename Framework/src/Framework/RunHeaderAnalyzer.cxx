#include "Framework/RunHeaderAnalyzer.h"

namespace framework {

void onNewRun(ldmx::RunHeader& rh) {
  rh.Print();
}

void analyze(const framework::Event& event) {
 std::cout << " heeey " << std::endl;
}

}
