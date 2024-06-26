#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>

#include "cell.h"
#include "common.h"

using namespace std::literals;

Sheet::Sheet() : size_{ 0, 0 } {}

Sheet::~Sheet() {}

//----------------------------------------------------------------------------------------------------------------------
std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::ResizeTable(const Position &pos) {
    if (pos.row >= static_cast<int>(cells_.size())) {
        cells_.resize(pos.row + 1);
    }

    auto &row = cells_[pos.row];
    if (pos.col >= static_cast<int>(row.size())) {
        row.resize(pos.col + 1);
    }
}
//----------------------------------------------------------------------------------------------------------------------
void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        CheckInvalidPosition(pos);
    }

    if (auto p = GetCell(pos)) {
        if (dynamic_cast<const Cell *>(p)->GetText() == text) {
            return;
        }

        cells_[pos.row][pos.col]->Set(text);
        return;
    }

    std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this, pos);
    cell->Set(text);

    ResizeTable(pos);
    cells_[pos.row][pos.col] = std::move(cell);

    if (!text.empty()) {
        SetSize(pos);
    }
}

const CellInterface *Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        CheckInvalidPosition(pos);
    }

    if (pos.row >= static_cast<int>(cells_.size()) || pos.col >= static_cast<int>(cells_[pos.row].size())) {
        return nullptr;
    }

    return cells_[pos.row][pos.col].get();
}

CellInterface *Sheet::GetCell(Position pos) {
    return const_cast<CellInterface *>(const_cast<const Sheet &>(*this).GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    using namespace std::literals::string_literals;

    if (!pos.IsValid()) {
        CheckInvalidPosition(pos);
    }

    if (pos.row >= static_cast<int>(cells_.size()) || pos.col >= static_cast<int>(cells_[pos.row].size())) {
        return;
    }

    cells_[pos.row][pos.col]->Set(""s);

    if (pos.row + 1 == size_.rows || pos.col + 1 == size_.cols) {
        RecalculateSize();
    }
}
//----------------------------------------------------------------------------------------------------------------------
Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::RecalculateSize() {
    int max_col = -1;
    int max_row = -1;

    for (int row = 0; row < static_cast<int>(cells_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(cells_[row].size()); ++col) {
            if (auto p = GetCell({row, col}); p != nullptr && !p->GetText().empty()) {
                max_col = std::max(max_col, col);
                max_row = std::max(max_row, row);
            }
        }
    }

    size_.rows = max_row + 1;
    size_.cols = max_col + 1;
}

void Sheet::SetSize(const Position &pos) {
    if (pos.row + 1 > size_.rows) {
        size_.rows = pos.row + 1;
    }
    if (pos.col + 1 > size_.cols) {
        size_.cols = pos.col + 1;
    }
}
//----------------------------------------------------------------------------------------------------------------------
void Sheet::PrintTexts(std::ostream &output) const {
    PrintTextOrValue(output, detail::CellType::TEXT);
}

void Sheet::PrintValues(std::ostream &output) const {
    PrintTextOrValue(output, detail::CellType::VALUE);
}

void Sheet::PrintTextOrValue(std::ostream &output, detail::CellType type) const {
    for (int row = 0; row < size_.rows; ++row) {
        bool first = true;

        for (int col = 0; col < size_.cols; ++col) {
            if (first) {
                first = false;
            } else {
                output << '\t';
            }

            if (auto p = GetCell({row, col})) {
                output << (type == detail::CellType::TEXT ? p->GetText() : p->GetValue());
            }
        }
        output << '\n';
    }
}
//----------------------------------------------------------------------------------------------------------------------

void Sheet::CheckInvalidPosition(const Position &pos) const {
    throw InvalidPositionException{"Invalid position: "s + std::to_string(pos.row) + ", "s + std::to_string(pos.col)};
}
//----------------------------------------------------------------------------------------------------------------------
