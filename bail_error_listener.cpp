#include "bail_error_listener.h"

#include <exception>
#include <string>

#include "antlr4-runtime.h"

void BailErrorListener::syntaxError(antlr4::Recognizer * /* recognizer */,
                                    antlr4::Token * /* offendingSymbol */,
                                    size_t /* line */,
                                    size_t /* charPositionInLine */,
                                    const std::string &msg,
                                    std::exception_ptr /* e */) {
  throw std::runtime_error("Error when lexing: " + msg);
}