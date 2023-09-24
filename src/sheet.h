#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>

class Sheet : public SheetInterface {
public:
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    CellInterface* GetCell(Position pos) override;
    const CellInterface* GetCell(Position pos) const override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;
    Position GetMaxPosition() const;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:

    size_t CountReverseEmptyRows(int stdrt_row);
    size_t CountReverseEmptyCols(int stdrt_col);

    struct RowMetadata
    {
        uint16_t health_cell = 0;
    };

    struct ColMetadata
    {
        uint16_t health_cell = 0;
    };

    std::vector<RowMetadata> row_metadata;
    std::vector<ColMetadata> col_metadata;
    std::unordered_map<Position, std::unique_ptr<Cell>, Position::PositionHasher> sheet_2d_;
};