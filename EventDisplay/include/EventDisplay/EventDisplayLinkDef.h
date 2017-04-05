/**
 * @file EventDisplayLinkDef.h
 * @brief Pre-processor macro commands for configuring creation of the ROOT class dictionary
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ class ldmx::EventDisplay+;

#endif

