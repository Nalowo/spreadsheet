#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set>

class Cell : public CellInterface 
{
public:
    enum class CellType: char {
        CELL_EMPTY,
        CELL_TEXT = ESCAPE_SIGN,
        CELL_FORMULA = FORMULA_SIGN
    };

    Cell(SheetInterface& sheet);
    ~Cell() = default;

    void Set(Position pos, std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    CellType GetType() const;

    std::vector<Position> GetReferencedCells() const override;

private:

    void CycleRefDetection(Position curr_pos) const;
    bool CycleDetector(std::unordered_set<Position, Position::PositionHasher>& visited, Position pos) const;
    void AddParent(Position pos);
    void AlertChildrenNewParent(Position curr_pos);
    void InvalidagteCache(bool is_start = true);

    class Impl
    {
    public:

        struct ImplPtrHasher
        {
            size_t operator()(const Impl* ptr) const noexcept
            {
                return std::hash<const void*>{}(ptr);
            }
        };

        virtual ~Impl() = default;
        virtual void Set(std::string text) = 0;
        virtual CellInterface::Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual CellType GetType() const = 0;
        virtual std::optional<std::vector<Position>> GetReferencedCells() const = 0;
    };
    
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Position, Position::PositionHasher> children_;
    std::unordered_set<Position, Position::PositionHasher> parents_;
    mutable std::optional<double> cache_;
};