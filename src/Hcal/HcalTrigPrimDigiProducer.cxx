#include "Hcal/HcalTrigPrimDigiProducer.h"

// #include "Hcal/HcalTriggerGeometry.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"
// #include "Tools/HgcrocTriggerCalculations.h"

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
  // const HcalTriggerGeometry& geom = getCondition<HcalTriggerGeometry>(
  //     HcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

  // const ldmx::HgcrocDigiCollection& hcalDigis =
  //     event.getObject<ldmx::HgcrocDigiCollection>(digiCollName_, digiPassName_);

  // // get the calibration object
  // const conditions::IntegerTableCondition& conditions =
  //     getCondition<conditions::IntegerTableCondition>(condObjName_);

  // // construct the calculator...
  // ldmx::HgcrocTriggerCalculations calc(conditions);

  // // Loop over the digis
  // for (unsigned int ix = 0; ix < hcalDigis.getNumDigis(); ix++) {
  //   const ldmx::HgcrocDigiCollection::HgcrocDigi pdigi = hcalDigis.getDigi(ix);
  //   // std::cout << HcalID(pdigi.id()) << pdigi << std::endl;

  //   ldmx::HcalTriggerID tid = geom.belongsTo(ldmx::HcalID(pdigi.id()));

  //   if (!tid.null()) {
  //     int tot = 0;
  //     if (pdigi.soi().isTOTComplete()) tot = pdigi.soi().tot();
  //     calc.addDigi(pdigi.id(), tid.raw(), pdigi.soi().adc_t(), tot);
  //   }
  // }

  // // Now, we compress the digis
  // calc.compressDigis(9);  // 9 is the number for Hcal...

  // const std::map<unsigned int, uint8_t>& results = calc.compressedEnergies();
  // ldmx::HgcrocTrigDigiCollection tdigis;

  // for (auto result : results) {
  //   if (result.second > 0) {
  //     tdigis.push_back(ldmx::HgcrocTrigDigi(result.first, result.second));
  //     // std::cout << HcalTriggerID(result.first) << "  " << tdigis.back() <<
  //     // std::endl;
  //   }
  // }

  // temporary for testing!!
  ldmx::HgcrocTrigDigiCollection tdigis;
  for(int i=0;i<4;i++){
      tdigis.push_back(ldmx::HgcrocTrigDigi(i,i+2));
  }

  // std::cout << hcalDigis.size() << " " << tdigis.size() << std::endl;
  event.add(getName(), tdigis);
}
}  // namespace hcal
DECLARE_PRODUCER_NS(hcal, HcalTrigPrimDigiProducer);
