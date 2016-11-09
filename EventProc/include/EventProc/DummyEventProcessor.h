#ifndef EVENTPROC_DUMMYEVENTPROCESSOR_H_
#define EVENTPROC_DUMMYEVENTPROCESSOR_H_

#include "EventProc/EventProcessor.h"

namespace eventproc {

class DummyEventProcessor : public EventProcessor {

    public:

        void initialize();

        void execute();

        void finish();

    private:

        int nProcessed_{0};
};

}

#endif
