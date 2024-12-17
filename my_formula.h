#ifndef SPREADSHEET__MY_FORMULA_H_
#define SPREADSHEET__MY_FORMULA_H_

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "formula.h"
#include "tree_shape_listener.h"
#include "utils.h"

#include "FormulaBaseListener.h"

class Formula : public IFormula {
 public:
  explicit Formula(std::string expr); // O(N), N - expr.size
  Value Evaluate(const ISheet &sheet) const override; // O(N), N - expr.size
  std::string GetExpression() const override; // O(N), N - expr.size
  std::string GetShrankExpr() const; // O(N), N - expr.size
  // O(1)
  std::vector<Position> GetReferencedCells() const override;
  // O(N), N - expr.size
  HandlingResult HandleInsertedRows(int before, int count) override;
  // O(N), N - expr.size
  HandlingResult HandleInsertedCols(int before, int count) override;
  // O(N), N - expr.size
  HandlingResult HandleDeletedRows(int first, int count) override;
  // O(N), N - expr.size
  HandlingResult HandleDeletedCols(int first, int count) override;

 private:
  void ResetShrankExpr(IFormula::HandlingResult res); // O(1)

  FormulaInfo info_;
  mutable std::optional<std::string> shrank_expr_;
};

#endif // SPREADSHEET__MY_FORMULA_H_
