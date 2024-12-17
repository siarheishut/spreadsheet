#ifndef SPREADSHEET_REFERENCED_CELLS_LISTENER_H_
#define SPREADSHEET_REFERENCED_CELLS_LISTENER_H_

#include <string>
#include <vector>

#include "common.h"
#include "FormulaParser.h"
#include "FormulaBaseListener.h"

class ReferencedCellsListener : public FormulaBaseListener {
 public:
  void exitCell(FormulaParser::CellContext *ctx) override;
  std::vector<Position> ReleaseRefs();
 private:
  std::vector<Position> referenced_cells_;
};

std::vector<Position> ReadRefs(const std::string &expr); // O(N), N - expr.size

#endif // SPREADSHEET_REFERENCED_CELLS_LISTENER_H_
