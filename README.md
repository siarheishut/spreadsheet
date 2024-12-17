# Spreadsheet Engine

This project involves building a spreadsheet engine similar to Microsoft Excel or Google Sheets, with support for cell references, formulas, and operations like row/column insertion and deletion. The engine is implemented in C++ and uses ANTLR 4 for parsing formula expressions.

---

## Features

### **Cells**
- Cells are addressed using position (e.g., "A1", "B2") or zero-indexed row/column pairs (e.g., `(0, 0)` for "A1").
- Supported cell content:
    - **Text:** Raw strings.
    - **Formulas:** Arithmetic expressions beginning with `=`.

### **Formulas**
- A formula evaluates to a number and supports:
    - Constants (`123`, `3.14`)
    - Binary operations (`+`, `-`, `*`, `/`)
    - Cell references (e.g., `A1 + B2 * C3`)
- Errors:
    - `#REF!` for invalid references.
    - `#VALUE!` for invalid numerical conversions.
    - `#DIV/0!` for division by zero or overflow.

### **Row and Column Operations**
- Insert/Delete rows or columns, updating formulas accordingly.
- Ensure the references remain valid or are replaced by `#REF!` if deleted.

### **Efficient Dependencies Management**
- Uses a **dependency graph** to handle cached values and dependency updates efficiently.

### **Printing**
- Prints the smallest rectangle encompassing non-empty cells.
- Supports separate views for raw text and evaluated values.

---

## Implementation Details

### **Parsing with ANTLR 4**
ANTLR generates a parser from a predefined grammar for formulas. This grammar handles precedence and operations like addition, multiplication, and cell references.

### **Position Handling**
- Converts between string and numeric representations using `Position::FromString()` and `Position::ToString()`.
- Handles up to 16,384 rows and columns.

### **Caching**
- Ensures O(1) complexity for value retrieval if dependencies remain unchanged.

### **Error Handling**
Handles:
- Invalid positions (`InvalidPositionException`).
- Syntax errors in formulas (`FormulaException`).
- Circular references (`CircularDependencyException`).
- Table size violations (`TableTooBigException`).

---

## Performance Expectations

| Operation                           | Complexity |
|-------------------------------------|------------|
| Any `ISheet` method call            | O(K)       |
| `GetCell()`                         | O(1)       |
| `ICell::GetValue()`                 | O(K)       |
| `ICell::GetText()`                  | O(1)       |
| `ICell::GetReferencedCells()`       | O(1)       |
| Repeated `SetCell()` with same args | O(1)       |
| Cached `GetValue()`                 | O(1)       |

> **Note:** `K` refers to the number of non-empty cells in the printable area.

## Error Messages

| Error         | Description                                                                 |
|---------------|-----------------------------------------------------------------------------|
| `#REF!`       | Reference to a deleted or out-of-bounds cell.                              |
| `#VALUE!`     | Invalid formula referencing non-numeric content.                           |
| `#DIV/0!`     | Division by zero or arithmetic overflow.                                   |
| Exceptions    | Throw `InvalidPositionException`, `FormulaException`, or `CircularDependencyException` as needed. |

---

## Design Patterns

1. **Dependency Graph:** Track relationships between cells to manage formula updates efficiently.
2. **Caching:** Store computed values for reuse unless invalidated by updates.
3. **Dynamic Memory for Cells:** Store pointers to avoid rigid positional ties.

---

## Testing

### Formula Parsing
Use provided unit tests for grammar rules and parsing tree generation.

### Edge Cases
- Circular dependencies.
- Invalid and boundary cases for positions and formulas.

---
