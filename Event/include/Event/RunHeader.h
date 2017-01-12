/**
 * @file RunHeader.h
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

namespace event {

/**
 * @name RunHeader
 * @brief Run header providing information about a set of events
 *
 * @note
 * This is a run header that can be used for describing sim or data events.
 * It provides the run number, name of the detector model used when events were created,
 * version of the detector used, and the type of the event branch.  The header
 * can also store named float, int or string parameters.
 */
class RunHeader : public TObject {

    public:

        /**
         * Fully qualified class constructor (except for parameter values).
         * @param runNumber The run number.
         * @param detectorName The name of the detector.
         * @param detectorVersion The version of the detector.
         * @param eventType The type of the events in the file (C++ class name).
         * @param description A short description of the run.
         */
        RunHeader(int runNumber, std::string detectorName, int detectorVersion, std::string description)
            : runNumber_(runNumber),
              detectorName_(detectorName),
              detectorVersion_(detectorVersion),
              description_(description) {
        }

        /**
         * No argument class constructor.
         *
         * @note
         * This exists for filling the object from a ROOT branch.
         */
        RunHeader() {;}

        /**
         * Class destructor.
         */
        virtual ~RunHeader() {;}

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
         * @param The name of the parameter.
         * @return The parameter value.
         */
        int getIntParameter(const std::string& name) {
            return intParameters_[name];
        }

        /**
         * Set an int parameter value.
         * @param The name of the parameter.
         * @return The parameter value.
         */
        void setIntParameter(const std::string& name, int value) {
            intParameters_[name] = value;
        }

        /**
         * Get a float parameter value.
         * @param The name of the parameter.
         * @return The parameter value.
         */
        float getFloatParameter(const std::string& name) {
            return floatParameters_[name];
        }

        /**
         * Set a float parameter value.
         * @param The name of the parameter.
         * @return The parameter value.
         */
        void setFloatParameter(const std::string& name, float value) {
            floatParameters_[name] = value;
        }

        /**
         * Get a string parameter value.
         * @param The name of the parameter.
         * @return The parameter value.
         */
        std::string getStringParameter(const std::string& name) {
            return stringParameters_[name];
        }

        /**
         * Set a string parameter value.
         * @param The name of the parameter.
         * @return The parameter value.
         */
        void setStringParameter(const std::string& name, std::string value) {
            stringParameters_[name] = value;
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
