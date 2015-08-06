/**
* Exception
*/

#pragma once

#include <exception>
#include <string>

namespace FluidSim {

class Exception : public std::exception {
public:
	Exception(const std::string & message)
			: errorMessage(message) { }

	virtual ~Exception() noexcept { }

	virtual const char * what() const noexcept {
		return errorMessage.c_str();
	}

private:
	std::string errorMessage;
};

} // end namespace FluidSim
