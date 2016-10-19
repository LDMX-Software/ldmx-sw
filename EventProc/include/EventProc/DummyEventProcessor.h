#ifndef EventProc_DummyEventProcessor_h
#define EventProc_DummyEventProcessor_h

#include "EventProc/EventProcessor.h"

namespace eventproc {

class DummyEventProcessor : public EventProcessor {

    public:

        void initialize();

        void execute();

        void finish();

    private:

        int nProcessed{0};
};

}

#endif
