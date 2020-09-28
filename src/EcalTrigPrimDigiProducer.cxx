#include "Ecal/EcalTrigPrimDigiProducer.h"

#include "Ecal/EcalTriggerGeometry.h"
#include "Event/HgcrocDigiCollection.h"
#include "Tools/HgcrocTriggerCalculations.h"

namespace ldmx {
    EcalTrigPrimDigiProducer::EcalTrigPrimDigiProducer(const std::string& name, Process& process) : Producer(name, process) {

    }

    void EcalTrigPrimDigiProducer::configure(Parameters& ps) {
        digiCollName_ = ps.getParameter<std::string>( "digiCollName" );
        digiPassName_ = ps.getParameter<std::string>( "digiPassName" );
        condObjName_ = ps.getParameter<std::string>("condObjName","EcalTrigPrimDigiConditions");
    }

    void EcalTrigPrimDigiProducer::produce(Event& event) {

        const EcalTriggerGeometry& geom=getCondition<EcalTriggerGeometry>(EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

        const HgcrocDigiCollection& ecalDigis = event.getObject<HgcrocDigiCollection>( digiCollName_ , digiPassName_ );
        
        // get the calibration object
        const IntegerTableCondition& conditions = getCondition<IntegerTableCondition>(condObjName_);

        // construct the calculator...
        HgcrocTriggerCalculations calc(conditions); 
    
        // Loop over the digis
        for (unsigned int ix=0; ix<ecalDigis.getNumDigis(); ix++) {
            const HgcrocDigiCollection::HgcrocDigi pdigi=ecalDigis.getDigi(ix);
            //std::cout << EcalID(pdigi.id()) << pdigi << std::endl;
          
            EcalTriggerID tid=geom.belongsTo(EcalID(pdigi.id()));

            if (!tid.null()) {
                int tot=0;
                if (pdigi.soi().isTOTComplete()) tot=pdigi.soi().tot();
                calc.addDigi(pdigi.id(),tid.raw(),pdigi.soi().adc_t(),tot);
            }
        }

        // Now, we compress the digis
        calc.compressDigis(9); // 9 is the number for Ecal...

        const std::map<unsigned int, uint8_t>& results=calc.compressedEnergies();
        HgcrocTrigDigiCollection tdigis;

        for (auto result : results) {
          if (result.second>0) {
            tdigis.push_back(HgcrocTrigDigi(result.first,result.second));
            //std::cout << EcalTriggerID(result.first) << "  " << tdigis.back() << std::endl;
          }
        }

        //std::cout << ecalDigis.size() << " " << tdigis.size() << std::endl;
        event.add(getName(),tdigis);
    }
}
DECLARE_PRODUCER_NS(ldmx, EcalTrigPrimDigiProducer);
