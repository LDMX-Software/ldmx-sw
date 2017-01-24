/**
 * @file EcalVetoProcessor.h
 * @brief Class that performs basic ECal digi and determines if an event is vetoable
 * @author Owen Colegrove, UCSB
 */

#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom2.h"
#include "TClonesArray.h"
#include "Event/TriggerResult.h"

#include "Event/EcalHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"

using detdescr::DetectorID;
using detdescr::EcalDetectorID;
using detdescr::EcalHexReadout;


/**
 * @class EcalVetoProcessor
 * @brief Performs basic ECal digi and determines if event is vetoable
 */
class EcalVetoProcessor : public ldmxsw::Producer {

public:
    typedef std::pair<int, int>   layer_cell_pair;

    typedef std::pair<int, float> cell_energy_pair;

  EcalVetoProcessor(const std::string& name, ldmxsw::Process& process) : ldmxsw::Producer(name,process) { }
		      
  virtual void configure(const ldmxsw::ParameterSet&);

  virtual void produce(event::Event& event);


 private:
    event::TriggerResult result_;
    EcalDetectorID detID;
    bool verbose,doesPassVeto;
    EcalHexReadout* hexReadout;
    static const int numEcalLayers,numLayersForMedCal,backEcalStartingLayer;
    static const float totalDepCut,totalIsoCut,backEcalCut,ratioCut;

  inline layer_cell_pair hitToPair(event::EcalHit* hit){
        int detIDraw = hit->getID();
        detID.setRawValue(detIDraw);
        detID.unpack();
        int layer = detID.getFieldValue("layer");
        int cellid = detID.getFieldValue("cell");
        return (std::make_pair(layer, cellid));
    };

};


