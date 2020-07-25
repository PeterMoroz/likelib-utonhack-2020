#include "txs_table.h"

#include <cassert>
#include <iostream>
#include <inttypes.h>

const std::array<wxString, TxsTable::NUM_OF_COLUMNS> TxsTable::COLUMNS_LABELS{
   wxT("Number"), wxT("Type"), wxT("From"), wxT("To"), wxT("Value"), wxT("Fee"), wxT("Timestamp"), wxT("Data"), wxT("Signature") };

TxsTable::~TxsTable()
{
}

wxString TxsTable::GetValue(int row, int col)
{
  wxString result;
  if (row < GetNumberRows())
  {
    switch (col)
    {
    case 0:
      result = wxString::Format(wxT("%u"), _rows[row]._number);
      break;
    case 1:
      result = _rows[row]._type;
      break;
    case 2:
      result = _rows[row]._from;
      break;
    case 3:
      result = _rows[row]._to;
      break;
    case 4:
      result = _rows[row]._value;
      break;
    case 5:
      result = wxString::Format(wxT("%" PRIu64 ""), _rows[row]._fee);
      break;
    case 6:
      result = wxString::Format(_T("%u"), _rows[row]._timestamp);
      break;
    case 7:
      result = _rows[row]._data.IsEmpty() ? wxT("<empty>") : _rows[row]._data;
      break;
    case 8:
      result = _rows[row]._signature ? wxT("verified") : wxT("bad signature");
      break;
    default:
      assert(false);
    }
  }
  return result;
}

void TxsTable::SetValue(int row, int col, const wxString& val)
{
  unsigned long number = 0;
  unsigned long timestamp = 0;
  unsigned long fee = 0;

  static_assert(sizeof(unsigned long) == sizeof(std::uint64_t), "Assume that unsigned long type is 64-bit length.");

  if (row < GetNumberRows())
  {
    switch (col)
    {
    case 0:
      if (!val.ToULong(&number))
      {
        std::cerr << "Could not convert " << val.ToStdString() << " to the ulong number\n";
        return;
      }
      _rows[row]._number = number;
      break;
    case 1:
      _rows[row]._type = val;
      break;
    case 2:
      _rows[row]._from = val;
      break;
    case 3:
      _rows[row]._to = val;
      break;
    case 4:
      _rows[row]._value = val;
      break;
    case 5:
      if (!val.ToULong(&fee))
      {
        std::cerr << "Could not convert " << val.ToStdString() << " to the ulong number\n";
        return;
      }
      _rows[row]._fee = fee;
      break;
    case 6:
      if (!val.ToULong(&timestamp))
      {
        std::cerr << "Could not convert " << val.ToStdString() << " to the ulong number\n";
        return;
      }
      _rows[row]._timestamp = timestamp;
      break;
    case 7:
      _rows[row]._data = val;
      break;
    case 8:
      {
        if (val.Cmp(wxT("verified")) == 0)
        {
          _rows[row]._signature = true;
        }
        else if (val.Cmp(wxT("bad signature")) == 0)
        {
          _rows[row]._signature = false;
        }
        else
        {
          assert(false);
          return;
        }
      }
      break;
    default:
      assert(false);
    }
  }
}

wxString TxsTable::GetColLabelValue(int col)
{
  return col < static_cast<int>(COLUMNS_LABELS.size()) ? COLUMNS_LABELS[col] : "";
}

void TxsTable::Clear()
{
  size_t numRows = _rows.size();
  _rows.clear();
  
  if (GetView())
  {
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, 0, numRows);
    GetView()->ProcessTableMessage(msg);
  }
}

bool TxsTable::InsertRows(size_t pos /*= 0*/, size_t numRows /*= 1*/)
{
  if (numRows == 0 || pos >= _rows.size())
  {
    return false;
  }

  try 
  {
    std::vector<TxInfo> tmp(numRows);
    std::vector<TxInfo> rows(_rows);
    rows.insert(rows.begin() + pos, tmp.begin(), tmp.end());
    _rows.swap(rows);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return false;
  }

  if (GetView())
  {
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED, pos, numRows);
    GetView()->ProcessTableMessage(msg);
  }
  
  return true;
}

bool TxsTable::AppendRows(size_t numRows /*= 1*/)
{
  if (numRows == 0)
  {
    return false;
  }

  try 
  {
    std::vector<TxInfo> rows(_rows);
    for (size_t i = 0; i < numRows; i++)
    {
      rows.push_back(TxInfo());
    }

    _rows.swap(rows);
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return false;
  }

  if (GetView())
  {
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
    GetView()->ProcessTableMessage(msg);
  }
  
  return true;

}

bool TxsTable::DeleteRows(size_t pos /*= 0*/, size_t numRows /*= 1*/)
{
  if (numRows == 0)
  {
    return false;
  }

  if (numRows == 1)
  {
    _rows.erase(_rows.begin() + pos);
  }
  else
  {
    _rows.erase(_rows.begin() + pos, _rows.begin() + pos + numRows);
  }

  if (GetView())
  {
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, pos, numRows);
    GetView()->ProcessTableMessage(msg);
  }

  return true;
}

