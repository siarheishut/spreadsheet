#include "utils.h"

#include <string>

#include "antlr4-runtime.h"
#include "FormulaLexer.h"
#include "FormulaParser.h"
#include "bail_error_listener.h"
#include "common.h"

size_t PositionHash::operator()(Position pos) const {
  return pos.row + pos.col * 10007;
}

std::optional<double> ToDouble(const std::string str) {
  std::optional<double> res;
  std::istringstream in(str);
  double num = 0.0;
  char ch;
  if ((in >> num) &&
      (!(in >> ch) || ch == std::char_traits<char>::eof() || in.eof())) {
    res.emplace(num);
  }
  return res;
}

namespace listener_utils {
void Run(const std::string &expr,
         antlr4::tree::ParseTreeListener *listener) {
  antlr4::ANTLRInputStream input(expr);

  FormulaLexer lexer(&input);
  BailErrorListener error_listener;
  lexer.removeErrorListeners();
  lexer.addErrorListener(&error_listener);

  antlr4::CommonTokenStream tokens(&lexer);

  FormulaParser parser(&tokens);
  auto error_handler = std::make_shared<antlr4::BailErrorStrategy>();
  parser.setErrorHandler(error_handler);
  parser.removeErrorListeners();

  antlr4::tree::ParseTree *tree;
  try {
    tree = parser.main();
  } catch (...) {
    throw FormulaException("Wrong formula format");
  }
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(listener, tree);
}
}