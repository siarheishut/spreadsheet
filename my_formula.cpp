#include "my_formula.h"

#include <cmath>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

#include "common.h"
#include "expr_shrink_listener.h"
#include "referenced_cells_listener.h"
#include "tree_shape_listener.h"
#include "shifted_formula_listener.h"
#include "error_handle_listener.h"
#include "utils.h"

// -----Formula-----------------------------------------------------------------

Formula::Formula(std::string expr) {
  auto refs = ReadRefs(expr);
  info_ = {
      .expr = std::move(expr),
      .referenced_cells = std::move(refs)};
}

Formula::Value Formula::Evaluate(const ISheet &sheet) const {
  FormulaEvaluatorListener l(sheet);
  listener_utils::Run(info_.expr, &l); // O(N), N - expr.size
  auto result = l.GetResult();
  if (std::holds_alternative<double>(result)) {
    auto value = std::get<double>(result);
    if (!std::isfinite(value)) {
      return FormulaError{FormulaError::Category::Div0};
    }
    return value;
  } else if (std::holds_alternative<FormulaError>(result)) {
    return std::get<FormulaError>(result);
  }
  throw std::logic_error("Unexpected result type");
}

std::string Formula::GetExpression() const {
  if (!shrank_expr_) {
    shrank_expr_ = ShrinkExpr(info_.expr, ShrinkMode::kPrintErrors);
  }
  return *shrank_expr_;
}

std::string Formula::GetShrankExpr() const {
  if (!shrank_expr_) {
    shrank_expr_ = ShrinkExpr(info_.expr, ShrinkMode::kSimple);
  }
  return *shrank_expr_;
}

std::vector<Position> Formula::GetReferencedCells() const {
  return info_.referenced_cells;
}

IFormula::HandlingResult Formula::HandleInsertedRows(int before, int count) {
  auto listener = CreateShiftedListener(info_.expr,
                                        OpType::kAddition,
                                        ShiftType::kRows,
                                        before,
                                        count);
  auto [res, info] = listener.ReleaseShiftResult();
  info_ = std::move(info);
  ResetShrankExpr(res);
  return res;
}

IFormula::HandlingResult Formula::HandleInsertedCols(int before, int count) {
  auto listener = CreateShiftedListener(info_.expr,
                                        OpType::kAddition,
                                        ShiftType::kCols,
                                        before,
                                        count);
  auto [res, info] = listener.ReleaseShiftResult();
  info_ = std::move(info);
  ResetShrankExpr(res);
  return res;
}

IFormula::HandlingResult Formula::HandleDeletedRows(int first, int count) {
  auto listener = CreateShiftedListener(info_.expr,
                                        OpType::kDeletion,
                                        ShiftType::kRows,
                                        first,
                                        count);
  auto [res, info] = listener.ReleaseShiftResult();
  info_ = std::move(info);
  ResetShrankExpr(res);
  return res;
}

IFormula::HandlingResult Formula::HandleDeletedCols(int first, int count) {
  auto listener = CreateShiftedListener(info_.expr,
                                        OpType::kDeletion,
                                        ShiftType::kCols,
                                        first,
                                        count);
  auto [res, info] = listener.ReleaseShiftResult();
  info_ = std::move(info);
  ResetShrankExpr(res);
  return res;
}

void Formula::ResetShrankExpr(IFormula::HandlingResult res) {
  if (res != IFormula::HandlingResult::NothingChanged) {
    shrank_expr_.reset();
  }
}

std::unique_ptr<IFormula> ParseFormula(std::string expression) {
  return std::make_unique<Formula>(std::move(expression));
}
