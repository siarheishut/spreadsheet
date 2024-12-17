
// Generated from /Users/sierzh/CLionProjects/spreadsheet/Formula.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "FormulaParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by FormulaParser.
 */
class  FormulaVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by FormulaParser.
   */
    virtual std::any visitMain(FormulaParser::MainContext *context) = 0;

    virtual std::any visitUnaryOp(FormulaParser::UnaryOpContext *context) = 0;

    virtual std::any visitParens(FormulaParser::ParensContext *context) = 0;

    virtual std::any visitLiteral(FormulaParser::LiteralContext *context) = 0;

    virtual std::any visitCell(FormulaParser::CellContext *context) = 0;

    virtual std::any visitBinaryOp(FormulaParser::BinaryOpContext *context) = 0;


};

