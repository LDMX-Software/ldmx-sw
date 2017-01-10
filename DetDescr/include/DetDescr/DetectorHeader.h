/**
 * @file DetectorHeader.h
 * @brief Class that defines a header with detector information
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef DETDESCR_DETECTORHEADER_H_
#define DETDESCR_DETECTORHEADER_H_

namespace detdescr {

/**
 * @class DetectorHeader
 * @brief Defines detector header information
 *
 * @note
 * This class provides information about a detector read in from a GDML file.
 * The information includes the name of the detector, its numerical (int) version,
 * a short description of the detector, and the name of its author.
 */
class DetectorHeader {

    public:

        /**
         * Class constructor.
         * @param name The name of the detector.
         * @param version The version of the detector.
         * @param description A short description of the detector.
         * @param author The name of the detector's author (first and last name).
         */
        DetectorHeader(std::string name, int version, std::string description, std::string author) :
            name_(name), version_(version), description_(description), author_(author) {
        }

        /**
         * Get the name of the detector.
         * @return The name of the detector.
         */
        const std::string& getName() {
            return name_;
        }

        /**
         * Get the version of the detector.
         * @return The version of the detector.
         */
        int getVersion() {
            return version_;
        }

        /**
         * Get a short description of the detector.
         * @return A short description of the detector.
         */
        const std::string& getDescription() {
            return description_;
        }

        /**
         * Get the author of the detector (first and last name).
         * @return The author of the detector.
         */
        const std::string& getAuthor() const {
            return author_;
        }

    private:

        /**
         * The name of the detector.
         */
        std::string name_;

        /**
         * The version of the detector.
         */
        int version_;

        /**
         * A short description of the detector.
         */
        std::string description_;

        /**
         * The name of the detector's author.
         */
        std::string author_;
};

}

#endif
