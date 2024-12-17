#include "sheet.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "cell.h"
#include "common.h"
#include "utils.h"

void Sheet::ForceInitializeCell(Position pos) {
  if (!pos.IsValid())
    throw InvalidPositionException{"Invalid position"};

  if (IsValid(pos) && cells_[pos.row][pos.col]) {
    return;
  }

  auto cell = std::make_unique<Cell>(*this, pos); // O(1)
  ExpandToFit(pos);

  if (!IsValid(pos)) {
    throw std::runtime_error("Sheet::SetCell : sheet must have been expanded");
  }
  empty_cells_.insert(cell.get());
  size_monitor_.Add(pos);
  cells_[pos.row][pos.col] = std::move(cell);
}

void Sheet::SetCell(Position pos, std::string text) {
  if (!pos.IsValid())
    throw InvalidPositionException{"Invalid position"};

  if (IsValid(pos)) {
    if (auto &cell = cells_[pos.row][pos.col]) {
      auto old_state = cell->State();
      cell->Set(std::move(text));
      auto new_state = cell->State();
      if (old_state == CellState::kEmpty && new_state != CellState::kEmpty) {
        empty_cells_.erase(cell.get());
        printable_size_monitor_.Add(pos);
      }
      if (old_state != CellState::kEmpty && new_state == CellState::kEmpty &&
          !cell->GetReferencingCells().empty()) {
        empty_cells_.insert(cell.get());
        printable_size_monitor_.Remove(pos);
      }
      return;
    }
  }

  if (text == "") {
    return;
  }

  ExpandToFit(pos);
  if (!IsValid(pos)) {
    throw std::runtime_error("Sheet::SetCell : sheet must have been expanded");
  }

  auto &cell = cells_[pos.row][pos.col];
  if (!cell) {
    cell = std::make_unique<Cell>(*this, pos);
  }
  cell->Set(std::move(text));
  printable_size_monitor_.Add(pos);
  size_monitor_.Add(pos);
  cells_[pos.row][pos.col] = std::move(cell);
}

const ICell *Sheet::GetCell(Position pos) const {
  if (!pos.IsValid())
    throw InvalidPositionException{"Invalid position"};

  if (!IsValid(pos)) {
    return nullptr;
  }
  return cells_[pos.row][pos.col].get();
}

ICell *Sheet::GetCell(Position pos) {
  if (!pos.IsValid())
    throw InvalidPositionException{"Invalid position"};

  if (!IsValid(pos)) {
    return nullptr;
  }
  return cells_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
  if (!pos.IsValid())
    throw InvalidPositionException{"Invalid position"};

  if (!IsValid(pos)) {
    return;
  }
  printable_size_monitor_.Remove(pos);
  size_monitor_.Remove(pos);
  cells_[pos.row][pos.col].reset();
}

void Sheet::InsertRows(int before, int count) {
  before = std::min(16384, std::max(before, 0));
  count = std::min(16384, std::max(count, 0));
  ValidateExpand(before, count, TableItem::kRows);
  UpdateCellsAfterRowAddition(before, count);
  printable_size_monitor_.UpdateAfterRowAddition(before, count);
  size_monitor_.UpdateAfterRowAddition(before, count);
  ExpandTable(before, count, TableItem::kRows);
}
void Sheet::InsertCols(int before, int count) {
  before = std::min(16384, std::max(before, 0));
  count = std::min(16384, std::max(count, 0));
  ValidateExpand(before, count, TableItem::kCols);
  UpdateCellsAfterColAddition(before, count);
  printable_size_monitor_.UpdateAfterColAddition(before, count);
  size_monitor_.UpdateAfterColAddition(before, count);
  ExpandTable(before, count, TableItem::kCols);
}

void Sheet::DeleteRows(int first, int count) {
  first = std::min(16384, std::max(first, 0));
  count = std::min(16384 - first, std::max(count, 0));
  if (count == 0) return;
  InvalidateCells(ShiftType::kRows, first, count);
  UpdateCellsAfterRowDeletion(first, count);
  UpdateEmptyCells();
  printable_size_monitor_.UpdateAfterRowDeletion(first, count);
  size_monitor_.UpdateAfterRowDeletion(first, count);
  cells_.erase(
      begin(cells_) + std::min(static_cast<int>(cells_.size()), first),
      begin(cells_) + std::min(static_cast<int>(cells_.size()), first + count));
}
void Sheet::DeleteCols(int first, int count) {
  first = std::min(16384, std::max(first, 0));
  count = std::min(16384 - first, std::max(count, 0));
  if (count == 0) return;
  InvalidateCells(ShiftType::kCols, first, count);
  UpdateCellsAfterColDeletion(first, count);
  UpdateEmptyCells();
  printable_size_monitor_.UpdateAfterColDeletion(first, count);
  size_monitor_.UpdateAfterColDeletion(first, count);
  for (auto &row : cells_) {
    row.erase(
        begin(row) + std::min(static_cast<int>(row.size()), first),
        begin(row) +
            std::min(static_cast<int>(row.size()), first + count));
  }
}

Size Sheet::GetPrintableSize() const {
  return printable_size_monitor_.GetSize();
}

void Sheet::PrintValues(std::ostream &out) const {
  PrintCells(out, PrintSettings::kValues);
}
void Sheet::PrintTexts(std::ostream &out) const {
  PrintCells(out, PrintSettings::kTexts);
}

void Sheet::PrintCells(std::ostream &out, PrintSettings print_settings) const {
  Size size = GetPrintableSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (j > 0) {
        out << "\t";
      }

      auto cell = GetCell(Position{i, j});
      if (!cell) continue;

      switch (print_settings) {
        case PrintSettings::kValues:
          out << cell->GetValue();
          break;
        case PrintSettings::kTexts:
          out << cell->GetText();
          break;
        default:
          throw std::logic_error("Unknown print settings");
      }
    }
    out << "\n";
  }
}

bool Sheet::IsValid(Position pos) const {
  if (pos.row >= cells_.size() || pos.col >= cells_[pos.row].size())
    return false;
  return true;
}

void Sheet::UpdateEmptyCells() {
  std::vector<Cell *> cells_to_delete;

  for (auto cell : empty_cells_) {
    if (!cell)
      throw std::runtime_error("Sheet::UpdateEmptyCells : cell shouldn't be "
                               "invalid here");
    if (cell->State() == CellState::kEmpty &&
        cell->GetReferencingCells().empty()) {
      cells_to_delete.push_back(cell);
    }
  }

  for (Cell *cell : cells_to_delete) {
    empty_cells_.erase(cell);
    auto [x, y] = cell->GetPosition();
    cells_[x][y].reset();
  }
}

void Sheet::ExpandToFit(Position pos) {
  if (pos.row >= cells_.size()) {
    cells_.resize(static_cast<size_t>(pos.row) + 1);
  }
  if (pos.col >= cells_[pos.row].size()) {
    cells_[pos.row].resize(static_cast<size_t>(pos.col) + 1);
  }
}

void Sheet::ValidateExpand(int before, int count, TableItem item) {
  if (item == TableItem::kRows) {
    if (before + count >= Position::kMaxRows ||
        size_monitor_.GetSize().rows + count >= Position::kMaxRows) {
      throw TableTooBigException("Rows capacity exceeded");
    }
  }
  if (item == TableItem::kCols) {
    if (before + count >= Position::kMaxCols ||
        size_monitor_.GetSize().cols + count >= Position::kMaxCols) {
      throw TableTooBigException("Rows capacity exceeded");
    }
  }
}

void Sheet::ExpandTable(int before, int count, TableItem item) {
  if (item == TableItem::kRows) {
    if (before >= cells_.size()) return;
    std::vector<std::vector<std::unique_ptr<Cell>>> vec(count);
    cells_.insert(begin(cells_) + before,
                  std::make_move_iterator(begin(vec)),
                  std::make_move_iterator(end(vec)));
  }
  if (item == TableItem::kCols) {
    for (int i = 0; i < printable_size_monitor_.GetSize().rows; ++i) {
      if (before >= cells_[i].size()) continue;
      std::vector<std::unique_ptr<Cell>> vec(count);
      cells_[i].insert(begin(cells_[i]) + before,
                       std::make_move_iterator(begin(vec)),
                       std::make_move_iterator(end(vec)));
    }

    std::vector<int> cols_to_expand;
    for (auto cell : empty_cells_) {
      auto row = cell->GetPosition().row;
      if (row >= printable_size_monitor_.GetSize().rows) {
        cols_to_expand.push_back(row);
      }
    }
    for (int row : cols_to_expand) {
      if (before >= cells_[row].size()) continue;
      std::vector<std::unique_ptr<Cell>> vec(count);
      cells_[row].insert(begin(cells_[row]) + before,
                         std::make_move_iterator(begin(vec)),
                         std::make_move_iterator(end(vec)));
    }
  }
}

void Sheet::InvalidateCells(ShiftType type, int first, int count) {
  std::stack<Position> st;
  switch (type) {
    case ShiftType::kRows: {
      auto rows = printable_size_monitor_.GetSize().rows;
      size_t begin = std::min<size_t>(std::max(first, 0), rows);
      size_t end = std::min<size_t>(std::max(first + count, 0), rows);

      for (auto i = begin; i < end; ++i) {
        for (auto &cell : cells_[i]) {
          if (!cell) continue;
          cell->SetState(CellState::kRefError);
          for (auto ref : cell->GetReferencingCells()) {
            st.push(ref);
          }
        }
      }
      break;
    }
    case ShiftType::kCols: {
      for (int i = 0; i < printable_size_monitor_.GetSize().rows; ++i) {
        size_t begin = std::min<size_t>(std::max(first, 0), cells_[i].size());
        size_t end = std::min<size_t>(std::max(first + count, 0),
                                      cells_[i].size());

        for (auto j = begin; j < end; ++j) {
          auto &cell = cells_[i][j];
          if (!cell) continue;
          cell->SetState(CellState::kRefError);
          for (auto ref : cell->GetReferencingCells()) {
            st.push(ref);
          }
        }
      }
      break;
    }
    default:
      throw std::runtime_error("Unexpected shift type");
  }

  std::unordered_set < Position, PositionHash > visited;

  while (!st.empty()) {
    auto pos = st.top();
    st.pop();

    auto cell = dynamic_cast<Cell *>(GetCell(pos));
    if (!cell) {
      throw std::runtime_error("InvalidateCells: null cell: failed precondition");
    }
    if (!visited.insert(pos).second || cell->State() == CellState::kRefError)
      continue;
    cell->SetState(CellState::kRefError);

    for (auto ref : cell->GetReferencingCells()) {
      st.push(ref);
    }
  }
}

void Sheet::UpdateCellsAfterRowAddition(int first_idx, int count) {
  auto size = printable_size_monitor_.GetSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (!IsValid({i, j}) || !cells_[i][j]) continue;
      cells_[i][j]->HandleInsertedRows(first_idx, count);
    }
  }
}

void Sheet::UpdateCellsAfterColAddition(int first_idx, int count) {
  auto size = printable_size_monitor_.GetSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (!IsValid({i, j}) || !cells_[i][j]) continue;
      cells_[i][j]->HandleInsertedCols(first_idx, count);
    }
  }
}

void Sheet::UpdateCellsAfterRowDeletion(int first_idx, int count) {
  auto size = printable_size_monitor_.GetSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (!IsValid({i, j}) || !cells_[i][j]) continue;
      cells_[i][j]->HandleDeletedRows(first_idx, count);
    }
  }
}

void Sheet::UpdateCellsAfterColDeletion(int first_idx, int count) {
  auto size = printable_size_monitor_.GetSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (!IsValid({i, j}) || !cells_[i][j]) continue;
      cells_[i][j]->HandleDeletedCols(first_idx, count);
    }
  }
}

std::unique_ptr<ISheet> CreateSheet() {
  return std::make_unique<Sheet>();
}
