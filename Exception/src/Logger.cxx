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

        logger makeLogger( const std::string& name ) {
            logger lg( log::keywords::channel = name); //already has severity built in
            return boost::move(lg);
        }

        void open(const std::vector<std::string>& loggingArgs) {

            //parse the CLI parameters
            //  defaults:
            int termLevelInt = 2, fileLevelInt = 5;
            std::string fileName = "ldmx_app.log"; 
            for ( auto it = loggingArgs.begin(); it != loggingArgs.end(); it++ ) {
                std::string currArg = *it;
                if ( currArg == "-f" or currArg == "--fileLog" ) {
                    //print ldmx-sw logging messages to a file
                    auto nextArg = std::next(it,1);
                    if ( nextArg != loggingArgs.end() and not nextArg->empty() ) {
                        //next argument exists
                        if ( strstr( nextArg->c_str() , ".log" ) ) fileName = *nextArg;
                        else if ( isdigit( (*nextArg)[0] ) ) fileLevelInt = atoi( nextArg->c_str() );

                        //second argument to -f exists ==> check if log file name
                        auto nextNextArg = std::next(it,2);
                        if ( nextNextArg != loggingArgs.end() 
                                and not nextNextArg->empty() 
                                and strstr( nextArg->c_str() , ".log" ) ) fileName = *nextNextArg;
                    } else {
                        // no other args ==> set to maximum output
                        fileLevelInt = 0;
                    }
                } else if ( currArg == "-v" or currArg == "--verbosity" ) {
                    //check for next arg being verbosity integer
                    auto nextArg = std::next(it,1);
                    //next argument could be the integer level for terminal logging
                    if ( nextArg != loggingArgs.end() 
                            and not nextArg->empty() 
                            and isdigit( (*nextArg)[0] ) ) termLevelInt = atoi( nextArg->c_str() );
                    else termLevelInt = 0; //no setting number ==> set to maximum output
                }
            } //loop through command line args
        
            //check parameters
            if ( termLevelInt < 0 ) termLevelInt = 0;
            if ( fileLevelInt < 0 ) fileLevelInt = 0;

            //some helpful types
            typedef sinks::text_ostream_backend ourSinkBack_t;
            typedef sinks::synchronous_sink< ourSinkBack_t > ourSinkFront_t;

            //allow our logs to access common attributes
            log::add_common_attributes();
    
            //get the core logging service
            boost::shared_ptr< log::core > core = log::core::get();

            //file sink is optional
            //  don't even make it if no fileName is provided 
            if ( fileLevelInt < 5 ) {
                boost::shared_ptr< ourSinkBack_t > fileBack = boost::make_shared< ourSinkBack_t >();
                fileBack->add_stream(
                    boost::make_shared< std::ofstream >(
                        fileName
                        )
                    );
    
                boost::shared_ptr< ourSinkFront_t > fileSink 
                    = boost::make_shared< ourSinkFront_t >( fileBack );
    
                //translate integer level to enum
                fileSink->set_filter(
                        log::expressions::attr<level>("Severity") >= level(fileLevelInt)
                        );
    
                //Currently:
                //  TimeStamp [ Channel ] severity : message
                fileSink->set_formatter(
                    [](const log::record_view &view, log::formatting_ostream &os ) {
                        os 
                            << log::extract<boost::posix_time::ptime>( "TimeStamp" , view )
                            << " [ " << log::extract<std::string>( "Channel", view ) << " ] "
                            << /*humanReadableLevel.at*/(log::extract<level>( "Severity" , view ))
                            << " : " << view[log::expressions::smessage];
                    }
                );

                core->add_sink(fileSink);
            } //file set to pass something

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

            //translate integer level to enum
            level passingLevel = termLevelInt > 4 ? fatal : level(termLevelInt);
            termSink->set_filter(
                    log::expressions::attr<level>("Severity") >= passingLevel
                    );

            //Currently:
            //  TimeStamp [ Channel ] severity : message
            termSink->set_formatter(
                [](const log::record_view &view, log::formatting_ostream &os ) {
                    os 
                        << log::extract<boost::posix_time::ptime>( "TimeStamp" , view )
                        << " [ " << log::extract<std::string>( "Channel", view ) << " ] "
                        << /*humanReadableLevel.at*/(log::extract<level>( "Severity" , view ))
                        << " : " << view[log::expressions::smessage];
                }
            );

            core->add_sink(termSink);
    
            return;

        } //open

        void close() {
            //prevents crashes on some systems when logging to a file
            log::core::get()->remove_all_sinks();

            return;
        }

    } //logging

} //ldmx

