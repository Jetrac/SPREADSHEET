#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "FormulaAST.h"
#include "common.h"

std::ostream& operator<<(std::ostream& output, FormulaError fe);

class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() = default;

    virtual Value Evaluate(const SheetInterface& sheet) const = 0;

    virtual bool HasCache() const = 0;
    virtual void ClearCache() = 0;

    virtual std::string GetExpression() const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);
