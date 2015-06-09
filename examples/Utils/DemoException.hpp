#pragma once

#include <exception>
#include <string>

class DemoException : public std::exception {
public:
	// Makes a copy of the message.
	DemoException(std::string message)
			: errorMessage(message) { }

	virtual ~DemoException() noexcept { }

	virtual const char * what() const noexcept {
		return errorMessage.c_str();
	}

private:
	std::string errorMessage;

};
