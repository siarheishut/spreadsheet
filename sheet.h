#ifndef SPREADSHEET_SRC_SHEET_H_
#define SPREADSHEET_SRC_SHEET_H_

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <unordered_set>

#include "common.h"
#include "sheet_size_monitor.h"
#include "utils.h"

class Cell;

using Cells = std::vector<std::vector<std::unique_ptr<Cell>>>;

enum class TableItem {
  kRows,
  kCols,
};

enum class PrintSettings {
  kValues,
  kTexts,
};

class Sheet final : public ISheet {
 public:
  ~Sheet() = default;

  // O(max(N, M, logK); N – pos.row, M – pos.col, K – non_empty_cells.size
  void ForceInitializeCell(Position pos);
  void SetCell(Position pos, std::string text) override;

  const ICell *GetCell(Position pos) const override;
  ICell *GetCell(Position pos) override;

  void ClearCell(Position pos) override;

  void InsertRows(int before, int count) override;
  void InsertCols(int before, int count) override;

  void DeleteRows(int first, int count) override;
  void DeleteCols(int first, int count) override;

  Size GetPrintableSize() const override;

  void PrintValues(std::ostream &out) const override;
  void PrintTexts(std::ostream &out) const override;

 private:
  bool IsValid(Position pos) const; // O(1)

  void UpdateEmptyCells(); // O(N), N – empty_cells_count

  void ExpandToFit(Position pos); // O(max(N, M); N – pos.row, M – pos.col

  void ValidateExpand(int before, int count, TableItem item); // O(1)

  // O(empty_cells.size)????
  void ExpandTable(int before, int count, TableItem item);

  // After rows/cols deletion invalidates cells, which refer to
  // deleted/invalidated cells.
  void InvalidateCells(ShiftType type, int first, int second);

  // O(N), N - formula_expr.size
  void UpdateCellsAfterRowAddition(int first_idx, int count);
  // O(N), N - formula_expr.size
  void UpdateCellsAfterColAddition(int first_idx, int count);
  // O(N), N - formula_expr.size
  void UpdateCellsAfterRowDeletion(int first_idx, int count);
  // O(N), N - formula_expr.size
  void UpdateCellsAfterColDeletion(int first_idx, int count);

  void PrintCells(std::ostream &out, PrintSettings print_settings) const;

  SheetSizeMonitor printable_size_monitor_;
  SheetSizeMonitor size_monitor_;
  std::unordered_set<Cell *> empty_cells_;
  Cells cells_;
};

#endif // SPREADSHEET_SRC_SHEET_H_
