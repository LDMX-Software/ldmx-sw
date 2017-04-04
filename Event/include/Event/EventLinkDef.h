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

#pragma link C++ namespace ldmx;
#pragma link C++ defined_in namespace ldmx;

#pragma link C++ class ldmx::CalorimeterHit+;
#pragma link C++ class ldmx::HcalHit+;
#pragma link C++ class ldmx::HcalVetoResult+;
#pragma link C++ class ldmx::EcalHit+;
#pragma link C++ class ldmx::EcalVetoResult+;
#pragma link C++ class ldmx::EventConstants+;
#pragma link C++ class ldmx::EventHeader+;
#pragma link C++ class ldmx::FindableTrackResult+;
#pragma link C++ class ldmx::RunHeader+;
#pragma link C++ class ldmx::SimCalorimeterHit+;
#pragma link C++ class ldmx::SimTrackerHit+;
#pragma link C++ class ldmx::SimParticle+;
#pragma link C++ class ldmx::TriggerResult+;

#endif

