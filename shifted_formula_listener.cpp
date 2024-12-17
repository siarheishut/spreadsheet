#include "shifted_formula_listener.h"

#include <stdexcept>
#include <string>

#include "common.h"
#include "formula.h"
#include "utils.h"
#include "FormulaParser.h"

ShiftedFormulaListener::ShiftedFormulaListener(
    const std::string &expr, OpType op_type, ShiftType shift_type,
    int first_idx, int count)
    : result_({
                  .handling_result = IFormula::HandlingResult::NothingChanged,
                  .info = FormulaInfo{.expr = expr}}),
      op_type_(op_type),
      shift_type_(shift_type),
      first_idx_(first_idx),
      count_(count) {
}

void ShiftedFormulaListener::exitUnaryOp(FormulaParser::UnaryOpContext *ctx) {
  if (data_.empty()) {
    throw std::runtime_error(
        "ShiftedFormulaListener: exitUnaryOp: no data: failed precondition");
  }

  auto &node = data_.top();
  if (ctx->ADD()) {
    node = '+' + node;
  } else {
    node = '-' + node;
  }
}

void ShiftedFormulaListener::exitParens(FormulaParser::ParensContext *ctx) {
  if (data_.empty()) {
    throw std::runtime_error(
        "ShiftedFormulaListener: exitParens: no data: failed precondition");
  }

  auto &node = data_.top();
  node = '(' + node + ')';
}

void ShiftedFormulaListener::exitLiteral(FormulaParser::LiteralContext *ctx) {
  data_.push(ctx->getText());
}

void ShiftedFormulaListener::exitCell(FormulaParser::CellContext *ctx) {
  auto text = ctx->getText();
  auto pos = Position::FromString(text);
  switch (shift_type_) {
    case ShiftType::kRows:
      switch (op_type_) {
        case OpType::kDeletion:
          ExitCellRowDeletion(std::move(text), pos);
          break;
        case OpType::kAddition:
          ExitCellRowAddition(std::move(text), pos);
          break;
        default:
          break;
      }
      break;
    case ShiftType::kCols:
      switch (op_type_) {
        case OpType::kDeletion:
          ExitCellColDeletion(std::move(text), pos);
          break;
        case OpType::kAddition:
          ExitCellColAddition(std::move(text), pos);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

void ShiftedFormulaListener::exitBinaryOp(FormulaParser::BinaryOpContext *ctx) {
  if (data_.empty()) {
    throw std::runtime_error(
        "ShiftedFormulaListener: exitBinaryOp: no data: failed precondition");
  }

  auto rhs = std::move(data_.top());
  data_.pop();

  if (data_.empty()) {
    throw std::runtime_error(
        "ShiftedFormulaListener: exitBinaryOp 2: no data: failed precondition");
  }

  auto lhs = std::move(data_.top());
  data_.pop();
  if (ctx->ADD()) {
    data_.push(std::move(lhs) + '+' + std::move(rhs));
  } else if (ctx->SUB()) {
    data_.push(std::move(lhs) + '-' + std::move(rhs));
  } else if (ctx->MUL()) {
    data_.push(std::move(lhs) + '*' + std::move(rhs));
  } else {
    data_.push(std::move(lhs) + '/' + std::move(rhs));
  }
}

ShiftedFormulaListener::ShiftResult
ShiftedFormulaListener::ReleaseShiftResult() {
  if (data_.empty()) {
    throw std::runtime_error(
        "ShiftedFormulaListener: no data: failed precondition");
  }

  result_.info.expr = std::move(data_.top());
  data_.pop();
  return std::move(result_);
}

void ShiftedFormulaListener::ExitCellRowAddition(
    std::string text, Position pos) {
  if (pos.row < first_idx_) {
    data_.push(std::move(text));
    result_.info.referenced_cells.push_back(pos);
  } else {
    pos.row += count_;
    data_.push(pos.ToString());
    result_.info.referenced_cells.push_back(pos);
    result_.handling_result = IFormula::HandlingResult::ReferencesRenamedOnly;
  }
}

void ShiftedFormulaListener::ExitCellColAddition(
    std::string text, Position pos) {
  if (pos.col < first_idx_) {
    data_.push(std::move(text));
    result_.info.referenced_cells.push_back(pos);
  } else {
    pos.col += count_;
    data_.push(pos.ToString());
    result_.info.referenced_cells.push_back(pos);
    result_.handling_result = IFormula::HandlingResult::ReferencesRenamedOnly;
  }
}

void ShiftedFormulaListener::ExitCellRowDeletion(
    std::string text, Position pos) {
  if (pos.row >= first_idx_ && pos.row < first_idx_ + count_) {
    data_.emplace(kInvalidPosStr);
    result_.handling_result = IFormula::HandlingResult::ReferencesChanged;
  } else if (text == kInvalidPosStr) {
    data_.emplace(kInvalidPosStr);
  } else if (pos.row < first_idx_) {
    data_.push(std::move(text));
    result_.info.referenced_cells.push_back(pos);
  } else {
    pos.row -= count_;
    data_.push(pos.ToString());
    result_.info.referenced_cells.push_back(pos);
    if (result_.handling_result == IFormula::HandlingResult::NothingChanged)
      result_.handling_result = IFormula::HandlingResult::ReferencesRenamedOnly;
  }
}
void ShiftedFormulaListener::ExitCellColDeletion(
    std::string text, Position pos) {
  if (pos.col >= first_idx_ && pos.col < first_idx_ + count_) {
    data_.emplace(kInvalidPosStr);
    result_.handling_result = IFormula::HandlingResult::ReferencesChanged;
  } else if (text == kInvalidPosStr) {
    data_.emplace(kInvalidPosStr);
  } else if (pos.col < first_idx_) {
    data_.push(std::move(text));
    result_.info.referenced_cells.push_back(pos);
  } else {
    pos.col -= count_;
    data_.push(pos.ToString());
    result_.info.referenced_cells.push_back(pos);
    if (result_.handling_result == IFormula::HandlingResult::NothingChanged)
      result_.handling_result = IFormula::HandlingResult::ReferencesRenamedOnly;
  }
}

ShiftedFormulaListener CreateShiftedListener(
    const std::string &expr, OpType op_type, ShiftType shift_type,
    int first_idx, int count) {
  first_idx = std::min(16384, first_idx);
  count = std::min(16384, count);
  ShiftedFormulaListener listener(expr, op_type, shift_type,
                                  first_idx, count);
  listener_utils::Run(expr, &listener);
  return listener;
}
