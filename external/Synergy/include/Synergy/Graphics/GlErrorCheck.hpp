/**
 * GlErrorCheck
 */

#pragma once

#include <string>

#if(DEBUG)
#define CHECK_GL_ERRORS Synergy::checkGLErrors(__FILE__, __LINE__)
#define CHECK_FRAMEBUFFER_COMPLETENESS Synergy::checkFramebufferCompleteness()
#else
#define CHECK_GL_ERRORS
#define CHECK_FRAMEBUFFER_COMPLETENESS
#endif

namespace Synergy {

    void checkGLErrors(const std::string & currentFileName, int currentLineNumber);

    void checkFramebufferCompleteness();

}
