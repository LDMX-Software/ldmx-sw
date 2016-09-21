#ifndef DETDESCR_IDFIELDDESCR_H_
#define DETDESCR_IDFIELDDESCR_H_ 1

// STL
#include <string>
#include <map>
#include <vector>

class IdField {

    public:

        typedef std::map<std::string, IdField*> IdFieldMap;
        typedef std::vector<IdField*> IdFieldList;

        IdField(std::string, unsigned index, unsigned startBit, unsigned endBit);

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
