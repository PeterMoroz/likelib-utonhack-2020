#include "blocks_table.h"

#include <cassert>
#include <inttypes.h>

const std::array<wxString, BlocksTable::NUM_OF_COLUMNS> BlocksTable::COLUMNS_LABELS{ 
  wxT("Number"), wxT("Hash"), wxT("Depth"), wxT("Timestamp"), wxT("Coinbase"), wxT("Nonce") };

BlocksTable::~BlocksTable()
{
}

wxString BlocksTable::GetValue(int row, int col)
{
  wxString result;
  if (row < GetNumberRows())
  {
    switch (col)
    {
    case 0:
      result = wxString::Format(wxT("%" PRIu64 ""), _rows[row]._number);
      break;
    case 1:
      result = _rows[row]._hash;
      break;
    case 2:
      result = wxString::Format(wxT("%" PRIu64 ""), _rows[row]._depth);
      break;
    case 3:
      result = wxString::Format(wxT("%u"), _rows[row]._timestamp);
      break;
    case 4:
      result = _rows[row]._coinbase;
      break;
    case 5:
      result = wxString::Format(wxT("%" PRIu64 ""), _rows[row]._nonce);
      break;
    default:
      assert(false);
    }
  }
  return result;
}

void BlocksTable::SetValue(int row, int col, const wxString& val)
{
  unsigned long number = 0;
  unsigned long depth = 0;
  unsigned long nonce = 0;
  unsigned long timestamp = 0;

  static_assert(sizeof(unsigned long) == sizeof(std::uint64_t), "Assume that unsigned long type is 64-bit length.");

  if (row < GetNumberRows())
  {
    switch (col)
    {
    case 0:
      assert(val.ToULong(&number));
      _rows[row]._number = number;
      break;
    case 1:
      _rows[row]._hash = val;
      break;
    case 2:
      assert(val.ToULong(&depth));
      _rows[row]._depth = depth;
      break;
    case 3:
      assert(val.ToULong(&timestamp));
      _rows[row]._timestamp = timestamp;
      break;
    case 4:
      _rows[row]._coinbase = val;
      break;
    case 5:
      assert(val.ToULong(&nonce));
      _rows[row]._nonce = nonce;
      break;
    default:
      assert(false);
    }
  }
}

wxString BlocksTable::GetColLabelValue(int col)
{
  return col < static_cast<int>(COLUMNS_LABELS.size()) ? COLUMNS_LABELS[col] : "";
}

void BlocksTable::Clear()
{
  size_t numRows = _rows.size();
  _rows.clear();
  
  if (GetView())
  {
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, 0, numRows);
    GetView()->ProcessTableMessage(msg);
  }
}

bool BlocksTable::InsertRows(size_t pos /*= 0*/, size_t numRows /*= 1*/)
{
  if (numRows == 0 || pos >= _rows.size())
  {
    return false;
  }

  try 
  {
    std::vector<BlockInfo> tmp(numRows);
    std::vector<BlockInfo> rows(_rows);
    rows.insert(rows.begin() + pos, tmp.begin(), tmp.end());
    _rows.swap(rows);
  }
  catch (const std::exception& ex)
  {
    return false;
  }

  if (GetView())
  {
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_INSERTED, pos, numRows);
    GetView()->ProcessTableMessage(msg);
  }
  
  return true;
}

bool BlocksTable::AppendRows(size_t numRows /*= 1*/)
{
  if (numRows == 0)
  {
    return false;
  }

  try 
  {
    std::vector<BlockInfo> rows(_rows);
    for (size_t i = 0; i < numRows; i++)
    {
      rows.push_back(BlockInfo());
    }

    _rows.swap(rows);
  }
  catch (const std::exception& ex)
  {
    return false;
  }

  if (GetView())
  {
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
    GetView()->ProcessTableMessage(msg);
  }
  
  return true;

}

bool BlocksTable::DeleteRows(size_t pos /*= 0*/, size_t numRows /*= 1*/)
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

