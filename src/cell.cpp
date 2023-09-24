#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <unordered_map>


class Cell::EmptyImpl final: public Cell::Impl
{
public:
    void Set(std::string) override {}
    CellInterface::Value GetValue() const override { return {""}; }
    std::string GetText() const override { return {""}; }
    CellType GetType() const override { return CellType::CELL_EMPTY; }
    std::optional<std::vector<Position>> GetReferencedCells() const override { return std::nullopt; }
};

class Cell::TextImpl final: public Cell::Impl
{
public:
    void Set(std::string text) override
    {
        text_ = std::move(text);
    }

    CellInterface::Value GetValue() const override
    {
        if (text_[0] == ESCAPE_SIGN)
        {
            return {text_.substr(1)};
        }
        return text_;
    }

    CellType GetType() const override { return CellType::CELL_TEXT; }

    std::string GetText() const override
    {
        return text_;
    }

    std::optional<std::vector<Position>> GetReferencedCells() const override { return std::nullopt; }
private:
    std::string text_;
};

class Cell::FormulaImpl final: public Cell::Impl
{
public:

    FormulaImpl(SheetInterface& sheet): sheet_(sheet) {}
    void Set(std::string text) override
    {
        formula_ = ParseFormula(std::move(text));
    }

    CellInterface::Value GetValue() const override
    {
        if (!formula_)
        {
            return FormulaError(FormulaError::Category::Value);
        }

        auto value = formula_->Evaluate(sheet_);

        struct
        {
            CellInterface::Value operator()(double val)
            {
                CellInterface::Value out{val};
                return out;
            }

            CellInterface::Value operator()(FormulaError val)
            {
                CellInterface::Value out{val};
                return out;
            }
        } vsiter;

        return std::visit(vsiter, value);
    }

    CellType GetType() const override { return CellType::CELL_FORMULA; }

    std::string GetText() const override
    {
        return "=" + formula_->GetExpression();
    }

    std::optional<std::vector<Position>> GetReferencedCells() const override
    {
        return formula_->GetReferencedCells();
    }

private:
    SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_ = nullptr;
};

Cell::Cell(SheetInterface& sheet): sheet_(sheet), impl_(std::make_unique<Cell::EmptyImpl>()){}

Cell::CellType Cell::GetType() const
{
    if (!impl_)
    {
        return CellType::CELL_EMPTY;
    }

    return impl_->GetType();
}

void Cell::Set(Position pos, std::string text) 
{

    if (text.size() > 1 && text[0] == FORMULA_SIGN)
    {
        auto imp_buff = std::make_unique<Cell::FormulaImpl>(sheet_);
        imp_buff->Set(std::move(text.substr(1)));

        auto referenced_cells = imp_buff->GetReferencedCells();

        if (referenced_cells)
        {
            for (auto pos_cild : *referenced_cells)
            {
                children_.insert(pos_cild);
            }

            CycleRefDetection(pos);
        }

        impl_ = std::move(imp_buff);
    }
    else if (!text.empty())
    {
        auto imp_buff = std::make_unique<Cell::TextImpl>();
        imp_buff->Set(std::move(text));
        impl_ = std::move(imp_buff);
    }
    else 
    {
        impl_ = std::make_unique<Cell::EmptyImpl>();
    }

    AlertChildrenNewParent(pos);
    InvalidagteCache();
}

void Cell::Clear()
{
    InvalidagteCache();
    impl_ = std::make_unique<Cell::EmptyImpl>();
}

Cell::Value Cell::GetValue() const
{
    if (!impl_)
    {
        return "";
    }

    return impl_->GetValue();
}

std::string Cell::GetText() const
{
    if (!impl_)
    {
        return "";
    }

    return impl_->GetText();
}

void Cell::AddParent(Position pos)
{
    parents_.insert(pos);
}


std::vector<Position> Cell::GetReferencedCells() const
{
    auto buff = impl_->GetReferencedCells();

    if (buff)
    {
        return std::move(*buff);
    }

    return {};
}

    void Cell::CycleRefDetection(Position curr_pos) const
    {
        for (auto cell : children_)
        {
            std::unordered_set<Position, Position::PositionHasher> visited{curr_pos};

            if (CycleDetector(visited, cell))
            {
                throw CircularDependencyException{""};
            }
        }
    }

    bool Cell::CycleDetector(std::unordered_set<Position, Position::PositionHasher>& visited, Position pos) const
    {
        if (!pos.IsValid())
        {
            return false;
        }

        if (visited.find(pos) != visited.end())
        {
            return true;
        }

        auto cell_ptr = sheet_.GetCell(pos);
        if (!cell_ptr)
        {
            return false;
        }

        visited.insert(pos);

        const auto cell_refs = static_cast<const Cell*>(cell_ptr)->children_;

        for (auto cell_pos : cell_refs)
        {
            if (CycleDetector(visited, cell_pos))
            {
                return true;
            }
        }

        return false;
    }

void Cell::AlertChildrenNewParent(Position curr_pos)
{
    for (auto cell : children_)
    {
        if (!cell.IsValid())
        {
            continue;
        }

        auto cell_ptr = sheet_.GetCell(cell);

        if (cell_ptr)
        {
            static_cast<Cell*>(cell_ptr)->AddParent(curr_pos);
        }
    }
}

void Cell::InvalidagteCache(bool is_start)
{
    if (cache_ || is_start)
    {
        cache_ = std::nullopt;
        for (auto cell : parents_)
        {
            auto cell_ptr = sheet_.GetCell(cell);
            reinterpret_cast<Cell*>(cell_ptr)->InvalidagteCache(false);
        }
    }
}