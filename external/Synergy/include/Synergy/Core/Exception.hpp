/**
* Exception
*
* @brief For throwing exceptions within Synergy with useful error messages.
*/

#pragma once

#include <exception>
#include <string>

namespace Synergy {
  
  using std::string;
  
  class Exception : public std::exception {
  public:
	  // Makes a copy of the message.
	  Exception(string message)
			  : errorMessage(message) { }

	  virtual ~Exception() noexcept { }

	  virtual const char * what() const noexcept {
		  return errorMessage.c_str();
	  }

  private:
	  string errorMessage;

  };
  
}
