/**
 * @file Logger.cxx
 * Interface for ldmx-sw to spdlog
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Exception/Logger.h"

// STL
#include <fstream>
#include <ostream>
#include <iostream>

// Boost
#include <boost/core/null_deleter.hpp> //to avoid deleting std::cout
#include <boost/log/utility/setup/common_attributes.hpp> //for loading commont attributes

namespace ldmx {

    namespace logging {

        void open(int termV, int fileV, const std::string& fileName) {
    
            //some helpful types
            typedef sinks::text_ostream_backend ourSinkBack_t;
            typedef sinks::synchronous_sink< ourSinkBack_t > ourSinkFront_t;

            //allow our logs to access common attributes
            log::add_common_attributes();
    
            //get the core logging service
            boost::shared_ptr< log::core > core = log::core::get();

            //file sink is optional
            //  don't even make it if no fileName is provided 
            if ( not fileName.empty() ) {
                boost::shared_ptr< ourSinkBack_t > fileBack = boost::make_shared< ourSinkBack_t >();
                fileBack->add_stream(
                    boost::make_shared< std::ofstream >(
                        fileName
                        )
                    );
    
                boost::shared_ptr< ourSinkFront_t > fileSink 
                    = boost::make_shared< ourSinkFront_t >( fileBack );
    
                //TODO translate fileV to boost style severities
    
                //TODO change format to something helpful
    
                core->add_sink(fileSink);
            }
    
            //terminal sink is always created
            boost::shared_ptr< ourSinkBack_t > termBack = boost::make_shared< ourSinkBack_t >();
            termBack->add_stream(
                boost::shared_ptr< std::ostream >(
                    &std::cout, //point this stream to std::cout
                    boost::null_deleter() //don't let boost delete std::cout
                    )
                );
            //flushes message to screen **after each message**
            termBack->auto_flush( true );

            boost::shared_ptr< ourSinkFront_t > termSink 
                = boost::make_shared< ourSinkFront_t >( termBack );

            //TODO translate termV to boost style severities

            //TODO change format to something helpful

            core->add_sink(termSink);

            //allow everything through to the sinks for their own filtering
    
            return;

        } //open

        void close() {
            //prevents crashes on some systems when logging to a file
            log::core::get()->remove_all_sinks();

            return;
        }

    } //logging

} //ldmx

