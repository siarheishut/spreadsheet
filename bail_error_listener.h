#ifndef SPREADSHEET_BAIL_ERROR_LISTENER_H_
#define SPREADSHEET_BAIL_ERROR_LISTENER_H_

#include <exception>
#include <string>

#include "antlr4-runtime.h"

class BailErrorListener : public antlr4::BaseErrorListener {
 public:
  void syntaxError(antlr4::Recognizer * /* recognizer */,
                   antlr4::Token * /* offendingSymbol */, size_t /* line */,
                   size_t /* charPositionInLine */, const std::string &msg,
                   std::exception_ptr /* e */) override;
};

#endif // SPREADSHEET_BAIL_ERROR_LISTENER_H_
