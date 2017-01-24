#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"
#include <iostream>
#include "TClonesArray.h"
#include "Event/SimParticle.h"
#include "Event/Event.h"
#include "TRandom.h"


class DummyProducer : public ldmxsw::Producer {
  TRandom random_;
public:
  DummyProducer(const std::string& name, ldmxsw::Process& process) : ldmxsw::Producer(name,process) { }

  virtual void configure(const ldmxsw::ParameterSet& ps) {
    n_particles=ps.getInteger("n_particles");
    ave_energy=ps.getDouble("ave_energy");
    direction=ps.getVDouble("direction");
  }
  
  virtual void produce(event::Event& event) {
    std::cout << "DummyProducer: Analyzing an event!" << std::endl;

    int np=random_.Poisson(n_particles);
    for (int i=0; i<np; i++) {
      event::SimParticle* a=(event::SimParticle*)tca->ConstructedAt(i);
      do {
	a->setEnergy(random_.Gaus(ave_energy,1.0));
      } while (a->getEnergy()<0);
      a->setPdgID(i+1);
    }
    event.add("simParticles",tca);
  }
  virtual void onFileOpen() {
    std::cout << "DummyProducer: Opening a file!" << std::endl;
  }
  virtual void onFileClose() {
    std::cout << "DummyProducer: Closing a file!" << std::endl;
  }
  virtual void onProcessStart() {
    std::cout << "DummyProducer: Starting processing!" << std::endl;
    tca=new TClonesArray("event::SimParticle",1000);
  }
  virtual void onProcessEnd() {
    std::cout << "DummyProducer: Finishing processing!" << std::endl;
  }
private:
  TClonesArray* tca;
  int n_particles;
  double ave_energy;
  std::vector<double> direction;
};

DECLARE_ANALYZER(DummyProducer);
