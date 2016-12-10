#include "SimApplication/EcalHitIO.h"

namespace sim {

void EcalHitIO::writeHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl) {
    int nHits = hc->GetSize();
    std::pair<std::map<LayerCellPair, int>::iterator, bool> isInserted;
    for (int iHit = 0; iHit < nHits; iHit++) {
        G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);

        std::pair<LayerCellPair, int> layer_cell_index =
                (std::make_pair(hitToPair(g4hit), outputColl->GetEntries()));

        isInserted = ecalReadoutMap.insert(layer_cell_index);
        SimCalorimeterHit* simHit;
        if (isInserted.second == false) {
            simHit = (SimCalorimeterHit*) outputColl->At(isInserted.first->second);
        } else {
            simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
        }
        g4hit->updateSimCalorimeterHit(simHit, !isInserted.second); /* copy data from G4 hit to readout hit */
    }
    ecalReadoutMap.clear();
}

EcalHitIO::LayerCellPair EcalHitIO::hitToPair(G4CalorimeterHit* g4hit) {
    int detIDraw = g4hit->getID();
    detID.setRawValue(detIDraw);
    detID.unpack();
    int layer = detID.getFieldValue("layer");
    int cellid = detID.getFieldValue("cell");
    return (std::make_pair(layer, cellid));
}

} // namespace sim
