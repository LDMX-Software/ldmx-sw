/**
 * @file RunHeader.h
 * @brief Class encapsulating run information such as run #, detector etc.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _EVENT_RUN_HEADER_H_
#define _EVENT_RUN_HEADER_H_

//----------//
//   ROOT   //
//----------//
#include "TObject.h" //For ClassDef

//----------------//
//   C++ StdLib   //
//----------------//
#include <map>
#include <string>
#include <ostream>

//-------------//
//   ldmx-sw   //
//-------------//
#include "Event/Version.h"

namespace ldmx {

    class RunHeader {

        public:

            /**
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
             * @param name The name of the parameter.
             * @param value The value of the parameter.
             * @return The parameter value.
             */
            void setIntParameter(const std::string& name, int value) { 
                intParameters_[name] = value;
            }

            /**
             * Get a float parameter value.
             * 
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            float getFloatParameter(const std::string& name) {
                return floatParameters_[name];
            }

            /**
             * Set a float parameter value.
             * 
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            void setFloatParameter(const std::string& name, float value) {
                floatParameters_[name] = value;
            }

            /**
             * Get a string parameter value.
             * 
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            std::string getStringParameter(const std::string& name) {
                return stringParameters_[name];
            }

            /**
             * Set a string parameter value.
             * 
             * @param name The name of the parameter.
             * @return value The parameter value.
             */
            void setStringParameter(const std::string& name, std::string value) {
                stringParameters_[name] = value;
            }

            /** Print a string desciption of this object. */
            void Print(std::ostream& o) const;

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
