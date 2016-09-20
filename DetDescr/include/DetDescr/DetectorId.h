#ifndef DETDESCR_DETECTORID_H_
#define DETDESCR_DETECTORID_H_ 1

// STL
#include <vector>

// LDMX
#include "DetDescr/IdField.h"

/**
 * Represents an ID in the detector with a raw, 32-bit value which can
 * be unpacked into a list of field values or packed from a list of field
 * values.
 */
class DetectorId {

    protected:

        /**
         * Protected constructor for sub-classes.
         */
        DetectorId();

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
        DetectorId(IdField::IdFieldList*);

        virtual ~DetectorId();

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
        IdField::IdFieldList* getFieldList();

        /**
         * Get the information for a field by name.
         */
        IdField* getField(const std::string&);

        /**
         * Get a field's value by name.
         */
        FieldValue getFieldValue(const std::string&);

    protected:

        RawValue rawValue;
        FieldValueList values;
        IdField::IdFieldMap idFieldMap;
        IdField::IdFieldList* idFieldList;
};

#endif
