#include "expr_shrink_listener.h"

#include <set>
#include <string>

#include "common.h"
#include "utils.h"
#include "antlr4-runtime.h"
#include "FormulaParser.h"

namespace {
bool NeedParenthesis(FormulaParser::ParensContext *ctx,
                     FormulaParser::BinaryOpContext *parent,
                     FormulaParser::BinaryOpContext *child) {
  return (parent->SUB() && ctx == parent->expr(1) &&
      (child->ADD() || child->SUB())) ||
      (parent->MUL() && (child->ADD() || child->SUB())) ||
      (parent->DIV() && (child->ADD() || child->SUB())) ||
      (parent->DIV() && ctx == parent->expr(1) &&
          (child->MUL() || child->DIV()));
}
}

ExprShrinkListener::ExprShrinkListener(ShrinkMode mode) : mode_(mode) {}

void ExprShrinkListener::enterMain(FormulaParser::MainContext *ctx) {
  if (!ctx->expr()) {
    throw std::runtime_error("ExprShrinkListener::enterMain : !ctx->expr()");
  }
  context_info[ctx->expr()].parent_type = ContextType::kMain;
  context_info[ctx->expr()].parent_ptr = ctx;
}

void ExprShrinkListener::enterUnaryOp(FormulaParser::UnaryOpContext *ctx) {
  if (!ctx->expr()) {
    throw std::runtime_error("ExprShrinkListener::enterUnaryOp : !ctx->expr()");
  }
  context_info[ctx->expr()].parent_type = ContextType::kUnaryOp;
  context_info[ctx->expr()].parent_ptr = ctx;
  AddChild(ctx, ContextType::kUnaryOp);
}

void ExprShrinkListener::exitUnaryOp(FormulaParser::UnaryOpContext *ctx) {
  if (data_.empty()) {
    throw std::runtime_error("ExprShrinkListener::exitUnaryOp failed");
  }

  auto &node = data_.top();
  if (ctx->ADD()) {
    node = '+' + node;
  } else {
    node = '-' + node;
  }
}

void ExprShrinkListener::enterParens(FormulaParser::ParensContext *ctx) {
  if (!ctx->expr()) {
    throw std::runtime_error("ExprShrinkListener::enterParens : !ctx->expr()");
  }
  context_info[ctx->expr()].parent_type = ContextType::kParens;
  context_info[ctx->expr()].parent_ptr = ctx;
  AddChild(ctx, ContextType::kParens);
}

void ExprShrinkListener::exitParens(FormulaParser::ParensContext *ctx) {
  if (context_info[ctx].parent_type == ContextType::kNone)
    return;

  auto parent_type = context_info[ctx].parent_type;
  if (parent_type == ContextType::kMain ||
      parent_type == ContextType::kParens) {
    return;
  }

  if ((parent_type == ContextType::kBinaryOp ||
      parent_type == ContextType::kUnaryOp) &&
      context_info[ctx].children_types.empty()) {
    throw std::runtime_error("ExprShrinkListener::exitParens: empty child types");
  }

  if (parent_type == ContextType::kUnaryOp &&
      context_info[ctx].children_types[0] == ContextType::kBinaryOp) {
    auto child = dynamic_cast<FormulaParser::BinaryOpContext *>(ctx->expr());

    if (!child) {
      throw std::runtime_error("ExprShrinkListener::exitParens: empty child");
    }

    if (child->ADD() || child->SUB()) {
      if (data_.empty()) {
        throw FormulaException("Invalid formula");
      }

      auto &node = data_.top();
      node = '(' + node + ')';
    }
  } else if (parent_type == ContextType::kBinaryOp &&
      context_info[ctx].children_types[0] == ContextType::kBinaryOp) {
    auto parent = dynamic_cast<FormulaParser::BinaryOpContext *>(
        context_info[ctx].parent_ptr);
    auto child = dynamic_cast<FormulaParser::BinaryOpContext *>(ctx->expr());

    if (!parent) {
      throw std::runtime_error("ExprShrinkListener::exitParens: empty parent");
    }
    if (!child) {
      throw std::runtime_error("ExprShrinkListener::exitParens: empty child");
    }

    if (NeedParenthesis(ctx, parent, child)) {
      if (data_.empty()) {
        throw FormulaException("Invalid formula");
      }
      auto &node = data_.top();
      node = '(' + node + ')';
    }
  }
}

void ExprShrinkListener::enterLiteral(FormulaParser::LiteralContext *ctx) {
  AddChild(ctx, ContextType::kLiteral);
}

void ExprShrinkListener::exitLiteral(FormulaParser::LiteralContext *ctx) {
  data_.push(ctx->getText());
}

void ExprShrinkListener::enterCell(FormulaParser::CellContext *ctx) {
  AddChild(ctx, ContextType::kCell);
}

void ExprShrinkListener::exitCell(FormulaParser::CellContext *ctx) {
  auto text = ctx->getText();
  if (text == kInvalidPosStr) {
    switch (mode_) {
      case ShrinkMode::kSimple:
        break;
      case ShrinkMode::kPrintErrors:
        text = std::string(FormulaError{FormulaError::Category::Ref}.
            ToString());
        break;
      default:
        throw std::runtime_error("ExprShrinkListener::exitCell : "
                                 "unexpected ShrinkMode type");
    }
    data_.push(std::move(text));
  } else {
    data_.push(std::move(text));
  }
}

void ExprShrinkListener::enterBinaryOp(FormulaParser::BinaryOpContext *ctx) {
  if (!ctx->expr(0) || !ctx->expr(1)) {
    throw std::runtime_error("ExprShrinkListener::enterBinaryOp : invalid ctx");
  }
  context_info[ctx->expr(0)].parent_type = ContextType::kBinaryOp;
  context_info[ctx->expr(1)].parent_type = ContextType::kBinaryOp;
  context_info[ctx->expr(0)].parent_ptr = ctx;
  context_info[ctx->expr(1)].parent_ptr = ctx;
  AddChild(ctx, ContextType::kBinaryOp);
}

void ExprShrinkListener::exitBinaryOp(FormulaParser::BinaryOpContext *ctx) {
  if (data_.empty()) {
    throw std::runtime_error("ExprShrinkListener::exitBinaryOp rhs failed");
  }
  auto rhs = std::move(data_.top());
  data_.pop();

  if (data_.empty()) {
    throw std::runtime_error("ExprShrinkListener::exitBinaryOp lhs failed");
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

std::string ExprShrinkListener::ReleaseResult() {
  if (data_.empty()) {
    throw std::runtime_error("ExprShrinkListener::ReleaseResult failed");
  }

  auto node = std::move(data_.top());
  data_.pop();
  return node;
}

void ExprShrinkListener::AddChild(antlr4::ParserRuleContext *ctx,
                                  ContextType type) {
  if (!ctx || !context_info[ctx].parent_ptr) {
    throw std::runtime_error("ExprShrinkListener::AddChild : nullptr aren't"
                             "expected");
  }
  context_info[context_info[ctx].parent_ptr].children_types.push_back(type);
}

std::string ShrinkExpr(const std::string &expression, ShrinkMode mode) {
  ExprShrinkListener listener(mode);
  listener_utils::Run(expression, &listener);
  return listener.ReleaseResult();
}
