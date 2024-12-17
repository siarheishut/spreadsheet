#include "cell_data.h"

#include <memory>
#include <string>
#include <vector>
#include <variant>

#include "cell.h"
#include "error_handle_listener.h"
#include "formula.h"
#include "my_formula.h"
#include "utils.h"

namespace cell_data {
Text::Text(std::string text) : text_(std::move(text)) {
  if (text_.empty()) {
    value_ = 0.0;
  } else if (auto num = ::ToDouble(text_)) {
    value_ = num.value();
  } else if (text_[0] == kEscapeSign) {
    value_ = text_.substr(1);
  } else {
    value_ = text_;
  }
}
std::string Text::GetText() const {
  return text_;
}
ICell::Value Text::GetValue() const {
  return value_;
}
bool Text::IsCached() const {
  return true;
}
void Text::ResetCache() const {
  return;
}
std::vector<Position> Text::GetReferencedCells() const {
  return {};
}
IFormula::HandlingResult Text::HandleInsertedRows(int before, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult Text::HandleInsertedCols(int before, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult Text::HandleDeletedRows(int first, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
IFormula::HandlingResult Text::HandleDeletedCols(int first, int count) {
  return IFormula::HandlingResult::NothingChanged;
}
}

namespace cell_data {
Formula::Formula(std::string expr, const ISheet &sheet)
    : sheet_(sheet), formula_(ParseFormula(std::move(expr))) {
}
std::string Formula::GetText() const {
  auto shrank_expr_ = dynamic_cast<::Formula *>(formula_.get())->
      GetShrankExpr();
  return '=' + HandleExprErrors(shrank_expr_, sheet_);
}
ICell::Value Formula::GetValue() const {
  if (std::holds_alternative<std::monostate>(value_)) {
    IFormula::Value res = formula_->Evaluate(sheet_);
    if (std::holds_alternative<double>(res)) {
      value_ = std::get<double>(res);
    } else if (std::holds_alternative<FormulaError>(res)) {
      value_ = std::get<FormulaError>(res);
    }
  }

  if (std::holds_alternative<double>(value_)) {
    return std::get<double>(value_);
  }
  return std::get<FormulaError>(value_);
}
std::vector<Position> Formula::GetReferencedCells() const {
  return formula_->GetReferencedCells();
}
IFormula::HandlingResult Formula::HandleInsertedRows(int before, int count) {
  return formula_->HandleInsertedRows(before, count);
}
IFormula::HandlingResult Formula::HandleInsertedCols(int before, int count) {
  return formula_->HandleInsertedCols(before, count);
}
IFormula::HandlingResult Formula::HandleDeletedRows(int first, int count) {
  return formula_->HandleDeletedRows(first, count);
}
IFormula::HandlingResult Formula::HandleDeletedCols(int first, int count) {
  return formula_->HandleDeletedCols(first, count);
}
bool Formula::IsCached() const {
  return !std::holds_alternative<std::monostate>(value_);
}
void Formula::ResetCache() const {
  value_.emplace<std::monostate>();
}
}
