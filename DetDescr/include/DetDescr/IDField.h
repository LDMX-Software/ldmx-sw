/**
 * @file IDField.h
 * @brief Class that defines a single field in a bit-packed detector ID
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */
#ifndef DETDESCR_IDFIELD_H_
#define DETDESCR_IDFIELD_H_

// STL
#include <string>
#include <map>
#include <vector>

namespace ldmx {

/**
 * @class IDField
 * @brief Provides information about a field within a DetectorID
 */
class IDField {

    public:

        /**
         * Map of name to field.
         */
        typedef std::map<std::string, IDField*> IDFieldMap;

        /**
         * List of fields.
         */
        typedef std::vector<IDField*> IDFieldList;

        /**
         * Class constructor.
         * @param name The name of the field.
         * @param index The index of the field in the ID.
         * @param startBit The start bit of the field.
         * @param endBit The end bit of the field.
         */
        IDField(std::string name, unsigned index, unsigned startBit, unsigned endBit);

        /**
         * Get the name of the field.
         * @return The name of the field.
         */
        const std::string& getFieldName();

        /**
         * Get the index of the field.
         * @return The index of the field.
         */
        unsigned getIndex();

        /**
         * Get the start bit of the field.
         * @return The start bit of the field.
         */
        unsigned getStartBit();

        /**
         * Get the end bit of the field.
         * @return The end bit of the field.
         */
        unsigned getEndBit();

        /**
         * Get a bit mask for this field.
         * @return A bit mask for this field.
         */
        unsigned getBitMask();

    private:

        /**
         * Utility for creating a bit mask from a start to end bit.
         * @param startBit The start bit.
         * @param endBit The end bit.
         */
        static unsigned createBitMask(unsigned startBit, unsigned endBit);

    private:

        /**
         * The name of the field.
         */
        std::string fieldName;

        /**
         * The index of the field.
         */
        unsigned index;

        /**
         * The start bit of the field.
         */
        unsigned startBit;

        /**
         * The end bit of the field.
         */
        unsigned endBit;

        /**
         * The bit mask of the field.
         */
        unsigned bitMask;
};

}

#endif
