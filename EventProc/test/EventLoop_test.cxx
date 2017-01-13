#include "EventProc/EventLoop.h"
#include "EventProc/EcalVetoProcessor.h"

using eventproc::EventLoop;

#include "EventProc/DummyEventProcessor.h"
#include "EventProc/ExampleCalHitProcessor.h"

using eventproc::DummyEventProcessor;
using eventproc::ExampleCalHitProcessor;

int main(int argc, const char* argv[])  {

    if (argc < 2) {
        std::cerr << "ERROR: Not enough arguments." << std::endl;
        throw std::runtime_error("Usage: EventLoop_test inputFileName");
    }

    // Setup the loop.
    EventLoop* loop = new EventLoop();
    loop->setPassName("recon");
    loop->setInputFileName(argv[1]);
    loop->setOutputFileName("EventLoop_test_output.root");
    loop->addEventProcessor(new DummyEventProcessor());
    loop->addEventProcessor(new ExampleCalHitProcessor());

    // Run the job.
    loop->initialize();
    loop->run(10);
    loop->finish();

    delete loop;
}
