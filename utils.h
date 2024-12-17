#ifndef SPREADSHEET__UTILS_H_
#define SPREADSHEET__UTILS_H_

#include <ostream>
#include <string>
#include <vector>
#include <variant>

#include "antlr4-runtime.h"

#include "common.h"

enum class ShiftType {
  kNone = 0,
  kRows,
  kCols,
};

enum class OpType {
  kNone = 0,
  kDeletion,
  kAddition,
};

static std::string kInvalidPosStr = "A16385";

std::ostream &operator<<(std::ostream &out, const ICell::Value &val);

struct FormulaInfo {
  std::string expr;
  std::vector<Position> referenced_cells;
};

std::optional<double> ToDouble(const std::string str);

struct PositionHash {
  size_t operator()(Position pos) const;
};

namespace listener_utils {
void Run(const std::string &expr, antlr4::tree::ParseTreeListener *listener);
}

#endif //SPREADSHEET__UTILS_H_
