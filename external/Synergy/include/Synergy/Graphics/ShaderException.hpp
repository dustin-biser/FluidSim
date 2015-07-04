/**
* ShaderException
*/

#pragma once

#include <exception>
#include <string>

namespace Synergy {

using std::string;

    class ShaderException : public std::exception {
    public:
        ShaderException(const string & message)
            : errorMessage(message) { }

        virtual ~ShaderException() noexcept { }

        virtual const char * what() const noexcept {
            return errorMessage.c_str();
        }

    private:
        string errorMessage;
    };

} // end namespace Synergy
