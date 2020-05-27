#include "DetDescr/TrigScintID.h" 

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/IDField.h"


namespace ldmx { 

    TrigScintID::TrigScintID() 
        : DetectorID() {

            auto fieldList{new IDField::IDFieldList()}; 
            fieldList->push_back(new IDField("module", 0, 0, 4)); 
            fieldList->push_back(new IDField("bar", 1, 5, 14)); 
            setFieldList(fieldList); 
        }

    TrigScintID::~TrigScintID() {}

}
