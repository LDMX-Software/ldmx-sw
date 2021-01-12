#include "../src/data.h"
#include "../src/TotalEnergy.h"

void TotalEnergy_ref(EcalTP Input_TPs[N_INPUT_TP], e_t energy){

    energy=0;
    for(int i=0;i<N_INPUT_TP;i++){
        // TODO get linear energy
        energy += Input_TPs[i].tp;
    }
}
