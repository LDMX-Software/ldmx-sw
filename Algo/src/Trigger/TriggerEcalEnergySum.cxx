#include "Trigger/TriggerEcalEnergySum.h"

#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "DetDescr/EcalHexReadout.h"

namespace ldmx {

    void TriggerEcalEnergySum::configure(Parameters &ps) {
	
    }

    void TriggerEcalEnergySum::produce(ldmx::Event &event) {

	const EcalTriggerGeometry& geom=getCondition<EcalTriggerGeometry>(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);
	const EcalHexReadout& hexReadout = getCondition<EcalHexReadout>(EcalHexReadout::CONDITIONS_OBJECT_NAME);
	
	if (!event.exists("ecalTrigDigis")) return;
        auto ecalTrigDigis{event.getObject<HgcrocTrigDigiCollection>("ecalTrigDigis")};

	float total_e=0;
	e_t total_e_trunc=0;

	for(const auto& trigDigi : ecalTrigDigis){
	    // HgcrocTrigDigi

	    EcalTriggerID tid( trigDigi.getId() /*raw value*/ );
	    // compressed ECal digis are 8xADCs (HCal will be 4x)
	    float sie =  8*trigDigi.linearPrimitive() * gain * mVtoMeV; // in MeV, before layer corrections
	    float e = (sie/mipSiEnergy * layerWeights.at( tid.layer() ) + sie) * secondOrderEnergyCorrection;
	    total_e += e;
	    total_e_trunc = total_e_trunc + e_t(e);
 	    // auto xy = geom.globalPosition( tid );
	    // double _x;
	    // double _y;
	    // double _z;
	    // auto center_ecalID = geom.centerInTriggerCell(tid);
	    // hexReadout.getCellAbsolutePosition(center_ecalID,_x,_y,_z);
	}
	std::cout << "Total ECal energy: " << total_e << " MeV (fixed-point: "<< total_e_trunc <<" MeV)" << std::endl;

    }


    void TriggerEcalEnergySum::onFileOpen() {

	ldmx_log(debug) << "Opening file!";

	return;
    }

    void TriggerEcalEnergySum::onFileClose() {

	ldmx_log(debug) << "Closing file!";

	return;
    }

    void TriggerEcalEnergySum::onProcessStart() {

	ldmx_log(debug) << "Process starts!";

	return;
    }

    void TriggerEcalEnergySum::onProcessEnd() {

	ldmx_log(debug) << "Process ends!";

	return;
    }

}

DECLARE_PRODUCER_NS(ldmx, TriggerEcalEnergySum);
