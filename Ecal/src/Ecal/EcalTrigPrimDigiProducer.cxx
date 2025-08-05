#include "Ecal/EcalTrigPrimDigiProducer.h"

namespace ecal {
EcalTrigPrimDigiProducer::EcalTrigPrimDigiProducer(const std::string& name,
                                                   framework::Process& process)
    : Producer(name, process) {}

void EcalTrigPrimDigiProducer::configure(framework::config::Parameters& ps) {
  digiCollName_ = ps.getParameter<std::string>("digiCollName");
  digiPassName_ = ps.getParameter<std::string>("digiPassName");
  condObjName_ =
      ps.getParameter<std::string>("condObjName", "EcalTrigPrimDigiConditions");
}

void EcalTrigPrimDigiProducer::produce(framework::Event& event) {
  const EcalTriggerGeometry& geom = getCondition<EcalTriggerGeometry>(
      EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

  const ldmx::HgcrocDigiCollection& ecal_digis =
      event.getObject<ldmx::HgcrocDigiCollection>(digiCollName_, digiPassName_);

  // get the calibration object
  const conditions::IntegerTableCondition& conditions =
      getCondition<conditions::IntegerTableCondition>(condObjName_);

  // construct the calculator...
  ldmx::HgcrocTriggerCalculations calc(conditions);

  // Loop over the digis
  for (unsigned int ix = 0; ix < ecal_digis.getNumDigis(); ix++) {
    const ldmx::HgcrocDigiCollection::HgcrocDigi pdigi = ecal_digis.getDigi(ix);

    ldmx::EcalTriggerID tid = geom.belongsTo(ldmx::EcalID(pdigi.id()));

    if (!tid.null()) {
      int tot = 0;
      if (pdigi.soi().isTOTComplete()) tot = pdigi.soi().tot();
      calc.addDigi(pdigi.id(), tid.raw(), pdigi.soi().adc_t(), tot);
    }
  }

  // Now, we compress the digis
  calc.compressDigis(9);  // 9 is the number for Ecal...

  const std::map<unsigned int, uint8_t>& results = calc.compressedEnergies();
  ldmx::HgcrocTrigDigiCollection tdigis;

  for (auto result : results) {
    if (result.second > 0) {
      tdigis.push_back(ldmx::HgcrocTrigDigi(result.first, result.second));
    }
  }

  ldmx_log(trace) << " Ecal digi size = " << ecal_digis.size()
                  << " trigger digi size = " << tdigis.size();
  event.add(getName(), tdigis);
}
}  // namespace ecal
DECLARE_PRODUCER(ecal::EcalTrigPrimDigiProducer);
