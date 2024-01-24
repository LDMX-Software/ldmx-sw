#ifndef ISPYRECHITS_H
#define ISPYRECHITS_H

#include "TFile.h"
#include "TGButton.h"
#include "TGFrame.h"
#include "TGLViewer.h"
#include "TGTextEntry.h"
#include "TRint.h"
#include "TString.h"
#include "TTree.h"
#include "TBufferJSON.h"

#include "TEveBrowser.h"
#include "TEveElement.h"
#include "TEveEventManager.h"
#include "TEveManager.h"
#include "TEveViewer.h"

#include "TBranchElement.h"

#include "EventDisplay/EveDetectorGeometry.h"
#include "EventDisplay/Objects.h"

#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include <iostream>

namespace ldmx {
    class ISpyRecHits: public framework::Analyzer{
        public:
            ISpyRecHits(const std::string& name, framework::Process& process)
                : framework::Analyzer(name,process) {}

            virtual void analyze(const framework::Event& event);
    }
}


#endif