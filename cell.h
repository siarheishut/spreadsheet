#ifndef SPREADSHEET_SRC_CELL_H_
#define SPREADSHEET_SRC_CELL_H_

#include <memory>
#include <ostream>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>

#include "common.h"
#include "formula.h"
#include "shifted_formula_listener.h"
#include "utils.h"
#include "cell_data.h"

enum class CellState {
  kEmpty = 0,
  kText,
  kFormula,
  kRefError,
  kValueError,
  kDiv0Error,
};

class Sheet;

class Cell final : public ICell {
 public:
  Cell(Sheet &sheet, Position position); // O(1)

  ~Cell() = default;

  // O(max(N, M); N – non-empty cells count; M – text.size
  void Set(std::string text);

  std::vector<Position> GetReferencedCells() const override; // O(1)

  // text.size()
  std::string GetText() const override;
  // O(N); N – text.size
  Value GetValue() const override;

  // O(1) ???
  std::unordered_set<Position, PositionHash> GetReferencingCells() const;

  CellState State() const; // O(1)
  void SetState(CellState cat); // O(1)

  void ClearRefs(); // O(1)
  void SetRefs(); // O(max(N, M); N – non-empty cells count; M – text.size
  void ResetCache(bool force); // O(N); N – non-empty cells count

  inline Position GetPosition() const { return pos_in_sheet_; } // O(1)

  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleInsertedRows(int before, int count);
  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleInsertedCols(int before, int count);
  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleDeletedRows(int first, int count);
  // O(N), N - formula_expr.size
  IFormula::HandlingResult HandleDeletedCols(int first, int count);

 private:
  struct InternalData {
    std::unique_ptr<ICellData> data;
    std::vector<Position> referenced_cells;
    std::unordered_set<Position, PositionHash> referencing_cells;
    CellState state;
  };

  // O(N); N – non-empty cells count
  bool IsAddingCircularDependency(const ICellData &new_data) const;

  Sheet &sheet_;
  Position pos_in_sheet_;
  InternalData internal_data_;
  // Set bool denotes circular dependency appeared in prev ;
  std::optional<std::pair<std::string, bool>> last_set_args_;
};

#endif // SPREADSHEET_SRC_CELL_H_
