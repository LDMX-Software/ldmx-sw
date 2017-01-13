#include "EventProc/EventLoop.h"
#include "EventProc/EcalVetoProcessor.h"

using eventproc::EventLoop;

#include "Event/SimCalorimeterHit.h"

int main(int argc, const char* argv[])  {

    if (argc < 3) {
        std::cerr << "ERROR: Not enough arguments." << std::endl;
        throw std::runtime_error("Usage: ldmx_ecal_veto_producer inputFileName outputFileName");
    }

    EventLoop* loop = new EventLoop();
    loop->setInputFileName(std::string(argv[1]));
    loop->setOutputFileName(std::string(argv[2]));
    loop->addEventProcessor(new eventproc::EcalVetoProcessor());

    loop->initialize();
    loop->run(500);
    loop->finish();

    delete loop;
}
