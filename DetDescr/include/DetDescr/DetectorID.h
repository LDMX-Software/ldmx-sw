/**
 * @file DetectorID.h
 * @brief Class that defines a 32-bit packed ID for uniquely identifying hits and detector components
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_DETECTORID_H_
#define DETDESCR_DETECTORID_H_

// STL
#include <vector>

// LDMX
#include "DetDescr/IDField.h"

namespace ldmx {

    /**
     * @class DetectorID
     * @brief Defines a 32-bit packed ID for uniquely identifying hits and detector components
     *
     * @note
     * Represents an ID in the detector with a raw, 32-bit value which can
     * be unpacked into a list of field values or packed from a list of field
     * values.
     */
    class DetectorID {

        protected:

            /**
             * Class constructor which is protected for use by sub-classes.
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
             * @param fieldList The list of fields.
             */
            DetectorID(IDField::IDFieldList* fieldList);

            /**
             * Class destructor, which will delete the field list and its objects.
             */
            virtual ~DetectorID();

            /**
             * Get the raw value of the detector ID.
             * @return The raw value.
             */
            RawValue getRawValue();

            /**
             * Set the raw value of the detector ID.
             * @param rawValue The raw value of the ID.
             */
            void setRawValue(RawValue rawValue);

            /**
             * Unpack the current raw value into a list.
             * @return The list of unpacked values.
             */
            const FieldValueList& unpack();

            /**
             * Pack the current list of field values into a raw value.
             * @return The packed ID.
             */
            RawValue pack();

            /**
             * Decode and return a field's value from the raw ID.
             * @param i The index of the field value.
             */
            FieldValue getFieldValue(int i);

            /**
             * Set a field value by index in the field value list.
             * @param i The index of the field value.
             * @param value The new field value.
             */
            void setFieldValue(int i, FieldValue value);

            /**
             * Set a field value by its name.
             * @param fieldName The name of the field.
             * @param fieldValue The new value of the field.
             */
            void setFieldValue(const std::string& fieldName, FieldValue fieldValue);

            /**
             * Get the list of field information.
             * @return The list of field information.
             */
            IDField::IDFieldList* getFieldList();

            /**
             * Get the information for a field by name.
             * @return The information of the field.
             *
             */
            IDField* getField(const std::string& fieldName);

            /**
             * Decode and return a field's value by name (e.g. "layer").
             * @return The value of the field.
             */
            FieldValue getFieldValue(const std::string& fieldName);

        protected:

            /**
             * Set the list of fields that defines this ID.
             * @param fieldList The list of fields defining the ID.
             */
            void setFieldList(IDField::IDFieldList* fieldList);

            /**
             * Reinitialize the ID in case the field list changed.
             * This is called automatically by <i>setFieldList</i>.
             * Function should be called by a subclass if a new field list
             * is set outside that method or if new fields are added
             * to the existing list.
             */
            void init();

        protected:

            /**
             * The raw, packed value of the ID.
             */
            RawValue rawValue_;

            /**
             * The unpacked list of field values.
             */
            FieldValueList fieldValues_;

            /**
             * The map of names to field information.
             */
            IDField::IDFieldMap fieldMap_;

            /**
             * The list of field information.
             */
            IDField::IDFieldList* fieldList_;
    };

}

#endif
