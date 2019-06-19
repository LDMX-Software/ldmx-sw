/**
 * @file RunHeader.h
<<<<<<< HEAD
 * @brief Class encapsulating run information such as run #, detector etc.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _EVENT_RUN_HEADER_H_
#define _EVENT_RUN_HEADER_H_

//----------//
//   ROOT   //
//----------//
#include "TObject.h"

//----------------//
//   C++ StdLib   //
//----------------//
#include <map>
#include <string>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/Version.h"

namespace ldmx {

=======
 * @brief Class defining a run header providing information about a set of events
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENT_RUNHEADER_H_
#define EVENT_RUNHEADER_H_

// ROOT
#include "TObject.h"

// STL
#include <string>
#include <map>

namespace ldmx {

    /**
     * @class RunHeader
     * @brief Run header providing information about a set of events
     *
     * @note
     * This is a run header that can be used for describing sim or data events.
     * It provides the run number, name of the detector model used when events were created,
     * version of the detector used, and the type of the event branch.  The header
     * can also store named float, int or string parameters.
     */
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    class RunHeader : public TObject {

        public:

            /**
<<<<<<< HEAD
             * Constructor.
             *
             * @param runNumber The run number.
             * @param detectorName The name of the detector.
             * @param description A short description of the run.
             */
            RunHeader(int runNumber, std::string detectorName,  
                      std::string description); 

            /**
             * Default constructor.
             *
             * @note This exists for filling the object from a ROOT branch.
             */
            RunHeader() {}

            /** Destructor. */
            virtual ~RunHeader() {}

            /** @return The run number. */
            int getRunNumber() const { return runNumber_; }

            /** @return The name of the detector used to create the events. */
            const std::string& getDetectorName() const { return detectorName_; }

            /** 
             * @return The git SHA-1 associated with the software tag used
             * to generate this file.
             */
            const std::string& getSoftwareTag() const { return softwareTag_; }

            /** @return A short description of the run. */
            const std::string& getDescription() const { return description_; }

            /**
             * Get an int parameter value.
             *
             * @param name The name of the parameter.
             * @return The parameter value.
             */
            int getIntParameter(const std::string& name) { return intParameters_[name]; }

            /**
             * Set an int parameter value.
             * 
=======
             * Fully qualified class constructor (except for parameter values).
             * @param runNumber The run number.
             * @param detectorName The name of the detector.
             * @param detectorVersion The version of the detector.
             * @param description A short description of the run.
             */
            RunHeader(int runNumber, std::string detectorName, int detectorVersion, std::string description)
                : runNumber_(runNumber), detectorName_(detectorName), detectorVersion_(detectorVersion), description_(description) {
            }

            /**
             * No argument class constructor.
             *
             * @note
             * This exists for filling the object from a ROOT branch.
             */
            RunHeader() {
            }

            /**
             * Class destructor.
             */
            virtual ~RunHeader() {}

            /**
             * Get the run number.
             * @return The run number.
             */
            int getRunNumber() const {
                return runNumber_;
            }

            /**
             * Get the name of the detector used to create the events.
             * @return The name of the detector used to create the events.
             */
            const std::string& getDetectorName() const {
                return detectorName_;
            }

            /**
             * Get the version of the detector used to create the events.
             * @return The version of the detector used to create the events.
             */
            int getDetectorVersion() const {
                return detectorVersion_;
            }

            /**
             * Get a short description of the run.
             * @return A short description of the run.
             */
            const std::string& getDescription() const {
                return description_;
            }

            /**
             * Get an int parameter value.
             * @param name The name of the parameter.
             * @return The parameter value.
             */
            int getIntParameter(const std::string& name) {
                return intParameters_[name];
            }

            /**
             * Set an int parameter value.
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
             * @param name The name of the parameter.
             * @param value The value of the parameter.
             * @return The parameter value.
             */
<<<<<<< HEAD
            void setIntParameter(const std::string& name, int value) { 
=======
            void setIntParameter(const std::string& name, int value) {
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
                intParameters_[name] = value;
            }

            /**
             * Get a float parameter value.
<<<<<<< HEAD
             * 
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            float getFloatParameter(const std::string& name) {
                return floatParameters_[name];
            }

            /**
             * Set a float parameter value.
<<<<<<< HEAD
             * 
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            void setFloatParameter(const std::string& name, float value) {
                floatParameters_[name] = value;
            }

            /**
             * Get a string parameter value.
<<<<<<< HEAD
             * 
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            std::string getStringParameter(const std::string& name) {
                return stringParameters_[name];
            }

            /**
             * Set a string parameter value.
<<<<<<< HEAD
             * 
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            void setStringParameter(const std::string& name, std::string value) {
                stringParameters_[name] = value;
            }

<<<<<<< HEAD
            /** Print a string desciption of this object. */
=======
            /**
             * Print information about this object.
             */
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
            void Print(Option_t *option = "") const;

            /**
             * Copy this object.
<<<<<<< HEAD
             * 
             * @param o The target object.
             */
            void Copy(TObject& o) const { ((RunHeader&) o) = *this; }

        private:

            /** Run number. */
            int runNumber_{0};

            /** Detector name. */
            std::string detectorName_{""};

            /** Run description. */
            std::string description_{""};

            /** 
             * git SHA-1 hash associated with the software tag used to generate
             * this file.
             */
            std::string softwareTag_{GIT_SHA1}; 

            /** Map of int parameters. */
            std::map<std::string, int> intParameters_;

            /** Map of float parameters. */
            std::map<std::string, float> floatParameters_;

            /** Map of string parameters. */
            std::map<std::string, std::string> stringParameters_;

            ClassDef(RunHeader, 2);

    }; // RunHeader

} // ldmx

#endif // _EVENT_RUN_HEADER_H_ 
=======
             * @param o The target object.
             */
            void Copy(TObject& o) const {
                ((RunHeader&) o) = *this;
            }

        private:

            /**
             * The run number.
             */
            int runNumber_{0};

            /**
             * The detector name.
             */
            std::string detectorName_{""};

            /**
             * The detector version.
             */
            int detectorVersion_{0};

            /**
             * The run description.
             */
            std::string description_{""};

            /**
             * The int parameters.
             */
            std::map<std::string, int> intParameters_;

            /**
             * The float parameters.
             */
            std::map<std::string, float> floatParameters_;

            /**
             * The string parameters.
             */
            std::map<std::string, std::string> stringParameters_;

            ClassDef(RunHeader, 1);
    };

}

#endif /* EVENT_RUNHEADER_H_ */
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
