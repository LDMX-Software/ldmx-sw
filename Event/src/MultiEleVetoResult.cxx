#include "Event/MultiEleVetoResult.h"

ClassImp(ldmx::MultiEleVetoResult)

namespace ldmx {

  MultiEleVetoResult::MultiEleVetoResult() : 
    TObject(){}

  MultiEleVetoResult::~MultiEleVetoResult(){}

  void MultiEleVetoResult::addElectron(){

    nRecoilElectrons++;
    
    cylinder_0_1_layer_0_0.push_back(0.) ;
    cylinder_0_1_layer_1_2.push_back(0.) ;
    cylinder_0_1_layer_3_6.push_back(0.) ;
    cylinder_0_1_layer_7_14.push_back(0.);
    cylinder_0_1_layer_15.push_back(0.)  ;
                       
    cylinder_1_3_layer_0_0.push_back(0.) ;
    cylinder_1_3_layer_1_2.push_back(0.) ;
    cylinder_1_3_layer_3_6.push_back(0.) ;
    cylinder_1_3_layer_7_14.push_back(0.);
    cylinder_1_3_layer_15.push_back(0.)  ; 
                       
    cylinder_3_5_layer_0_0.push_back(0.) ;
    cylinder_3_5_layer_1_2.push_back(0.) ;
    cylinder_3_5_layer_3_6.push_back(0.) ;
    cylinder_3_5_layer_7_14.push_back(0.);
    cylinder_3_5_layer_15.push_back(0.)  ;
                       
    cylinder_5_layer_0_0.push_back(0.)   ;
    cylinder_5_layer_1_2.push_back(0.)   ;
    cylinder_5_layer_3_6.push_back(0.)   ;
    cylinder_5_layer_7_14.push_back(0.)  ;
    cylinder_5_layer_15.push_back(0.)    ;
  }

  void MultiEleVetoResult::Copy(TObject& object) const {
    MultiEleVetoResult& result = (MultiEleVetoResult&) object;
    
    result.cylinder_0_1_layer_0_0       = cylinder_0_1_layer_0_0 ;
    result.cylinder_0_1_layer_1_2 	= cylinder_0_1_layer_1_2 ;
    result.cylinder_0_1_layer_3_6 	= cylinder_0_1_layer_3_6 ;
    result.cylinder_0_1_layer_7_14	= cylinder_0_1_layer_7_14;
    result.cylinder_0_1_layer_15  	= cylinder_0_1_layer_15  ;

    result.cylinder_1_3_layer_0_0 	= cylinder_1_3_layer_0_0 ;
    result.cylinder_1_3_layer_1_2 	= cylinder_1_3_layer_1_2 ;
    result.cylinder_1_3_layer_3_6 	= cylinder_1_3_layer_3_6 ;
    result.cylinder_1_3_layer_7_14	= cylinder_1_3_layer_7_14;
    result.cylinder_1_3_layer_15  	= cylinder_1_3_layer_15  ;

    result.cylinder_3_5_layer_0_0 	= cylinder_3_5_layer_0_0 ;
    result.cylinder_3_5_layer_1_2 	= cylinder_3_5_layer_1_2 ;
    result.cylinder_3_5_layer_3_6 	= cylinder_3_5_layer_3_6 ;
    result.cylinder_3_5_layer_7_14	= cylinder_3_5_layer_7_14;
    result.cylinder_3_5_layer_15  	= cylinder_3_5_layer_15  ;

    result.cylinder_5_layer_0_0   	= cylinder_5_layer_0_0   ;
    result.cylinder_5_layer_1_2   	= cylinder_5_layer_1_2   ;
    result.cylinder_5_layer_3_6   	= cylinder_5_layer_3_6   ;
    result.cylinder_5_layer_7_14  	= cylinder_5_layer_7_14  ;
    result.cylinder_5_layer_15    	= cylinder_5_layer_15    ;
  }
  
  void MultiEleVetoResult::Clear(Option_t *option){

      nRecoilElectrons=0;

      cylinder_0_1_layer_0_0.clear();
      cylinder_0_1_layer_1_2.clear(); 
      cylinder_0_1_layer_3_6.clear();
      cylinder_0_1_layer_7_14.clear();
      cylinder_0_1_layer_15.clear();
      
      cylinder_1_3_layer_0_0.clear();
      cylinder_1_3_layer_1_2.clear(); 
      cylinder_1_3_layer_3_6.clear(); 
      cylinder_1_3_layer_7_14.clear();
      cylinder_1_3_layer_15.clear();  
      
      cylinder_3_5_layer_0_0.clear(); 
      cylinder_3_5_layer_1_2.clear(); 
      cylinder_3_5_layer_3_6.clear(); 
      cylinder_3_5_layer_7_14.clear();
      cylinder_3_5_layer_15.clear();  
      
      cylinder_5_layer_0_0.clear();   
      cylinder_5_layer_1_2.clear();   
      cylinder_5_layer_3_6.clear();   
      cylinder_5_layer_7_14.clear();  
      cylinder_5_layer_15.clear();    

  }
  
  void MultiEleVetoResult::Print(Option_t *option){
      std::cout << "[ MultiEleVetoResult ]: " << std::endl;
      for( int i_ele = 0 ; i_ele < cylinder_5_layer_0_0.size() ; i_ele++ ){
	  std::cout << " - - - - - - - electron " << i_ele << " - - - - - - - - - " << std::endl;
	  std::cout << "cylinder_0_1_layer_0_0       " << cylinder_0_1_layer_0_0[i_ele] << std::endl;
	  std::cout << "cylinder_0_1_layer_1_2 	 " << cylinder_0_1_layer_1_2[i_ele] << std::endl;
	  std::cout << "cylinder_0_1_layer_3_6 	 " << cylinder_0_1_layer_3_6[i_ele] << std::endl;
	  std::cout << "cylinder_0_1_layer_7_14	 " << cylinder_0_1_layer_7_14[i_ele]<< std::endl;
	  std::cout << "cylinder_0_1_layer_15  	 " << cylinder_0_1_layer_15[i_ele]  << std::endl;
	  
	  std::cout << "cylinder_1_3_layer_0_0 	 " << cylinder_1_3_layer_0_0[i_ele] << std::endl;
	  std::cout << "cylinder_1_3_layer_1_2 	 " << cylinder_1_3_layer_1_2[i_ele] << std::endl;
	  std::cout << "cylinder_1_3_layer_3_6 	 " << cylinder_1_3_layer_3_6[i_ele] << std::endl;
	  std::cout << "cylinder_1_3_layer_7_14	 " << cylinder_1_3_layer_7_14[i_ele]<< std::endl;
	  std::cout << "cylinder_1_3_layer_15  	 " << cylinder_1_3_layer_15[i_ele]  << std::endl;
	  
	  std::cout << "cylinder_3_5_layer_0_0 	 " << cylinder_3_5_layer_0_0[i_ele] << std::endl;
	  std::cout << "cylinder_3_5_layer_1_2 	 " << cylinder_3_5_layer_1_2[i_ele] << std::endl;
	  std::cout << "cylinder_3_5_layer_3_6 	 " << cylinder_3_5_layer_3_6[i_ele] << std::endl;
	  std::cout << "cylinder_3_5_layer_7_14	 " << cylinder_3_5_layer_7_14[i_ele]<< std::endl;
	  std::cout << "cylinder_3_5_layer_15  	 " << cylinder_3_5_layer_15[i_ele]  << std::endl;
	  
	  std::cout << "cylinder_5_layer_0_0   	 " << cylinder_5_layer_0_0[i_ele]   << std::endl;
	  std::cout << "cylinder_5_layer_1_2   	 " << cylinder_5_layer_1_2[i_ele]   << std::endl;
	  std::cout << "cylinder_5_layer_3_6   	 " << cylinder_5_layer_3_6[i_ele]   << std::endl;
	  std::cout << "cylinder_5_layer_7_14  	 " << cylinder_5_layer_7_14[i_ele]  << std::endl;
	  std::cout << "cylinder_5_layer_15    	 " << cylinder_5_layer_15[i_ele]    << std::endl;
      }
  }

}
