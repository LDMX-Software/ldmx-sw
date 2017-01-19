#ifndef LDMXSW_FRAMEWORK_EXCEPTION_H_
#define LDMXSW_FRAMEWORK_EXCEPTION_H_

#include <exception>
#include <string>

namespace ldmxsw {

/** Standard base exception class with some useful output information
 */
class Exception : public std::exception {

    public:

        Exception() throw() { }

        Exception( const std::string& name, const std::string& message, const std::string& module, int line, const std::string& function ) : name_{name},message_{message},module_{module},function_{function},line_{line} { }

        virtual ~Exception() throw() { }
    
        const std::string& name() const throw() { return name_; }

        const std::string& message() const throw() { return message_; }

        const std::string& module() const throw() { return module_; }

        const std::string& function() const throw() { return function_; }
        
        int line() const throw() { return line_; }

        const char* what () const throw() { return message_.c_str(); }

    private:

        std::string name_, message_, module_, function_;
        int line_;
};

  
}

#define EXCEPTION_RAISE( EXCEPTION, MSG ) \
throw Exception( EXCEPTION, MSG, __FILE__, __LINE__, __FUNCTION__)


#endif
