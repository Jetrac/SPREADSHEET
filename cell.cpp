#include "cell.h"

#include <cassert>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <variant>

namespace detail {
    class Impl {
    public:
        using Value = Cell::Value;

        virtual std::string GetText() const = 0;

        virtual Value GetValue(const SheetInterface &sheet) const = 0;

        virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    class EmptyImpl final : public Impl {
    public:
        std::string GetText() const override {
            using namespace std::literals::string_literals;
            return ""s;
        }

        Value GetValue(const SheetInterface &sheet) const override {
            using namespace std::literals::string_literals;
            return ""s;
        }

        std::vector<Position> GetReferencedCells() const override {
            return {};
        }
    };

    class TextImpl final : public Impl {
    public:
        TextImpl(std::string text) : text_(text) {}

        std::string GetText() const override {
            return text_;
        }

        Value GetValue(const SheetInterface &) const override {
            if (!text_.empty() && text_[0] == ESCAPE_SIGN) {
                return text_.substr(1);
            }

            return text_;
        }

        std::vector<Position> GetReferencedCells() const override {
            return {};
        }

    private:
        std::string text_;
    };

    class FormulaImpl final : public Impl {
    public:
        FormulaImpl(std::string expression) : formula_(ParseFormula(expression)) {}

        std::string GetText() const override {
            return FORMULA_SIGN + formula_->GetExpression();
        }

        Value GetValue(const SheetInterface &sheet) const override {
            std::variant<double, FormulaError> res = formula_->Evaluate(sheet);

            if (auto v = std::get_if<double>(&res)) {
                return *v;
            } else if (auto v = std::get_if<FormulaError>(&res)) {
                return *v;
            }

            assert(false);
            return {};
        }

        std::vector<Position> GetReferencedCells() const override {
            return formula_->GetReferencedCells();
        }

        bool HasCache() const { return formula_->HasCache(); }

        void ClearCache() { formula_->ClearCache(); }

    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

}  // namespace detail
//----------------------------------------------------------------------------------------------------------------------
Cell::Cell(const SheetInterface &sheet,Position pos ) :sheet_(sheet),pos_(pos) {}
Cell::~Cell() {}

void Cell::Set(std::string text) {
    EraseEdges();
    if (text.empty()) {
        impl_ = std::make_unique<detail::EmptyImpl>();
    } else if (text[0] == FORMULA_SIGN && text.size() > 1u) {
        std::unique_ptr<detail::FormulaImpl> formula_impl = std::make_unique<detail::FormulaImpl>(text.substr(1));

        CheckCircularError(formula_impl->GetReferencedCells());
        AddEdges(formula_impl->GetReferencedCells());

        impl_ = std::move(formula_impl);
    } else {
        impl_ = std::make_unique<detail::TextImpl>(text);
    }
}

void Cell::CheckCircularError(const std::vector<Position> &references) const {
    std::unordered_set<Position, Position::Hash> visited_cells;
    CheckCircularError(pos_, references, visited_cells);
}

void Cell::CheckCircularError(
        const Position &this_cell_pos, const std::vector<Position> &references,
        std::unordered_set<Position, Position::Hash> &visited_cells
) const {
    for (const Position &ref_cell_pos: references) {
        if (visited_cells.count(ref_cell_pos)) { continue; }
        if (this_cell_pos == ref_cell_pos) { throw CircularDependencyException{"Circular dependency found"}; }

        visited_cells.insert(ref_cell_pos);

        const CellInterface *ref_cell = sheet_.GetCell(ref_cell_pos);

        if (ref_cell == nullptr) { continue; }

        CheckCircularError(this_cell_pos, ref_cell->GetReferencedCells(), visited_cells);
    }
}
//----------------------------------------------------------------------------------------------------------------------
void Cell::ClearCaches(const std::unordered_set<Position, Position::Hash> &reffering_cells) {
    for (const Position &cell_pos: reffering_cells) {
        const Cell *cell = dynamic_cast<const Cell *>(sheet_.GetCell(cell_pos));
        detail::FormulaImpl *formula_impl = dynamic_cast<detail::FormulaImpl *>(cell->impl_.get());

        if (!formula_impl->HasCache()) { continue; }

        formula_impl->ClearCache();

        ClearCaches(cell->referring_cells_);
    }
}
//----------------------------------------------------------------------------------------------------------------------
void Cell::AddEdges(const std::vector<Position> &references) {
    using namespace std::literals::string_literals;

    for (const Position &pos: references) {
        if (sheet_.GetCell(pos) == nullptr) {
            SheetInterface &non_const_sheet = const_cast<SheetInterface &>(sheet_);
            non_const_sheet.SetCell(pos, ""s);
        }

        referenced_cells_.insert(pos);

        const CellInterface *ref_cell_interface = sheet_.GetCell(pos);
        Cell *ref_cell = const_cast<Cell *>(dynamic_cast<const Cell *>(ref_cell_interface));

        ref_cell->referring_cells_.insert(pos_);
    }
}

void Cell::EraseEdges() {
    ClearCaches(referring_cells_);

    for (const Position &cell_pos: referenced_cells_) {
        const CellInterface *cell_interface = sheet_.GetCell(cell_pos);
        if (!cell_interface) { continue; }

        Cell *cell = const_cast<Cell *>(dynamic_cast<const Cell *>(cell_interface));
        cell->referring_cells_.erase(pos_);
    }

    referenced_cells_.clear();
}
//----------------------------------------------------------------------------------------------------------------------
std::string Cell::GetText() const { return impl_->GetText(); }

Cell::Value Cell::GetValue() const { return impl_->GetValue(sheet_); }

std::vector<Position> Cell::GetReferencedCells() const { return impl_->GetReferencedCells(); }
//----------------------------------------------------------------------------------------------------------------------
