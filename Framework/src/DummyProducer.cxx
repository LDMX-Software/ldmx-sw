/**
 * @file DummyProducer.cxx
 * @brief Class that defines a dummy Producer implementation that adds some random data to the event
 * @author Jeremy Mans, University of Minnesota
 */

// ROOT
#include "TClonesArray.h"
#include "TRandom.h"

// LDMX
#include "Event/SimParticle.h"
#include "Event/Event.h"
#include "Event/EventConstants.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

// STL
#include <iostream>

namespace ldmx {

/**
 * @class DummyProducer
 * @brief Defines a dummy Producer implementation that adds some random data to the event
 */
class DummyProducer : public Producer {

    public:

        DummyProducer(const std::string& name, const Process& process) :
                Producer(name, process) {
        }

        virtual void configure(const ParameterSet& ps) {
            nParticles_ = ps.getInteger("nParticles_");
            aveEnergy_ = ps.getDouble("aveEnergy_");
            direction_ = ps.getVDouble("direction_");
        }

        virtual void produce(Event& event) {
            std::cout << "DummyProducer: Analyzing an event!" << std::endl;

            int np = random_.Poisson(nParticles_);
            for (int i = 0; i < np; i++) {
                SimParticle* a = (SimParticle*) tca_->ConstructedAt(i);
                do {
                    a->setEnergy(random_.Gaus(aveEnergy_, 1.0));
                } while (a->getEnergy() < 0);
                a->setPdgID(i + 1);
            }
            event.add("simParticles", tca_);
        }

        virtual void onFileOpen() {
            std::cout << "DummyProducer: Opening a file!" << std::endl;
        }

        virtual void onFileClose() {
            std::cout << "DummyProducer: Closing a file!" << std::endl;
        }

        virtual void onProcessStart() {
            std::cout << "DummyProducer: Starting processing!" << std::endl;
            tca_ = new TClonesArray(EventConstants::SIM_PARTICLE.c_str(), 1000);
        }

        virtual void onProcessEnd() {
            std::cout << "DummyProducer: Finishing processing!" << std::endl;
        }

    private:
        TClonesArray* tca_{nullptr};
        int nParticles_{0};
        double aveEnergy_{0};
        std::vector<double> direction_;
        TRandom random_;
};

}

DECLARE_ANALYZER_NS(ldmx, DummyProducer);
