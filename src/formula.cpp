#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <functional>
#include <unordered_set>
#include <charconv>

using namespace std::literals;

static double StringConvertToDigit(const std::string& str)
{
    double out = 0;

    if (str.empty())
    {
        return out;
    }

    const std::from_chars_result res = std::from_chars(str.data(), str.data() + str.size(), out);

    
    if (res.ec != std::errc() || res.ptr != str.data() + str.size())
    {
        throw FormulaError(FormulaError::Category::Value);
    }

    return out;
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) 
{
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:

    explicit Formula(std::string expression): ast_(ParseFormulaAST(expression)) {}

    Value Evaluate(const SheetInterface& sheet) const override
    {
        FormulaInterface::Value out;

        auto funct = [&sheet](Position args) 
        {
            if (!args.IsValid())
            {
                throw FormulaError(FormulaError::Category::Ref);
            }

            double out = 0;

            auto cell_buf = sheet.GetCell(args);
            auto cell_value = cell_buf ? cell_buf->GetValue() : 0.0;

            if (std::holds_alternative<std::string>(cell_value))
            {
                out = StringConvertToDigit(std::get<std::string>(cell_value));
            }
            else if (std::holds_alternative<FormulaError>(cell_value))
            {
                throw std::get<FormulaError>(cell_value);
            }
            else if (std::holds_alternative<double>(cell_value))
            {
                out = std::get<double>(cell_value);
            }

            return out;
        };

        try
        {
            out = ast_.Execute(funct);
        }
        catch (const FormulaError& e)
        {
            out = e;
        }

        return out;
    }

    std::string GetExpression() const override 
    {
        std::stringstream out;
        ast_.PrintFormula(out);

        return out.str();
    }

        virtual std::vector<Position> GetReferencedCells() const 
    {
        std::vector<Position> out;
        std::unordered_set<Position, Position::PositionHasher> buff;

        for (auto ref : ast_.GetCells())
        {
            buff.insert(ref);
        }
        out.assign(buff.begin(), buff.end());
        std::sort(out.begin(), out.end());

        return out;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) 
{
    return std::make_unique<Formula>(std::move(expression));
}