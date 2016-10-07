/**
 *
 * @file SimTrackerHit.h
 * @brief Class used to encapsulate information from a hit in a tracking 
 *        detector.
 * @author
 *
 */

#ifndef EVENT_SIMTRACKERHIT_H_
#define EVENT_SIMTRACKERHIT_H_ 

// ROOT
#include "TObject.h"
#include "TRef.h"

// LDMX
#include "Event/SimParticle.h"

class SimTrackerHit: public TObject {

    public:

        /** Constructor */
        SimTrackerHit();
        
        /** Destructor */
        virtual ~SimTrackerHit();

        void Print(Option_t *option = "") const;

        /**
         *
         */
        int getID() const { return id; };

        /**
         * Get the global position of the tracker hit.
         *
         * @return A vector containing the coordinates of the hit.
         */
        std::vector<double> getPosition() const;

        //double* getStartPosition();

        //double* getEndPosition();

        /**
         *
         */
        float getEdep() const { return edep; };

        /**
         *
         */
        float getTime() const { return time; };

        /** 
         * Get the momentum.
         *
         * @return A vector containing the momentum components.
         */
        std::vector<double> getMomentum() const;
        
        //float* getMomentum();

        /**
         *
         */
        SimParticle* getSimParticle();

        /**
         *
         */
        void setID(const long id) { this->id = id; };

        //void setStartPosition(double x, double y, double z);

        //void setEndPosition(double x, double y, double z);
        
        /**
         *
         */
        void setEdep(const float edep) { this->edep = edep; };

        /**
         *
         */
        void setTime(const float time) { this->time; };
        
        /**
         *
         */
        void setMomentum(const float px, const float py, const float pz);

        void setSimParticle(SimParticle* simParticle);

    private:

        /** */
        long id{0};
        //double startPosition[3];
        //double endPosition[3];
        
        /** */
        float edep{0};

        /** */
        float time{0};
        
        /** */
        float px{0};

        /** */
        float py{0};

        /** */
        float pz{0};

        /** */
        float x{0};
        
        /** */
        float y{0};
        
        /** */
        float z{0};

        /** */
        TRef simParticle{nullptr};

        ClassDef(SimTrackerHit, 1);
};

#endif
