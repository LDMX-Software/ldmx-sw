#include "../src/TotalEnergy.h"
#include "../../Algo/include/Trigger/DiscreteInputs_IO.h"

#define NTEST 5

int main(){

    DiscreteInputs inputs("test.dump");
    //DiscreteInputs inputs("../../../../data/test.dump");
    
    for (int test = 0; test < NTEST; test++) {
        
        EcalTP Input_TPs[N_INPUT_TP];
        e_t energy_hw ;
        e_t energy_ref;

        //std::vector<EcalTP> inputs;
        if (!inputs.nextEvent( /*inputs*/ )) break;

        TotalEnergy_ref(Input_TPs, energy_ref);
        TotalEnergy_hw(Input_TPs,  energy_hw);
        
        printf( "total energy = %f %f \n", float(energy_ref), float(energy_hw) );
    }

}
