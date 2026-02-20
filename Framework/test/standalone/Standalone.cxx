#include "Framework/EventProcessor.h"

class Standalone : public framework::Analyzer {
 public:
  Standalone(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  ~Standalone() override = default;
  void onProcessStart() override;
  void analyze(const framework::Event& event) override;
};

void Standalone::onProcessStart() {
  histograms_.create("event", "Event Number", 11, 0, 20);
}

void Standalone::analyze(const framework::Event& event) {
  histograms_.fill("event", event.getEventNumber());
}

DECLARE_ANALYZER(Standalone);
