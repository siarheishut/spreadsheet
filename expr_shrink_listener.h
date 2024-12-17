#ifndef SPREADSHEET_EXPR_SHRINK_LISTENER_H_
#define SPREADSHEET_EXPR_SHRINK_LISTENER_H_

#include <exception>
#include <memory>
#include <set>
#include <stack>
#include <string>

#include "FormulaBaseListener.h"

#include "sheet.h"

enum class ShrinkMode {
  kNone = 0,
  kSimple,
  kPrintErrors
};

class ExprShrinkListener : public FormulaBaseListener {
 public:
  enum class ContextType {
    kNone = 0,
    kMain,
    kUnaryOp,
    kParens,
    kLiteral,
    kCell,
    kBinaryOp,
  };

  explicit ExprShrinkListener(ShrinkMode mode);

  void enterMain(FormulaParser::MainContext *ctx) override;

  void enterUnaryOp(FormulaParser::UnaryOpContext *ctx) override;
  void exitUnaryOp(FormulaParser::UnaryOpContext *ctx) override;

  void enterParens(FormulaParser::ParensContext *ctx) override;
  void exitParens(FormulaParser::ParensContext *ctx) override;

  void enterLiteral(FormulaParser::LiteralContext *ctx) override;
  void exitLiteral(FormulaParser::LiteralContext *ctx) override;

  void enterCell(FormulaParser::CellContext *ctx) override;
  void exitCell(FormulaParser::CellContext *ctx) override;

  void enterBinaryOp(FormulaParser::BinaryOpContext *ctx) override;
  void exitBinaryOp(FormulaParser::BinaryOpContext *ctx) override;

  std::string ReleaseResult();

 private:
  void AddChild(antlr4::ParserRuleContext *ctx, ContextType type);

  struct ContextInfo {
    ContextType parent_type;
    std::vector<ContextType> children_types;
    antlr4::ParserRuleContext *parent_ptr;
  };

  ShrinkMode mode_;

  std::unordered_map<antlr4::ParserRuleContext *, ContextInfo> context_info;
  std::stack<std::string> data_;
};

// O(N); N – expression.size
std::string ShrinkExpr(const std::string &expression, ShrinkMode mode);

#endif // SPREADSHEET_EXPR_SHRINK_LISTENER_H_
