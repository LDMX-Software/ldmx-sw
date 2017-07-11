#include "DetDescr/DetectorElement.h"
#include "DetDescr/EcalDetectorElement.h"

#include <typeinfo>
#include <iostream>

using namespace ldmx;

int main(int, const char* argv[]) {
    DetectorElementFactory* fac = DetectorElementFactory::instance();
    DetectorElement* de = fac->create("EcalDetectorElement");
}
