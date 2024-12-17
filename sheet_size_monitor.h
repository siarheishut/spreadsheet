#ifndef SPREADSHEET__SHEET_SIZE_MONITOR_H_
#define SPREADSHEET__SHEET_SIZE_MONITOR_H_

#include <set>

#include "common.h"

class SheetSizeMonitor {
 public:
  void Add(Position pos); // O(logN); N – non_empty_cells.size
  void Remove(Position pos); // O(logN); N – non_empty_cells.size
  Size GetSize() const; // O(N); N – non_empty_cells.size

  void UpdateAfterRowAddition(int first_idx, int count); // O(N)
  void UpdateAfterColAddition(int first_idx, int count); // O(N)
  void UpdateAfterRowDeletion(int first_idx, int count); // O(N)
  void UpdateAfterColDeletion(int first_idx, int count); // O(N)

 private:
  std::set<Position> non_empty_cells_;
  mutable std::optional<int> max_col_;
};

#endif //SPREADSHEET__SHEET_SIZE_MONITOR_H_
