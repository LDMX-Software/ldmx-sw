#ifndef EVENT_EVENTIMPL_H_
#define EVENT_EVENTIMPL_H_

// ROOT
#include "TObject.h"
#include "TClonesArray.h"

class TTree;
class TBranch;

// STL
#include <string>
#include <map>

namespace event {

class EventImpl {

    public:

        static const char* TREE_NAME;

        EventImpl(const std::string& thePassName);

        void setITree(TTree* tree);

        void setOTree(TTree* tree);

        TTree* createTree();

        std::string makeBranchName(const std::string& collectionName, const std::string& passName) const {
            return collectionName + "_" + passName;
        }
        std::string makeBranchName(const std::string& collectionName) const {
            return makeBranchName(collectionName, passName_);
        }

        virtual ~EventImpl();

        /**
         * Adds a clones array to the event/tree.
         */
        void add(const std::string& collectionName, TClonesArray*);

        /**
         * Adds a general object to the event/tree.
         * Important notes: all objects _must_ implement/replace
         * TObject::Clone to simply call "new" and create an empty
         * new object and implement TObject::Copy to either copy the
         * contents of the object or _swap_ them to the calling
         * function, which is more efficient.
         */
        void add(const std::string& collectionName, TObject*);

        /**
         * These two methods only apply to the current pass of processing --
         * it is not allowed to modify any object from a previous pass.  They will
         * require null if there no object with that name in the current pass.
         */
        // TClonesArray* getMutable(const std::string& collectionName,const std::string& passName);
        // TObject* getMutable(const std::string& collectionName, const std::string& passName);

        const TObject* get(const std::string& collectionName, const std::string& passName);

        const TClonesArray* getCollection(const std::string& collectionName, const std::string& passName) {
            return (TClonesArray*) get(collectionName, passName);
        }

        bool nextEvent();

        void onEndOfEvent();

        void onEndOfFile();

    private:

        Long64_t entries_{-1};
        Long64_t ientry_ {-1};
        std::string passName_;
        TTree* otree_{nullptr};
        TTree* itree_{nullptr};

        mutable std::map<std::string, TBranch*> branches_;
        mutable std::map<std::string, TObject*> objects_;
        std::map<std::string, TObject*> objectsOwned_;

        std::vector<TBranch*> newBranches_;
};

}

#endif
