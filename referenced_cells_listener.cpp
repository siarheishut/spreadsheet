#include "referenced_cells_listener.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include "common.h"
#include "utils.h"
#include "FormulaParser.h"

void ReferencedCellsListener::exitCell(FormulaParser::CellContext *ctx) {
  auto pos = Position::FromString(ctx->CELL()->getText());
  if (pos.IsValid()) {
    referenced_cells_.push_back(pos);
  } else {
    throw FormulaException("Wrong formula format");
  }
}

std::vector<Position> ReferencedCellsListener::ReleaseRefs() {
  return std::move(referenced_cells_);
}

std::vector<Position> ReadRefs(const std::string &expr) {
  ReferencedCellsListener listener;
  listener_utils::Run(expr, &listener);
  auto refs = listener.ReleaseRefs();
  std::sort(begin(refs), end(refs));
  refs.erase(std::unique(begin(refs), end(refs)), end(refs));
  return refs;
}
