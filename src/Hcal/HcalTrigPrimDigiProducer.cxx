#include "Hcal/HcalTrigPrimDigiProducer.h"

#include "Hcal/HcalTriggerGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
#include "Tools/HgcrocTriggerCalculations.h"

namespace hcal {
HcalTrigPrimDigiProducer::HcalTrigPrimDigiProducer(const std::string& name,
                                                   framework::Process& process)
    : Producer(name, process) {}

void HcalTrigPrimDigiProducer::configure(framework::config::Parameters& ps) {
  digiCollName_ = ps.getParameter<std::string>("digiCollName");
  digiPassName_ = ps.getParameter<std::string>("digiPassName");
  condObjName_ =
      ps.getParameter<std::string>("condObjName", "HcalTrigPrimDigiConditions");
}

void HcalTrigPrimDigiProducer::produce(framework::Event& event) {
  const HcalTriggerGeometry& geom = getCondition<HcalTriggerGeometry>(
      HcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

  const ldmx::HgcrocDigiCollection& hcalDigis =
      event.getObject<ldmx::HgcrocDigiCollection>(digiCollName_, digiPassName_);

  // get the calibration object
  const conditions::IntegerTableCondition& conditions =
      getCondition<conditions::IntegerTableCondition>(condObjName_);

  // construct the calculator...
  ldmx::HgcrocTriggerCalculations calc(conditions);
  stq_tps.clear();
  
  // Loop over the digis
  for (unsigned int ix = 0; ix < hcalDigis.getNumDigis(); ix++) {
    const ldmx::HgcrocDigiCollection::HgcrocDigi pdigi = hcalDigis.getDigi(ix);
    // ldmx::HcalTriggerID tid = geom.belongsTo( ldmx::HcalDigiID(pdigi.id()) );
    ldmx::HcalTriggerID tid = geom.belongsToQuad( ldmx::HcalDigiID(pdigi.id()) );
    ldmx::HcalTriggerID stq_id = geom.belongsToSTQ( ldmx::HcalDigiID(pdigi.id()) );

    if (!tid.null()) {
      int tot = 0;
      if (pdigi.soi().isTOTComplete()) tot = pdigi.soi().tot();
      calc.addDigi(pdigi.id(), tid.raw(), pdigi.soi().adc_t(), tot);
    }

    auto ptr = stq_tps.find(stq_id.raw());
    if (ptr != stq_tps.end()) ptr->second += pdigi.soi().adc_t();
    else stq_tps[stq_id.raw()] = pdigi.soi().adc_t();
  }
  
  // // Now, we compress the digis
  calc.compressDigis(4);

  // const std::map<unsigned int, uint8_t> results;
  const std::map<unsigned int, uint8_t>& results = calc.compressedEnergies();
  ldmx::HgcrocTrigDigiCollection tdigis;

  for (auto result : results) {
    if (result.second > 0) {
      tdigis.push_back(ldmx::HgcrocTrigDigi(result.first, result.second));
    }
  }

  ldmx::HgcrocTrigDigiCollection stq_digis;
  for (auto result : stq_tps) {
    if (result.second > 0) {
      stq_digis.push_back(ldmx::HgcrocTrigDigi(result.first, result.second));
    }
  }

  event.add(getName(), tdigis);
  event.add("hcalTrigPrimDigiSTQs", stq_digis);
}
}  // namespace hcal
DECLARE_PRODUCER_NS(hcal, HcalTrigPrimDigiProducer);
