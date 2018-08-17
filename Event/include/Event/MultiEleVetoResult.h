/**
 * @file HcalVetoResult.h
 * @brief Class used for storing multielectron veto features
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENT_MULTIELEVETORESULT_H_
#define EVENT_MULTIELEVETORESULT_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <iostream>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include <TObject.h>

namespace ldmx { 
    
    class MultiEleVetoResult : public TObject { 
        
        public: 

            /** Constructor */
            MultiEleVetoResult(); 

            /** Destructor */
            ~MultiEleVetoResult(); 

	    /** push zero for all data members */
	    void addElectron();

            /** Copy the object */
            void Copy(TObject& object) const; 

            /** Reset the object. */
            void Clear(Option_t *option = ""); 
            
            /** Print out the object */
            void Print(Option_t *option = "");

	    int nRecoilElectrons{0};
	    std::vector<float> cylinder_0_1_layer_0_0 ;
	    std::vector<float> cylinder_0_1_layer_1_2 ;
	    std::vector<float> cylinder_0_1_layer_3_6 ;
	    std::vector<float> cylinder_0_1_layer_7_14;
	    std::vector<float> cylinder_0_1_layer_15  ;
                      
	    std::vector<float> cylinder_1_3_layer_0_0 ;
	    std::vector<float> cylinder_1_3_layer_1_2 ;
	    std::vector<float> cylinder_1_3_layer_3_6 ;
	    std::vector<float> cylinder_1_3_layer_7_14;
	    std::vector<float> cylinder_1_3_layer_15  ;
                      
	    std::vector<float> cylinder_3_5_layer_0_0 ;
	    std::vector<float> cylinder_3_5_layer_1_2 ;
	    std::vector<float> cylinder_3_5_layer_3_6 ;
	    std::vector<float> cylinder_3_5_layer_7_14;
	    std::vector<float> cylinder_3_5_layer_15  ;
                      
	    std::vector<float> cylinder_5_layer_0_0   ;
	    std::vector<float> cylinder_5_layer_1_2   ;
	    std::vector<float> cylinder_5_layer_3_6   ;
	    std::vector<float> cylinder_5_layer_7_14  ;
	    std::vector<float> cylinder_5_layer_15    ;

  private :
	    ClassDef(MultiEleVetoResult, 1); 
	
    }; // MultiEleVetoResult
}


#endif // EVENT_MULTIELEVETORESULT_H_
