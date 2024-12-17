#ifndef SPREADSHEET__ERROR_HANDLE_LISTENER_H_
#define SPREADSHEET__ERROR_HANDLE_LISTENER_H_

#include "error_handle_listener.h"

#include <string>
#include "antlr4-runtime.h"
#include "sheet.h"
#include "FormulaBaseListener.h"

class HandleErrorsListener : public FormulaBaseListener {
 public:
  HandleErrorsListener(const ISheet &sheet);

  void exitUnaryOp(FormulaParser::UnaryOpContext *ctx) override;
  void exitParens(FormulaParser::ParensContext *ctx) override;
  void exitLiteral(FormulaParser::LiteralContext *ctx) override;
  void exitCell(FormulaParser::CellContext *ctx) override;
  void exitBinaryOp(FormulaParser::BinaryOpContext *ctx) override;

  std::string ReleaseResult();

 private:
  const ISheet &sheet_;

  std::stack<std::string> data_;
};

std::string HandleExprErrors(const std::string &expression,
                             const ISheet &sheet); // O(N), N – expression.size

#endif //SPREADSHEET__ERROR_HANDLE_LISTENER_H_
