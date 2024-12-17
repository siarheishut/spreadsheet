
// Generated from /Users/sierzh/CLionProjects/spreadsheet/Formula.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "FormulaVisitor.h"


/**
 * This class provides an empty implementation of FormulaVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  FormulaBaseVisitor : public FormulaVisitor {
public:

  virtual std::any visitMain(FormulaParser::MainContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryOp(FormulaParser::UnaryOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParens(FormulaParser::ParensContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(FormulaParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCell(FormulaParser::CellContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBinaryOp(FormulaParser::BinaryOpContext *ctx) override {
    return visitChildren(ctx);
  }


};

