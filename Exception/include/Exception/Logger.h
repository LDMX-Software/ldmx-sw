/**
 * @file Logger.h
 * Interface for ldmx-sw to boost logging
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EXCEPTION_LOGGER_H
#define EXCEPTION_LOGGER_H

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace ldmx {

    namespace logging = boost::log;
    namespace sinks = boost::log::sinks;
    namespace keywords = boost::log::keywords;

    /**
     * Initialize the logging
     *
     * @param termV verbosity to be output to terminal
     * @param fileV verbosity to be output to file
     */
    void initLog(int termV, int fileV);

}

#define ldmx_log(severity)                                          \
    logging::record rec = ldmxLogger::get().open_record( severity ) \
    if ( rec ) {                                                    \
        ldmx::logging::record_ostream strm(rec);                    \
        strm << msg;                                                \
        strm.flush();                                               \
        ldmxLogger::get().push_record(boost::move(rec));            \
    }

#endif //EXCEPTION_LOGGER_H
