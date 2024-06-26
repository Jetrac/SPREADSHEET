#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "cell.h"
#include "common.h"

namespace detail {
    enum class CellType {
        TEXT,
        VALUE
    };
}

class Sheet : public SheetInterface {
public:
    using Table = std::vector<std::vector<std::unique_ptr<CellInterface>>>;
private:
    Table cells_;
    Size size_;
public:
    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintTexts(std::ostream& output) const override;
    void PrintValues(std::ostream& output) const override;
private:
    void ResizeTable(const Position& pos);

    void PrintTextOrValue(std::ostream& output, detail::CellType type) const;

    void CheckInvalidPosition(const Position& pos) const;

    void RecalculateSize();

    void SetSize(const Position& pos);
};
