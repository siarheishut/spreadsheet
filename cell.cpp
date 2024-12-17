#include <memory>
#include <string>
#include <vector>
#include <unordered_set>

#include "cell.h"
#include "formula.h"
#include "utils.h"

Cell::Cell(Sheet &sheet, Position position)
    : sheet_(sheet),
      pos_in_sheet_(position),
      internal_data_({.data = std::make_unique<cell_data::Text>("")}) {}

void Cell::Set(std::string text) {
  if (last_set_args_) {
    if (last_set_args_->second && text == last_set_args_->first) {
      throw CircularDependencyException("Circular dependency appeared");
    }
    if (text == last_set_args_->first) {
      return;
    }
  }

  CellState state;
  std::unique_ptr<ICellData> data;

  if (text.empty()) {
    state = CellState::kEmpty;
    data = std::make_unique<cell_data::Text>("");
  } else if (text.size() > 1 && text[0] == kFormulaSign) {
    state = CellState::kFormula;
    data = std::make_unique<cell_data::Formula>(text.substr(1), sheet_);
  } else {
    state = CellState::kText;
    data = std::make_unique<cell_data::Text>(text);
  }

  if (State() == CellState::kEmpty && state == CellState::kEmpty)
    return;
  if (State() == CellState::kText && state == CellState::kText &&
      GetText() == data->GetText())
    return;
  if (State() != CellState::kEmpty && state != CellState::kEmpty &&
      State() != CellState::kText && state != CellState::kText &&
      GetText() == data->GetText())
    return;

  if (IsAddingCircularDependency(*data)) {
    last_set_args_ = {std::move(text), true};
    throw CircularDependencyException("Circular dependency appeared");
  }

  ClearRefs();
  internal_data_.data = std::move(data);
  internal_data_.state = state;
  internal_data_.referenced_cells = internal_data_.data->GetReferencedCells();
  // internal_data_.referencing_cells - stays unchanged
  SetRefs();

  ResetCache(true);
  last_set_args_ = {std::move(text), false};
}

std::vector<Position> Cell::GetReferencedCells() const {
  return internal_data_.referenced_cells;
}

std::string Cell::GetText() const {
  if (!internal_data_.data) {
    throw std::runtime_error("Cell::GetText : Data can't be not initialized "
                             "here");
  }
  return internal_data_.data->GetText();
}

ICell::Value Cell::GetValue() const {
  if (State() == CellState::kRefError) {
    return FormulaError{FormulaError::Category::Ref};
  } else if (State() == CellState::kValueError) {
    return FormulaError{FormulaError::Category::Value};
  } else if (State() == CellState::kDiv0Error) {
    return FormulaError{FormulaError::Category::Div0};
  }
  if (!internal_data_.data) {
    throw std::runtime_error("Cell::GetValue : Data can't be not initialized "
                             "here");
  }
  return internal_data_.data->GetValue();
}

std::unordered_set<Position, PositionHash> Cell::GetReferencingCells() const {
  return internal_data_.referencing_cells;
}

CellState Cell::State() const {
  return internal_data_.state;
}

void Cell::SetState(CellState state) {
  internal_data_.state = state;
}

void Cell::ClearRefs() {
  for (Position pos : internal_data_.referenced_cells) {
    auto cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    if (!cell)
      throw std::runtime_error(
          "clear refs: failed precondition: null referenced cell");
    cell->internal_data_.referencing_cells.erase(pos_in_sheet_);
  }
}

void Cell::SetRefs() {
  for (auto pos : internal_data_.referenced_cells) {
    auto cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    if (!cell) {
      sheet_.ForceInitializeCell(pos);
      cell = dynamic_cast<Cell *>(sheet_.GetCell(pos));
    }
    if (!cell) {
      throw std::runtime_error("ebat kopat");
    }
    cell->internal_data_.referencing_cells.insert(pos_in_sheet_);
  }
}

void Cell::ResetCache(bool force) {
  if (internal_data_.data->IsCached() || force) {
    internal_data_.data->ResetCache();
    last_set_args_.reset();

    for (auto ref : internal_data_.referencing_cells) {
      if (!sheet_.GetCell(ref)) {
        throw std::runtime_error("ukusi menya pchela");
      }
      if (auto cell = dynamic_cast<Cell *>(sheet_.GetCell(ref))) {
        cell->ResetCache(false);
      } else {
        throw std::runtime_error("Cell::ResetCache : cell shouldn't be null"
                                 "here");
      }
    }
  }
}

IFormula::HandlingResult Cell::HandleInsertedRows(int before, int count) {
  ResetCache(true);
  if (!internal_data_.data) {
    throw std::runtime_error("Cell::HandleInsertedRows : Data can't be not "
                             "initialized");
  }
  auto res = internal_data_.data->HandleInsertedRows(before, count);
  if (pos_in_sheet_.row >= before) {
    internal_data_.referenced_cells = internal_data_.data->GetReferencedCells();
    pos_in_sheet_.row += before;
  }
  return res;
}

IFormula::HandlingResult Cell::HandleInsertedCols(int before, int count) {
  ResetCache(true);
  if (!internal_data_.data) {
    throw std::runtime_error("Cell::HandleInsertedCols : Data can't be not "
                             "initialized");
  }
  auto res = internal_data_.data->HandleInsertedCols(before, count);
  if (pos_in_sheet_.col >= before) {
    internal_data_.referenced_cells = internal_data_.data->GetReferencedCells();
    pos_in_sheet_.col += before;
  }
  return res;
}

IFormula::HandlingResult Cell::HandleDeletedRows(int first, int count) {
  ResetCache(true);
  if (!internal_data_.data) {
    throw std::runtime_error("Cell::HandleDeletedRows : Data can't be not "
                             "initialized");
  }
  auto res = internal_data_.data->HandleDeletedRows(first, count);
  ClearRefs();
  internal_data_.referenced_cells = internal_data_.data->GetReferencedCells();
  if (pos_in_sheet_.row >= first + count) {
    pos_in_sheet_.row -= count;
  } else if (pos_in_sheet_.row >= first) {
    return res;
  }
  SetRefs();
  return res;
}

IFormula::HandlingResult Cell::HandleDeletedCols(int first, int count) {
  ResetCache(true);
  if (!internal_data_.data) {
    throw std::runtime_error("Cell::HandleDeletedCols : Data can't be not "
                             "initialized");
  }
  auto res = internal_data_.data->HandleDeletedCols(first, count);
  ClearRefs();
  internal_data_.referenced_cells = internal_data_.data->GetReferencedCells();
  if (pos_in_sheet_.col >= first + count) {
    pos_in_sheet_.col -= count;
  } else if (pos_in_sheet_.col >= first) {
    return res;
  }
  SetRefs();
  return res;
}

bool Cell::IsAddingCircularDependency(const ICellData &new_data) const {
  std::unordered_set < Position, PositionHash > visited;
  std::stack<Position> st;
  for (auto ref : new_data.GetReferencedCells()) {
    st.push(ref);
  }
  while (!st.empty()) {
    auto node = st.top();
    st.pop();
    if (node == pos_in_sheet_) return true;
    if (!visited.insert(node).second) continue;
    if (auto cell = sheet_.GetCell(node)) {
      for (auto ref : cell->GetReferencedCells()) {
        st.push(ref);
      }
    }
  }
  return false;
}
