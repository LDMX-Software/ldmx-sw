#ifndef DETDESCR_ECALHEXREADOUT_H_
#define DETDESCR_ECALHEXREADOUT_H_

// STL
#include <map>
#include <iostream>

// ROOT
#include "TH2Poly.h"

namespace detdescr {

class EcalHexReadout {

    public:

        EcalHexReadout(const double width = 1000, const double side = 4.59360);


		inline TH2Poly * getMap(){
			return ecalMap;
		};
        inline int getCellId(float x, float y){
            return ecalMap->FindBin(x,y);
        };
        inline std::pair<float,float> getCellCentroidXYPair(int cellId){
            std::pair<std::map<int,std::pair<float,float>>::iterator, bool> isInserted;
            if (!isInserted.second) {
                std::cout << "This cellId does not exist, returning (-1,-1)" << std::endl;
                return std::make_pair<float,float>(-1.,-1.);
            }
            return isInserted.first->second;
        };
    private:

        void buildMap(Double_t xstart,
                Double_t ystart, //Map starting points
                Double_t a,  // side length
                Int_t k,     // # hexagons in a column
                Int_t s);
        TH2Poly *ecalMap;
        std::map<int,std::pair<float,float>> cellIdtoCoords;
};

}

#endif
