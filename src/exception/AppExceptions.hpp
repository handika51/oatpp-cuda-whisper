#ifndef AppExceptions_hpp
#define AppExceptions_hpp

#include <stdexcept>
#include <string>

namespace app { namespace exception {

class AudioProcessingException : public std::runtime_error {
public:
    AudioProcessingException(const std::string& message) : std::runtime_error(message) {}
};

class ValidationException : public std::runtime_error {
public:
    ValidationException(const std::string& message) : std::runtime_error(message) {}
};

}}

#endif