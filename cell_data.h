#pragma once

#ifndef SPREADSHEET__CELL_DATA_H_
#define SPREADSHEET__CELL_DATA_H_

#include <string>
#include <variant>
#include "sheet.h"
#include "formula.h"

class ICellData {
 public:
  virtual ~ICellData() = default;
  virtual std::string GetText() const = 0;
  virtual ICell::Value GetValue() const = 0;
  virtual bool IsCached() const = 0;
  virtual void ResetCache() const = 0;
  virtual std::vector<Position> GetReferencedCells() const = 0;
  virtual IFormula::HandlingResult HandleInsertedRows(int before,
                                                      int count) = 0;
  virtual IFormula::HandlingResult HandleInsertedCols(int before,
                                                      int count) = 0;
  virtual IFormula::HandlingResult HandleDeletedRows(int first, int count) = 0;
  virtual IFormula::HandlingResult HandleDeletedCols(int first, int count) = 0;
};

namespace cell_data {
class Text final : public ICellData {
 public:
  explicit Text(std::string text); // O(N); N – text.size

  std::string GetText() const override; // O(N), N - text.size
  ICell::Value GetValue() const override; // Worst case: O(N), N - str.size
  bool IsCached() const override; // O(1)
  void ResetCache() const override; // O(1)
  std::vector<Position> GetReferencedCells() const override; // O(1)
  // O(1)
  IFormula::HandlingResult HandleInsertedRows(int before, int count) override;
  // O(1)
  IFormula::HandlingResult HandleInsertedCols(int before, int count) override;
  // O(1)
  IFormula::HandlingResult HandleDeletedRows(int first, int count) override;
  // O(1)
  IFormula::HandlingResult HandleDeletedCols(int first, int count) override;

 private:
  std::string text_;
  ICell::Value value_;
};

class Formula final : public ICellData {
 public:
  Formula(std::string expr, const ISheet &sheet); // O(N), N – expr.size

  std::string GetText() const override; // O(N); N – text.size
  ICell::Value GetValue() const override; // Worst case: O(N); N – str.size
  bool IsCached() const override; // O(1)
  void ResetCache() const override; // O(1)
  std::vector<Position> GetReferencedCells() const override; // O(1)
  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleInsertedRows(int before, int count) override;
  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleInsertedCols(int before, int count) override;
  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleDeletedRows(int first, int count) override;
  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleDeletedCols(int first, int count) override;

 private:
  const ISheet &sheet_;
  std::unique_ptr<IFormula> formula_;
  mutable std::variant<std::monostate, double, FormulaError> value_;
};
}

#endif //SPREADSHEET__CELL_DATA_H_
