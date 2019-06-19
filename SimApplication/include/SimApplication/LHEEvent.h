/**
 * @file LHEEvent.h
 * @brief Class defining an LHE event with a list of particles and information from the header block
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_LHEEVENT_H_
#define SIMAPPLICATION_LHEEVENT_H_

// LDMX
#include "SimApplication/LHEParticle.h"

// STL
#include <vector>

namespace ldmx {

    /**
     * @class LHEEvent
     * @brief LHE event with a list of particles and information from the header block
     *
     * @note
     * Detailed information on the Les Houches Event (LHE) format is provided here:
     * <a href="https://arxiv.org/abs/hep-ph/0609017">A standard format for Les Houches Event Files</a>.
     */
    class LHEEvent {

        public:

            /**
             * Class constructor.
             * @param data The string data of the event header.
             */
            LHEEvent(std::string& data);

            /**
             * Class destructor.
             */
            virtual ~LHEEvent();

            /**
             * Get the number of particles (NUP) in the event.
             * @return The number of particles in event.
             */
<<<<<<< HEAD
            int getNUP() const;
=======
            int getNUP();
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

            /**
             * Get the ID of the physics process (IDRUP).
             * @return The ID of the physics process.
             */
<<<<<<< HEAD
            int getIDPRUP() const;
=======
            int getIDPRUP();
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

            /**
             * Get the event weight (XWGTUP).
             * @return The event weight.
             */
<<<<<<< HEAD
            double getXWGTUP() const;
=======
            double getXWGTUP();
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

            /**
             * Get the scale Q of parton distributions (SCALUP).
             * @return The scale Q of parton distributions.
             */
<<<<<<< HEAD
            double getSCALUP() const;
=======
            double getSCALUP();
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

            /**
             * Get the value of the QED coupling (AQEDUP).
             * @return The value of the QED coupling.
             */
<<<<<<< HEAD
            double getAQEDUP() const;
=======
            double getAQEDUP();
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

            /**
             * Get the value of the QED coupling (AQCDUP).
             * @return The value of the QED coupling.
             */
<<<<<<< HEAD
            double getAQCDUP() const;


	    /**
	     * Set the vertex location (careful to match units as expected!)
             */
  	    void setVertex(double x, double y, double z);
	
	    /**
	     * Parse the vertex from a line of the form "#vertex [x] [y] [z]"
             */
	    void setVertex(const std::string& line);
	
	    /**
	     * Get the vertex location (careful to match units as expected!)
             * @return Array double[3] with x,y,z ordering
             */
	    const double* getVertex() const;
	    	
=======
            double getAQCDUP();

>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
            /**
             * Add a particle to the event.
             * @particle The particle to add.
             */
            void addParticle(LHEParticle* particle);

            /**
             * Get the list of particles in the event.
             * @return The list of particles in the event.
             */
            const std::vector<LHEParticle*>& getParticles();

        private:

            /**
             * Number of particles.
             */
            int nup_;

            /**
             * The physics process ID.
             */
            int idprup_;

            /**
             * The event weight.
             */
            double xwgtup_;

            /**
             * Scale Q of parton distributions.
             */
            double scalup_;

            /**
             * QCD coupling value.
             */
            double aqedup_;

            /**
             * QCD coupling value.
             */
            double aqcdup_;

<<<<<<< HEAD

	    /**
             * Vertex location
             */
	    double vtx_[3];
	
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
            /**
             * The list of particles.
             */
            std::vector<LHEParticle*> particles_;
    };

}

#endif
