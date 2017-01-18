#ifndef EVENT_TRIGGERRESULT_H_
#define EVENT_TRIGGERRESULT_H_

// STL
#include <iostream>

// ROOT
#include "TObject.h"
#include "TString.h"
#include "TArrayD.h"

namespace event {

class TriggerResult : public TObject {

    public:

        /** Constructor */
        TriggerResult();

        /** Destructor */
        virtual ~TriggerResult();

        /** Print a description of this object */
        void Print(Option_t *option = "") const;

        /** Reset the TriggerResult object */
        void Clear(Option_t *option = "");

        /** Return the name of the trigger */        
        const TString& name() const { return name_; }

        /** Return pass/fail status of the trigger */
        bool passed() const { return pass_; }

        /** Return algorithm variable i (see algorithm code for details) */
        double algoVar(int element) const { return variables_[element]; }

        /** Set name and pass of trigger */
        void set(const TString& name, bool pass, int nvar); 

        /** Set array of variables for trigger */
        void setVar(int element, double value);

    private:

        TString name_{};
        bool pass_{false};
        TArrayD variables_;

        ClassDef(TriggerResult, 1);

};
}

#endif
