#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <cmath>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid position");
    }

    auto cell_iter = sheet_2d_.find(pos);

    if (cell_iter == sheet_2d_.end())
    {
        cell_iter = sheet_2d_.insert({pos, std::make_unique<Cell>(*this)}).first;
    }

    cell_iter->second->Set(pos, std::move(text));

    if (sheet_2d_.at(pos)->GetType() != Cell::CellType::CELL_EMPTY)
    {
        if (row_metadata.size() < static_cast<size_t>(pos.row + 1))
        {
            row_metadata.resize(row_metadata.size() + ((static_cast<size_t>(pos.row + 1)) - row_metadata.size()), {0});
        }
        if (col_metadata.size() < static_cast<size_t>(pos.col + 1))
        {
            col_metadata.resize(col_metadata.size() + ((static_cast<size_t>(pos.col + 1)) - col_metadata.size()), {0});
        }

        ++row_metadata[pos.row].health_cell;
        ++col_metadata[pos.col].health_cell;
    }
}


CellInterface* Sheet::GetCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("");
    }

    auto cell_iter = sheet_2d_.find(pos);

    if (cell_iter == sheet_2d_.end())
    {
        if (pos < GetMaxPosition())
        {
            sheet_2d_.insert({pos, std::make_unique<Cell>(*this)});
            cell_iter = sheet_2d_.find(pos);
        }
        else
        {
            return nullptr;
        }
    }

    return cell_iter->second.get();
}

const CellInterface* Sheet::GetCell(Position pos) const
{
    return const_cast<Sheet*>(this)->GetCell(pos);
}

void Sheet::ClearCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("");
    }

    auto cell_iter = sheet_2d_.find(pos);
    if (cell_iter == sheet_2d_.end())
    {
        return;
    }
    else
    {
        sheet_2d_.at(pos)->Clear();
    }

    --row_metadata[pos.row].health_cell;
    --col_metadata[pos.col].health_cell;

    if (GetMaxPosition() == pos)
    {
        sheet_2d_.erase(cell_iter);
        row_metadata.resize(row_metadata.size() - CountReverseEmptyRows(pos.row));
        col_metadata.resize(col_metadata.size() - CountReverseEmptyCols(pos.col));
    }
}

size_t Sheet::CountReverseEmptyRows(int stdrt_row)
{
    size_t out = 0;

    if (stdrt_row != GetMaxPosition().row)
    {
        return out;
    }

    for (int row = stdrt_row; row >= 0; --row)
    {
        if (!row_metadata[row].health_cell)
        {
            ++out;
        }
        else
        {
            break;
        }
    }

    return out;
}

size_t Sheet::CountReverseEmptyCols(int stdrt_col)
{
    size_t out = 0;

    if (stdrt_col != GetMaxPosition().col)
    {
        return out;
    }

    for (int col = stdrt_col; col >= 0; --col)
    {
        if (!col_metadata[col].health_cell)
        {
            ++out;
        }
        else
        {
            break;
        }
    }

    return out;
}

Size Sheet::GetPrintableSize() const
{
    return {static_cast<int>(row_metadata.size()), static_cast<int>(col_metadata.size())};
}

Position Sheet::GetMaxPosition() const
{
    return {static_cast<int>(row_metadata.size()) - 1, static_cast<int>(col_metadata.size()) - 1};
}

void Sheet::PrintValues(std::ostream& output) const
{

    using namespace std::literals;

    for (int row = 0; row < GetPrintableSize().rows; ++row)
    {
        for (int col = 0; col < GetPrintableSize().cols; ++col)
        {
            if (col != 0)
            {
                output << '\t';
            }
            if (auto cel_iter = sheet_2d_.find(Position{row, col}); cel_iter == sheet_2d_.end())
            {
                output << "";
            }
            else
            {
                const auto& val = cel_iter->second->GetValue();
                std::visit([&output](const auto& val) {output << val;}, val);
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const 
{
    using namespace std::literals;

    for (int row = 0; row < GetPrintableSize().rows; ++row)
    {
        for (int col = 0; col < GetPrintableSize().cols; ++col)
        {
            if (col != 0)
            {
                output << '\t';
            }

            if (auto cel_iter = sheet_2d_.find(Position{row, col}); cel_iter == sheet_2d_.end())
            {
                output << "";
            }
            else
            {
                output << cel_iter->second->GetText();
            }
            
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

