#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace framework::performance;

#pragma link C++ class framework::performance::Measurement+;
#pragma link C++ class std::map<std::string,framework::performance::Measurement>+;

#endif
