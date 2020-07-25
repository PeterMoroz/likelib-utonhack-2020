#pragma once

#include <wx/grid.h>
#include <wx/string.h>

#include <array>
#include <vector>


class TxsTable : public wxGridTableBase
{
  struct TxInfo
  {
    unsigned _number = 0;
    wxString _type;
    wxString _from;
    wxString _to;
    wxString _value;
    std::uint64_t _fee = 0;
    std::uint32_t _timestamp = 0;
    wxString _data;
    bool _signature = false;
  };

  static const std::size_t NUM_OF_COLUMNS = 9;
  static const std::array<wxString, NUM_OF_COLUMNS> COLUMNS_LABELS;

public:
  TxsTable() = default;
  ~TxsTable() override;

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
  std::vector<TxInfo> _rows;
};

