
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace trigger;
#pragma link C++ class trigger::TrigEnergySum + ;
#pragma link C++ class std::vector < trigger::TrigEnergySum> + ;
#pragma link C++ class trigger::TrigCaloHit + ;
#pragma link C++ class std::vector < trigger::TrigCaloHit> + ;
#pragma link C++ class trigger::TrigCaloCluster + ;
#pragma link C++ class std::vector < trigger::TrigCaloCluster> + ;
#pragma link C++ class trigger::TrigMip + ;
#pragma link C++ class std::vector < trigger::TrigMip> + ;
#pragma link C++ class trigger::TrigParticle + ;
#pragma link C++ class std::vector < trigger::TrigParticle> + ;

#endif
