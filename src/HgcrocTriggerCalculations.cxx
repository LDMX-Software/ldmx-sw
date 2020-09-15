#include "Ecal/HgcrocTriggerCalculations.h"

namespace ldmx {
  
    unsigned int HgcrocTriggerCalculations::singleChannelCharge(int adc, int tot,
								int adc_ped, int adc_thresh,
								int tot_ped, int tot_thresh, int tot_gain) {
	
	unsigned int charge_adc=0;
	
	if (adc>(adc_ped+adc_thresh)) charge_adc=adc-adc_ped; // otherwise zero

	unsigned int eff_tot{0};
	if (tot>tot_thresh && tot>tot_ped) eff_tot=tot-tot_ped;
	else eff_tot=tot_thresh-tot_ped;
	
	unsigned int charge_tot{0};
	charge_tot=eff_tot*tot_gain;
	
	return (tot!=0)?(charge_tot):(charge_adc);
        
    }

    HgcrocTriggerCalculations::HgcrocTriggerCalculations(const IntegerTableCondition& ict) : conditions_{ict, true} {
    }

    void HgcrocTriggerCalculations::addDigi(unsigned int id, int adc, int tot, bool keep_zeroes) {
	unsigned int charge=singleChannelCharge(adc,tot,
						conditions_.adc_pedestal(id),conditions_.adc_threshold(id),
						conditions_.tot_pedestal(id),conditions_.tot_threshold(id),
						conditions_.tot_gain(id));
	if (charge>0 || keep_zeroes) linearCharge_[id]=charge;
    }
    
    HgcrocTriggerConditions::HgcrocTriggerConditions(const IntegerTableCondition& ict, bool validate): ict_{ict} {
	if (!validate) return;
	if (ict_.getColumnCount()<5) {
	    EXCEPTION_RAISE("ConditionsException","Inconsistent condition for HgcrocTriggerConditions :"+ict.getName());
	}
	std::vector<std::string> expected_colnames;
	expected_colnames.push_back("ADC_PEDESTAL");
	expected_colnames.push_back("ADC_THRESHOLD");
	expected_colnames.push_back("TOT_PEDESTAL");
	expected_colnames.push_back("TOT_THRESHOLD");
	expected_colnames.push_back("TOT_GAIN");

	for (size_t i=0; i<5; i++) {
	    if (ict_.getColumnNames()[i]!=expected_colnames[i]) {
		EXCEPTION_RAISE("ConditionsException","Expected column '"+expected_colnames[i]+"', got '"+ict_.getColumnNames()[i]+"'");
	    }
	}
    }

    

}
