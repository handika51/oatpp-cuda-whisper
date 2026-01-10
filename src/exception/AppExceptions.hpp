#ifndef AppExceptions_hpp
#define AppExceptions_hpp

#include <stdexcept>
#include <string>

class AudioProcessingException : public std::runtime_error {
public:
    AudioProcessingException(const std::string& message) : std::runtime_error(message) {}
};

#endif