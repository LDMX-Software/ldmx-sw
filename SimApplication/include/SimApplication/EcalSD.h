#ifndef SIMAPPLICATION_ECALSD_H_
#define SIMAPPLICATION_ECALSD_H_

// LDMX
#include "Event/Event.h"
#include "SimApplication/CalorimeterSD.h"

// ROOT
#include "TH2Poly.h"
#include "TMath.h"
using detdescr::DetectorID;
using event::Event;

namespace sim {

class EcalSD : public CalorimeterSD {

    public:

		EcalSD(G4String name,
				G4String theCollectionName,
				int subdet,
				DetectorID* detID);

		virtual ~EcalSD();

		G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist);

		void initialiseMap(const double width, const double side);

		void buildMap(Double_t xstart,
				Double_t ystart, //Map starting points
				Double_t a,  // side length
				Int_t k,     // # hexagons in a column
				Int_t s);
		inline TH2Poly * getMap(){
			return map;
		};
    private:
		TH2Poly *map;
};

}

#endif
