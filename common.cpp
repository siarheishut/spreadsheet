#include "common.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <tuple>

#include "utils.h"

const int kLetterCount = 26;

// -----Position----------------------------------------------------------------

bool Position::operator==(const Position &rhs) const {
  return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position &rhs) const {
  return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
  return (row >= 0 && col >= 0 && row < kMaxRows && col < kMaxCols);
}

// O(N), N – pos_str.size
std::string Position::ToString() const {
  if (!IsValid()) return "";
  std::string str_row, str_col;
  str_row = std::to_string(row + 1);

  int cols = col;
  while (cols >= 0) {
    str_col.push_back('A' + cols % kLetterCount);
    cols = cols / kLetterCount - 1;
  }
  std::reverse(begin(str_col), end(str_col));
  return {std::move(str_col) + std::move(str_row)};
}

// O(N), N – str.size
Position Position::FromString(std::string_view str) {
  if (str.size() > 8) {
    return Position{-1, -1};
  }
  int idx = 0;
  while (idx < str.size() && std::isupper(str[idx])) {
    ++idx;
  }
  auto col = str.substr(0, idx);
  auto row = str.substr(idx);

  if (row.empty() || col.empty()) {
    return Position{-1, -1};
  }
  for (auto c : row) {
    if (!std::isdigit(c)) {
      return Position{-1, -1};
    }
  }

  int rows = 0;
  int row_size = row.size();
  std::string row_str(row);
  if (std::istringstream row_in(row_str); !(row_in >> rows) || !row_in.eof() ||
      row_size != std::to_string(rows).size()) {
    return Position{-1, -1};
  }

  int cols = 0;
  for (char ch : col) {
    cols *= kLetterCount;
    cols += ch - 'A' + 1;
  }

  --rows;
  --cols;
  Position result{rows, cols};

  if (!result.IsValid()) return Position{-1, -1};

  return result;
}

// -----Size--------------------------------------------------------------------

bool Size::operator==(const Size &rhs) const {
  return std::tie(rows, cols) == std::tie(rhs.rows, rhs.cols);
}

// -----FormulaError------------------------------------------------------------

FormulaError::FormulaError(FormulaError::Category category)
    : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
  return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
  return category_ == rhs.GetCategory();
}

std::string_view FormulaError::ToString() const {
  switch (category_) {
    case Category::Ref:
      return "#REF!";
    case Category::Value:
      return "#VALUE!";
    case Category::Div0:
      return "#DIV/0!";
  }
  return "";
}

std::ostream &operator<<(std::ostream &output, FormulaError fe) {
  return output << fe.ToString();
}
