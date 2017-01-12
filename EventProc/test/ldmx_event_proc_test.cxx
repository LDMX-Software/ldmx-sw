#include "Event/SimEvent.h"
#include "EventProc/EventLoop.h"
#include "EventProc/RootEventSource.h"
#include "EventProc/DummyEventProcessor.h"

using event::SimEvent;
using eventproc::EventLoop;
using eventproc::RootEventSource;
using eventproc::DummyEventProcessor;

int main(int argc, const char* argv[])  {

    if (argc < 2) {
        std::cerr << "ERROR: Missing at least one input file argument." << std::endl;
        exit(1);
    }

    /*
    std::list<std::string> fileList;

    for (int iFile = 1; iFile < argc; iFile++) {
        std::cout << "Adding file " << argv[iFile] << std::endl;
        fileList.push_back(argv[iFile]);
    }

    RootEventSource* src = new RootEventSource(fileList, new SimEvent());
    EventLoop* loop = new EventLoop();
    loop->setEventSource(src);
    loop->addEventProcessor(new DummyEventProcessor);
    loop->initialize();
    loop->run(-1);
    loop->finish(); */
}
