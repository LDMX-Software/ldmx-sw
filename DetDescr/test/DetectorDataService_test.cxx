#include "DetDescr/DetectorDataServiceImpl.h"

using namespace ldmx;

int main(int, const char* argv[])  {
    DetectorDataServiceImpl* svc = new DetectorDataServiceImpl();
    svc->setDetectorName("ldmx-det-full-v1-fieldmap");
    svc->initialize();
}
