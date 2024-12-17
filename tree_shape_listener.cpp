#include "tree_shape_listener.h"

#include <charconv>
#include <stdexcept>
#include <sstream>
#include <string>
#include <system_error>
#include <variant>

#include "common.h"
#include "FormulaParser.h"

FormulaEvaluatorListener::FormulaEvaluatorListener(const ISheet &sheet)
    : sheet_(sheet) {}

void FormulaEvaluatorListener::exitUnaryOp(FormulaParser::UnaryOpContext *ctx) {
  if (error_) {
    return;
  }

  if (data_.empty()) {
    throw std::runtime_error(
        "FormulaEvaluatorListener::exitUnaryOp: no data: failed precondition");
  }

  auto &node = data_.top();
  if (ctx->SUB()) {
    node = -node;
  }
}

void FormulaEvaluatorListener::exitLiteral(FormulaParser::LiteralContext *ctx) {
  if (error_)
    return;
  auto text = ctx->NUMBER()->getSymbol()->getText();
  std::istringstream in(text);
  double num = 0.0;
  char ch;
  if ((in >> num)
      && (!(in >> ch) || (ch == std::char_traits<char>::eof()) || in.eof())) {
    data_.push(num);
  } else {
    error_ = FormulaError::Category::Value;
  }
}

void FormulaEvaluatorListener::exitCell(FormulaParser::CellContext *ctx) {
  if (error_)
    return;

  auto text = ctx->CELL()->getText();
  if (text == kInvalidPosStr) {
    error_ = FormulaError::Category::Ref;
    return;
  }
  Position pos = Position::FromString(text);
  if (auto cell = sheet_.GetCell(pos); !cell) {
    data_.push(0);
  } else if (auto value = cell->GetValue();
      std::holds_alternative<double>(value)) {
    data_.push(std::get<double>(value));
  } else if (std::holds_alternative<std::string>(value)) {
    std::istringstream in(std::get<std::string>(value));
    if (in.str().empty()) {
      data_.push(0);
      return;
    }

    int num = 0;
    if (!(in >> num) || !in.eof()) {
      error_ = FormulaError::Category::Value;
    } else {
      data_.push(num);
    }
  } else if (std::holds_alternative<FormulaError>(value)) {
    error_ = std::get<FormulaError>(value);
  }
}

void FormulaEvaluatorListener::exitBinaryOp(FormulaParser::BinaryOpContext *ctx) {
  if (error_)
    return;

  if (data_.empty()) {
    throw std::runtime_error(
        "FormulaEvaluatorListener::exitBinaryOp: no data: failed precondition");
  }

  auto rhs = data_.top();
  data_.pop();

  if (data_.empty()) {
    throw std::runtime_error(
        "FormulaEvaluatorListener::exitBinaryOp: no data: failed precondition");
  }

  auto lhs = data_.top();
  data_.pop();
  if (ctx->ADD()) {
    data_.push(lhs + rhs);
  } else if (ctx->SUB()) {
    data_.push(lhs - rhs);
  } else if (ctx->MUL()) {
    data_.push(lhs * rhs);
  } else if (ctx->DIV()) {
    data_.push(lhs / rhs);
  }
}

std::variant<double, FormulaError> FormulaEvaluatorListener::GetResult() const {
  if (error_)
    return *error_;

  if (data_.empty()) {
    throw std::runtime_error(
        "FormulaEvaluatorListener::GetResult: no data: failed precondition");
  }
  return data_.top();
}
