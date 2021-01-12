#include "../src/TotalEnergy.h"

#define NTEST 5

int main(){

    for (int test = 0; test < NTEST; test++) {

        EcalTP Input_TPs[N_INPUT_TP];
        e_t energy_hw ;
        e_t energy_ref;

        TotalEnergy_ref(Input_TPs, energy_ref);
        TotalEnergy_hw(Input_TPs,  energy_hw);
        
        printf( "total energy = %f %f \n", float(energy_ref), float(energy_hw) );
    }

}
