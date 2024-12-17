#include "sheet_size_monitor.h"

void SheetSizeMonitor::Add(Position pos) {
  non_empty_cells_.insert(pos);
  if (max_col_.has_value()) {
    max_col_ = std::max(*max_col_, pos.col);
  } else {
    max_col_ = pos.col;
  }
}

void SheetSizeMonitor::Remove(Position pos) {
  non_empty_cells_.erase(pos);
  max_col_.reset();
}

Size SheetSizeMonitor::GetSize() const {
  if (non_empty_cells_.empty()) {
    return {0, 0};
  }

  auto row = prev(non_empty_cells_.end())->row;
  if (max_col_) {
    return {row + 1, *max_col_ + 1};
  }

  max_col_ = 0;
  for (auto pos : non_empty_cells_) {
    max_col_ = std::max(*max_col_, pos.col);
  }

  return {row + 1, *max_col_ + 1};
}

void SheetSizeMonitor::UpdateAfterRowAddition(int first_idx, int count) {
  for (auto &pos : non_empty_cells_) {
    if (pos.row < first_idx) continue;
    const_cast<Position &>(pos).row = pos.row + count;
  }
}

void SheetSizeMonitor::UpdateAfterColAddition(int first_idx, int count) {
  for (auto &pos : non_empty_cells_) {
    if (pos.col < first_idx) continue;
    const_cast<Position &>(pos).col = pos.col + count;
  }
  max_col_.reset();
}

void SheetSizeMonitor::UpdateAfterRowDeletion(int first_idx, int count) {
  for (auto it = begin(non_empty_cells_);
       it != non_empty_cells_.end();) {
    if (it->row >= first_idx && it->row < first_idx + count) {
      it = non_empty_cells_.erase(it);
    } else {
      it = next(it);
    }
  }

  for (auto &pos : non_empty_cells_) {
    if (pos.row >= first_idx + count) {
      const_cast<Position &>(pos).row = pos.row - count;
    }
  }
  max_col_.reset();
}

void SheetSizeMonitor::UpdateAfterColDeletion(int first_idx, int count) {
  for (auto it = begin(non_empty_cells_);
       it != non_empty_cells_.end();) {
    if (it->col >= first_idx && it->col < first_idx + count) {
      it = non_empty_cells_.erase(it);
    } else {
      it = next(it);
    }
  }

  for (auto &pos : non_empty_cells_) {
    if (pos.col >= first_idx + count) {
      const_cast<Position &>(pos).col = pos.col - count;
    }
  }
  max_col_.reset();
}
