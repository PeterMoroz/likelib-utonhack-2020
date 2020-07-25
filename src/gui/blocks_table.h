#pragma once

#include <wx/grid.h>
#include <wx/string.h>

#include <cstdint>
#include <array>
#include <vector>


class BlocksTable : public wxGridTableBase
{
  struct BlockInfo
  {
    std::uint64_t _number = 0;
    wxString _hash;
    std::uint64_t _depth = 0;
    std::uint32_t _timestamp = 0;
    wxString _coinbase;
    std::uint64_t _nonce;
  };

  static const std::size_t NUM_OF_COLUMNS = 6;
  static const std::array<wxString, NUM_OF_COLUMNS> COLUMNS_LABELS;

public:
  BlocksTable() = default;
  ~BlocksTable() override;

  int GetNumberRows() override
  {
    return _rows.size();
  }

  int GetNumberCols() override
  {
    return NUM_OF_COLUMNS;
  }

  wxString GetValue(int row, int col) override;
  void SetValue(int row, int col, const wxString& val) override;

  wxString GetColLabelValue(int col) override;

  void Clear() override;
  bool InsertRows(size_t pos = 0, size_t numRows = 1) override;
  bool AppendRows(size_t numRows = 1) override;
  bool DeleteRows(size_t pos = 0, size_t numRows = 1) override;

private:
  std::vector<BlockInfo> _rows;
};

