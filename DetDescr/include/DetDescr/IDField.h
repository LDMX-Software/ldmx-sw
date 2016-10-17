#ifndef DetDescr_IDField_h
#define DetDescr_IDField_h

// STL
#include <string>
#include <map>
#include <vector>

class IDField {

    public:

        typedef std::map<std::string, IDField*> IDFieldMap;
        typedef std::vector<IDField*> IDFieldList;

        IDField(std::string, unsigned index, unsigned startBit, unsigned endBit);

        const std::string& getFieldName();

        unsigned getIndex();

        unsigned getStartBit();

        unsigned getEndBit();

        unsigned getBitMask();

    private:

        static unsigned createBitMask(unsigned, unsigned);

    private:

        std::string fieldName;
        unsigned index;
        unsigned startBit;
        unsigned endBit;
        unsigned bitMask;
};

#endif
