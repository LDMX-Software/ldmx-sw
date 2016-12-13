#include "SimApplication/EcalHitIO.h"

namespace sim {

void EcalHitIO::writeHitsCollection(G4CalorimeterHitsCollection* hc, TClonesArray* outputColl) {
    int nHits = hc->GetSize();
    std::pair<std::map<LayerCellPair, int>::iterator, bool> isInserted;
    for (int iHit = 0; iHit < nHits; iHit++) {
        G4CalorimeterHit* g4hit = (G4CalorimeterHit*) hc->GetHit(iHit);

        LayerCellPair layer_cell = hitToPair(g4hit);
        std::pair<LayerCellPair, int> layer_cell_index =
                std::make_pair(layer_cell, outputColl->GetEntries());

        isInserted = ecalReadoutMap.insert(layer_cell_index);
        SimCalorimeterHit* simHit;
        if (isInserted.second == false) {
            simHit = (SimCalorimeterHit*) outputColl->At(isInserted.first->second);
        } else {
            simHit = (SimCalorimeterHit*) outputColl->ConstructedAt(outputColl->GetEntries());
        }
        g4hit->updateSimCalorimeterHit(simHit, !isInserted.second); /* copy data from G4 hit to readout hit */

        /**
         * Set the XY position using the hex readout and Z from the G4 hit (volume center).
         */
        // FIXME: This check if the hit is new does not always seem to work.
        //if (!isInserted.second) {
        std::pair<float,float> XYPair = hexReadout->getCellCentroidXYPair(layer_cell.second);
        simHit->setPosition(XYPair.first, XYPair.second, simHit->getPosition()[2]);
        //}
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
