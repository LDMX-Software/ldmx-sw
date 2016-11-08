#include "EventProc/EventLoop.h"
#include "EventProc/RootEventSource.h"
#include "EventProc/HcalDigiProcessor.h"

using eventproc::EventLoop;
using eventproc::RootEventSource;

#include "Event/SimCalorimeterHit.h"

int main(int argc, const char* argv[])  {

    if (argc < 3) {
        std::cerr << "Wrong number of inputs - (1) input file (2) output file " << std::endl;
        exit(1);
    }

    std::list<std::string> fileList;

    std::cout << "Adding file " << argv[1] << std::endl;
    fileList.push_back(argv[1]);
    
    RootEventSource* src = new RootEventSource(fileList, new SimEvent());
    EventLoop* loop = new EventLoop();
    loop->setEventSource(src);
    loop->addEventProcessor(new eventproc::HcalDigiProcessor(argv[2]));
    loop->initialize();
    loop->run(-1);
    loop->finish();
}
