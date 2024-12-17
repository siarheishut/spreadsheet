#include "error_handle_listener.h"

#include <stdexcept>
#include <string>

#include "cell.h"
#include "utils.h"

HandleErrorsListener::HandleErrorsListener(const ISheet &sheet)
    : sheet_(sheet) {}

void HandleErrorsListener::exitUnaryOp(FormulaParser::UnaryOpContext *ctx) {
  if (data_.empty()) {
    throw std::runtime_error("HandleErrorsListener::exitUnaryOp failed");
  }

  auto &node = data_.top();
  if (ctx->ADD()) {
    node = '+' + node;
  } else {
    node = '-' + node;
  }
}

void HandleErrorsListener::exitParens(FormulaParser::ParensContext *ctx) {
  if (data_.empty()) {
    throw FormulaException("Invalid formula");
  }
  auto &node = data_.top();
  node = '(' + node + ')';
}

void HandleErrorsListener::exitLiteral(FormulaParser::LiteralContext *ctx) {
  data_.push(ctx->getText());
}

void HandleErrorsListener::exitBinaryOp(FormulaParser::BinaryOpContext *ctx) {
  if (data_.empty()) {
    throw std::runtime_error("HandleErrorsListener::exitBinaryOp rhs failed");
  }
  auto rhs = std::move(data_.top());
  data_.pop();

  if (data_.empty()) {
    throw std::runtime_error("HandleErrorsListener::exitBinaryOp lhs failed");
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

void HandleErrorsListener::exitCell(FormulaParser::CellContext *ctx) {
  auto text = ctx->getText();
  auto pos = Position::FromString(ctx->getText());

  if (text == kInvalidPosStr) {
    text = std::string(FormulaError{FormulaError::Category::Ref}.ToString());
  } else if (auto cell = dynamic_cast<const Cell *>(sheet_.GetCell(pos));
      cell && cell->State() == CellState::kValueError) {
    text = std::string(FormulaError{FormulaError::Category::Value}.ToString());
  } else if (cell && cell->State() == CellState::kDiv0Error) {
    text = std::string(FormulaError{FormulaError::Category::Div0}.ToString());
  } else {
    text = Position::FromString(std::move(text)).ToString();
  }
  data_.push(std::move(text));
}

std::string HandleErrorsListener::ReleaseResult() {
  if (data_.empty()) {
    throw std::runtime_error("HandleErrorsListener::ReleaseResult");
  }
  auto result = std::move(data_.top());
  data_.pop();
  return result;
}

std::string HandleExprErrors(const std::string &expression,
                             const ISheet &sheet) {
  HandleErrorsListener listener(sheet);
  listener_utils::Run(expression, &listener);
  return listener.ReleaseResult();
}
