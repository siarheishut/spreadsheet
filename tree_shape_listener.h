#ifndef SPREADSHEET_FORMULAAST_H_
#define SPREADSHEET_FORMULAAST_H_

#include <optional>
#include <stack>
#include <variant>

#include "FormulaBaseListener.h"

#include "common.h"
#include "sheet.h"

class FormulaEvaluatorListener : public FormulaBaseListener {
 public:
  explicit FormulaEvaluatorListener(const ISheet &sheet);

  void exitUnaryOp(FormulaParser::UnaryOpContext *ctx) override;
  void exitLiteral(FormulaParser::LiteralContext *ctx) override;
  void exitCell(FormulaParser::CellContext *ctx) override;
  void exitBinaryOp(FormulaParser::BinaryOpContext *ctx) override;

  std::variant<double, FormulaError> GetResult() const;

 private:
  const ISheet &sheet_;
  std::stack<double> data_;
  std::optional<FormulaError> error_;
};

#endif // SPREADSHEET_FORMULAAST_H_
