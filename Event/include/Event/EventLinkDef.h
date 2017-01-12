/**
 * @file EventLinkDef.h
 * @brief Pre-processor macro commands for configuring creation of the ROOT class dictionary
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace event;
#pragma link C++ defined_in namespace event;

#pragma link C++ class event::Event+;
#pragma link C++ class event::EventConstants+;
#pragma link C++ class event::EventHeader+;
#pragma link C++ class event::RunHeader+;
#pragma link C++ class event::SimEvent+;
#pragma link C++ class event::SimCalorimeterHit+;
#pragma link C++ class event::SimTrackerHit+;
#pragma link C++ class event::SimParticle+;

#endif

