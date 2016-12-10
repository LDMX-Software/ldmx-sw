#ifndef DETDESCR_DETECTORID_H_
#define DETDESCR_DETECTORID_H_

// STL
#include <vector>

// LDMX
#include "DetDescr/IDField.h"

namespace detdescr {

/**
 * Represents an ID in the detector with a raw, 32-bit value which can
 * be unpacked into a list of field values or packed from a list of field
 * values.
 */
class DetectorID {

    protected:

        /**
         * Protected constructor for sub-classes.
         */
        DetectorID();

    public:

        /**
         * Definition of the raw value type.
         */
        typedef unsigned RawValue;

        /**
         * Definition of the field value type.
         */
        typedef unsigned FieldValue;

        /**
         * A list of field values.
         */
        typedef std::vector<FieldValue> FieldValueList;

        /**
         * Define a new detector ID from a list of field information.
         */
        DetectorID(IDField::IDFieldList*);

        /**
         * Class destructor, which will delete the field list and its objects.
         */
        virtual ~DetectorID();

        /**
         * Get the raw value of the detector ID.
         */
        RawValue getRawValue();

        /**
         * Set the raw value of the detector ID.
         */
        void setRawValue(RawValue);

        /**
         * Unpack the current raw value into a list.
         */
        const FieldValueList& unpack();

        /**
         * Pack the current list of field values into a raw value.
         */
        RawValue pack();

        /**
         * Get a field value by index.
         */
        FieldValue getFieldValue(int);

        /**
         * Set a field value by index in the field value list.
         */
        void setFieldValue(int, FieldValue);

        /**
         * Set a field value by its name.
         */
        void setFieldValue(const std::string&, FieldValue);

        /**
         * Get the list of field information.
         */
        IDField::IDFieldList* getFieldList();

        /**
         * Get the information for a field by name.
         */
        IDField* getField(const std::string&);

        /**
         * Get a field's value by name.
         */
        FieldValue getFieldValue(const std::string&);

    protected:

        /**
         * Set the list of fields that defines this ID.
         * @param fieldList The list of fields defining the ID.
         */
        void setFieldList(IDField::IDFieldList* fieldList);

        /**
         * Reinitialize the ID in case the field list changed.
         * This is called automatically by <i>setFieldList</i>.
         * Function should be called by a subclass if a new list
         * is set outside that method or if new fields are added
         * to the existing list.
         */
        void init();

    protected:

        RawValue rawValue_;
        FieldValueList fieldValues_;
        IDField::IDFieldMap fieldMap_;
        IDField::IDFieldList* fieldList_;
};

}

#endif
