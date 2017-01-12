#include "EventProc/EventLoop.h"
#include "EventProc/RootEventSource.h"
#include "EventProc/EcalVetoProcessor.h"

using eventproc::EventLoop;
using eventproc::RootEventSource;

#include "Event/SimCalorimeterHit.h"

int main(int argc, const char* argv[])  {

    /*if (argc < 3) {
        std::cerr << "Wrong number of inputs - (1) input file (2) output file " << std::endl;
        exit(1);
    }*/

    //////////////////////////////////////////////////////////////////////////
    // - - - - - - - - - - - - output tree setup - - - - - - - - - - - - -  //
    //////////////////////////////////////////////////////////////////////////
    /*
    TFile* outputFile = new TFile(outputFileName,"RECREATE");
    TTree* outputTree = new TTree("ecalVeto","ecalVeto");*/

    //std::list<std::string> fileList;

    //std::cout << "Adding file " << argv[1] << std::endl;
    //fileList.push_back(argv[1]);

    //RootEventSource* src = new RootEventSource(fileList, new SimEvent());
    event::EventFile simFile(argv[1], "LDMX_Event", false, 9);  
    TString outputFileName = argv[2];
    event::EventFile* outputFile = new event::EventFile(argv[2], &simFile);
    event::EventImpl* event = new event::EventImpl("recon");  
    outputFile->setupEvent(event); 
    EventLoop* loop = new EventLoop();
    loop->setEventSource(outputFile);
    //loop->addEventProcessor(new eventproc::EcalVetoProcessor());
    loop->initialize();
    loop->run(500);
    loop->finish();

    outputFile->close();
    delete event;
    delete outputFile;
    delete loop;
}
