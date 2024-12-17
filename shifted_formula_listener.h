#ifndef SPREADSHEET_SHIFTEDFORMULALISTENER_H_
#define SPREADSHEET_SHIFTEDFORMULALISTENER_H_

#include <stack>
#include <string>

#include "common.h"
#include "formula.h"
#include "utils.h"
#include "FormulaBaseListener.h"

class ShiftedFormulaListener : public FormulaBaseListener {
 public:
  struct ShiftResult {
    IFormula::HandlingResult handling_result;
    FormulaInfo info;
  };

  ShiftedFormulaListener(const std::string &str,
                         OpType op_type,
                         ShiftType shift_type,
                         int first_idx,
                         int count);

  void exitUnaryOp(FormulaParser::UnaryOpContext *ctx) override;
  void exitParens(FormulaParser::ParensContext *ctx) override;
  void exitLiteral(FormulaParser::LiteralContext *ctx) override;
  void exitCell(FormulaParser::CellContext *ctx) override;
  void exitBinaryOp(FormulaParser::BinaryOpContext *ctx) override;

  ShiftResult ReleaseShiftResult();

 private:
  void ExitCellRowAddition(std::string text, Position pos);
  void ExitCellColAddition(std::string text, Position pos);
  void ExitCellRowDeletion(std::string text, Position pos);
  void ExitCellColDeletion(std::string text, Position pos);

  ShiftResult result_;

  std::stack<std::string> data_;
  OpType op_type_;
  ShiftType shift_type_;
  int first_idx_;
  int count_;
};

ShiftedFormulaListener CreateShiftedListener(
    const std::string &expr, OpType op_type, ShiftType shift_type,
    int first_idx, int count); // O(N), N – expr.size

#endif // SPREADSHEET_SHIFTEDFORMULALISTENER_H_
