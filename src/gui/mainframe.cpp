#include "mainframe.h"

#include <wx/textfile.h>
#include <wx/colour.h>

#include "client/config.hpp"


#include "blocks_table.h"
#include "txs_table.h"

#include <cassert>
#include <sstream>
#include <inttypes.h>

constexpr int ENDPOINTS_CB = 101;
constexpr int PROTOCOLS_CB = 102;

constexpr int FILE_QUIT = wxID_EXIT;
constexpr int HELP_ABOUT = wxID_ABOUT;

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(FILE_QUIT, MainFrame::OnQuit)
  EVT_MENU(HELP_ABOUT, MainFrame::OnAbout)
  EVT_COMBOBOX(ENDPOINTS_CB, MainFrame::OnNodeEndpointChanged)
  EVT_COMBOBOX(PROTOCOLS_CB, MainFrame::OnProtocolChanged)
  EVT_GRID_RANGE_SELECT(MainFrame::OnGridRangeSelect)
END_EVENT_TABLE()


MainFrame::MainFrame(const wxString& title)
  : wxFrame(NULL, wxID_ANY, title)
{
  loadEndpointsFromFile(wxT("nodes.txt"));
  // SetIcon(wxICON(sample));
  wxMenu* fileMenu = new wxMenu();
  wxMenu* helpMenu = new wxMenu();
  helpMenu->Append(HELP_ABOUT, wxT("&About...\tF1"), wxT("Show about dialog"));
  fileMenu->Append(FILE_QUIT, wxT("E&xit\tAlt-X"), wxT("Quit the program"));
  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(fileMenu, wxT("&File"));
  menuBar->Append(helpMenu, wxT("&Help"));
  SetMenuBar(menuBar);
  CreateStatusBar(3);

  _book = new wxBookCtrl(this, wxID_ANY);
  wxPanel* panel = new wxPanel(_book);
  wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(vSizer);
  wxBoxSizer* hSizer1 = new wxBoxSizer(wxHORIZONTAL);

  wxString protocols[] = 
  {
    wxT("HTTP"),
    wxT("GRPC")
  };

  wxComboBox* endpointsCB = new wxComboBox(panel, ENDPOINTS_CB, wxEmptyString, 
                                          wxDefaultPosition, wxSize(100, -1),
                                          _nodesEndpoints, wxCB_DROPDOWN | wxCB_READONLY);

  hSizer1->Add(endpointsCB, 1, wxEXPAND | wxLEFT, 8);
  wxComboBox* protocolsCB = new wxComboBox(panel, PROTOCOLS_CB, wxEmptyString, 
                                          wxDefaultPosition, wxSize(100, -1),
                                          2, protocols, wxCB_DROPDOWN | wxCB_READONLY);

  hSizer1->Add(protocolsCB, 1);
  vSizer->Add(hSizer1, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  vSizer->Add(-1, 10);

  wxBoxSizer* hSizer2 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* textBlockNumber = new wxStaticText(panel, wxID_ANY, wxT("Top block number:"));
  hSizer2->Add(textBlockNumber, 0, wxRIGHT, 8);
  _textBlockNumber = new wxStaticText(panel, wxID_ANY, wxT("<no number>"));
  hSizer2->Add(_textBlockNumber, 1, wxRIGHT);
  vSizer->Add(hSizer2, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  vSizer->Add(-1, 10);

  wxBoxSizer* hSizer3 = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* textBlockHash = new wxStaticText(panel, wxID_ANY, wxT("Top block hash:"));
  hSizer3->Add(textBlockHash, 0, wxRIGHT, 8);
  _textBlockHash = new wxStaticText(panel, wxID_ANY, wxT("<no hash>"));
  hSizer3->Add(_textBlockHash, 1, wxRIGHT);
  vSizer->Add(hSizer3, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
  vSizer->Add(-1, 10);
  panel->SetSizer(vSizer);
  _book->AddPage(panel, wxT("Nodes"), false);

  panel = new wxPanel(_book);
  vSizer = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(vSizer);
  _blocksGrid = new wxGrid(panel, wxID_ANY, wxPoint(0, 0), wxSize(400, 300));
  BlocksTable* blocksTable = new BlocksTable();
  _blocksGrid->SetTable(blocksTable);
  _blocksGrid->EnableEditing(false);
  _blocksGrid->HideRowLabels();
  _blocksGrid->SetSelectionMode(wxGrid::wxGridSelectRows);

  for (int i = 0; i < blocksTable->GetNumberCols(); i++)
  {
    _blocksGrid->SetColLabelValue(i, blocksTable->GetColLabelValue(i));
  }

  vSizer->Add(_blocksGrid, 1, wxEXPAND | wxALL, 5);
  _book->AddPage(panel, wxT("Blocks"), false);


  panel = new wxPanel(_book);
  vSizer = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(vSizer);
  _txsGrid = new wxGrid(panel, wxID_ANY, wxPoint(0, 0), wxSize(400, 300));
  TxsTable* txsTable = new TxsTable();
  _txsGrid->SetTable(txsTable);
  _txsGrid->EnableEditing(false);
  _txsGrid->HideRowLabels();

  for (int i = 0; i < txsTable->GetNumberCols(); i++)
  {
    _txsGrid->SetColLabelValue(i, txsTable->GetColLabelValue(i));
  }

  vSizer->Add(_txsGrid, 1, wxEXPAND | wxALL, 5);
  _book->AddPage(panel, wxT("Transactions"), false);

  panel = new wxPanel(_book);
  vSizer = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(vSizer);
  _textLog = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxPoint(0, 250), wxSize(100, 50), wxTE_MULTILINE);
  vSizer->Add(_textLog, 1, wxEXPAND | wxALL, 5);
  _book->AddPage(panel, wxT("Trace"), false);

  endpointsCB->SetSelection(0);
  protocolsCB->SetSelection(0);

  _endpoint = endpointsCB->GetStringSelection();
  _protocol = protocolsCB->GetStringSelection();

  connectToNode();
}

void MainFrame::OnQuit(wxCommandEvent& event)
{
  Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
  wxString msg;
  msg.Printf(wxT("Transactions Explorer\nVersion %u.%u\nRPC API Version %u\n"), 1, 0, config::API_VERSION);
  wxMessageBox(msg, wxT("About"), wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnNodeEndpointChanged(wxCommandEvent& event)
{
  *_textLog << wxT("Endpoint changed: ") << event.GetString() << wxT("\n");
  _endpoint = event.GetString();
  connectToNode();
}

void MainFrame::OnProtocolChanged(wxCommandEvent& event)
{
  *_textLog << wxT("Protocol changed: ") << event.GetString() << wxT("\n");
  _protocol = event.GetString();
  connectToNode();
}

void MainFrame::OnGridRangeSelect(wxGridRangeSelectEvent& event)
{
  wxGrid* grid = static_cast<wxGrid*>(event.GetEventObject());
  if (grid == _blocksGrid)
  {
    if (event.Selecting())
    {
      int row = grid->GetGridCursorRow();
      *_textLog << wxT("Selected row: ") << row << wxT("\n");
    }
  }
}

void MainFrame::loadEndpointsFromFile(const wxString& filename)
{
  _nodesEndpoints.Clear();
  wxTextFile txtFile(filename);
  txtFile.Open();

  wxString line = txtFile.GetFirstLine();
  for (; !txtFile.Eof(); line = txtFile.GetNextLine())
  {
    _nodesEndpoints.Add(line);
  }

  txtFile.Close();
}

void MainFrame::connectToNode()
{
  SetStatusText(_endpoint, 0);
  SetStatusText(_protocol, 1);
  SetStatusText(wxT("Not connected"), 2);

  *_textLog << wxT("Trying to connect to RPC server at ")
      << _endpoint << wxT(", using protocol ") << _protocol << wxT("\n");

  if (_protocol.Cmp(wxT("HTTP")) == 0)
  {
    _rpcClient = rpc::createRpcClient(rpc::ClientMode::HTTP, _endpoint.ToStdString());
  }
  else if (_protocol.Cmp(wxT("GRPC")) == 0)
  {
    _rpcClient = rpc::createRpcClient(rpc::ClientMode::GRPC, _endpoint.ToStdString());
  }
  else
  {
    *_textLog << wxT("Unsupported RPC protocol.\n");
    return;
  }

  assert(_rpcClient);

  try
  {
    auto nodeInfo = _rpcClient->getNodeInfo();

    if (config::API_VERSION == nodeInfo.api_version)
    {
      SetStatusText(wxT("Available"), 2);
      _textBlockNumber->SetLabel(wxString::Format(wxT("%" PRIu64 ""), nodeInfo.top_block_number));
      _textBlockHash->SetLabel(wxString(nodeInfo.top_block_hash.toHex().c_str(), wxConvUTF8));
      _dataModel.setTopBlockNumber(nodeInfo.top_block_number);
      loadDataModelFromNode();
    }
    else
    {
      wxString msg;
      msg.Printf(wxT("The node %s is not available by the %s protocol."), _endpoint, _protocol);
      wxMessageBox(msg, wxT("Transactions Explorer"), wxOK | wxICON_ERROR, this);
      SetStatusText(wxT("Not Available."), 2);
      _textBlockNumber->SetLabel(wxT("<no number>"));
      _textBlockHash->SetLabel(wxT("<no hash>"));
      _dataModel.setTopBlockNumber(0);
    }
  }
  catch (const std::exception& ex)
  {
    wxString msgText;
    wxString errText(ex.what(), wxConvUTF8);
    msgText.Printf(wxT("Could not connect to node %s, using the %s protocol.\nAn error: %s"),
              _endpoint, _protocol, errText);
    wxMessageBox(msgText, wxT("Transactions Explorer"), wxOK | wxICON_ERROR, this);
    SetStatusText(wxT("Not Available"), 2);
    return;
  }

}

void MainFrame::loadDataModelFromNode()
{
  _dataModel.Clear();

  std::uint64_t topBlockNumber = _dataModel.getTopBlockNumber();
  for (std::uint64_t blockNumber = 0; blockNumber < topBlockNumber; blockNumber++)
  {
    // wxTextCtrl::operator<< has no overloading for uint64_t, so the intermediate string is used
    wxString tmp = wxString::Format(wxT("Loading block #%" PRIu64 "\n"), blockNumber);
    *_textLog << tmp;

    try
    {
      lk::ImmutableBlock block = _rpcClient->getBlock(blockNumber);

      // Don't know what does it mean and why the such block treated as not valid.
      // Just copied from client/actions.cpp ActionGetBlock::execute() .
      if (block.getTimestamp().getSeconds() == 0 && block.getDepth() == lk::BlockDepth(-1))
      {
        break;
      }
      _dataModel.AppendBlock(block);

      for (const auto& tx : block.getTransactions())
      {
        _dataModel.AppendTransaction(tx);
      }
    }
    catch (const std::exception& ex)
    {
      std::cerr << "An exception when loading the data model from the node: " << ex.what() << std::endl;
    }
  }

  updateBlocksTable();
  updateTransactionsTable();
}

void MainFrame::updateBlocksTable()
{
  wxGridTableBase* gridTable = _blocksGrid->GetTable();
  gridTable->Clear();

  std::size_t numberBlocks = _dataModel.GetNumberBlocks();
  if (!gridTable->AppendRows(numberBlocks))
  {
    wxString tmp = wxString::Format(
      wxT("An error when updateBlockTable. Could not append %" PRIu64 "blocks\n"), 
      numberBlocks);
    *_textLog << tmp;
    return;
  }

  for (std::size_t i = 0; i < numberBlocks; i++)
  {
    const lk::ImmutableBlock& block = _dataModel.GetBlock(i);
    wxString text = wxString::Format(wxT("%" PRIu64 ""), i);
    gridTable->SetValue(i, 0, text);
    wxString hash(block.getHash().toHex().c_str(), wxConvUTF8);
    gridTable->SetValue(i, 1, hash);
    text = wxString::Format(wxT("%" PRIu64 ""), block.getDepth());
    gridTable->SetValue(i, 2, text);
    text = wxString::Format(wxT("%u"), block.getTimestamp().getSeconds());
    gridTable->SetValue(i, 3, text);
    wxString coinbase(block.getCoinbase().toString().c_str(), wxConvUTF8);
    gridTable->SetValue(i, 4, coinbase);
    text = wxString::Format(wxT("%" PRIu64 ""), block.getNonce());
    gridTable->SetValue(i, 5, text);
  }
  
   _blocksGrid->AutoSize();
}

void MainFrame::updateTransactionsTable()
{
  wxGridTableBase* gridTable = _txsGrid->GetTable();
  gridTable->Clear();

  std::size_t numberTransactions = _dataModel.GetNumberTransactions();
  if (!gridTable->AppendRows(numberTransactions))
  {
    wxString tmp = wxString::Format(
      wxT("An error when updateTransactionTable. Could not append %" PRIu64 "transactions\n"), 
      numberTransactions);
    *_textLog << tmp;
    return;
  }

  for (std::size_t i = 0; i < numberTransactions; i++)
  {
    const lk::Transaction& tx = _dataModel.GetTransaction(i);
    wxString text = wxString::Format(wxT("%" PRIu64 ""), i);
    gridTable->SetValue(i, 0, text);

    if (tx.getTo() == lk::Address::null()) 
    {
        gridTable->SetValue(i, 1, wxT("contract creation"));
    }
    else if (tx.getData().isEmpty()) 
    {
        gridTable->SetValue(i, 1, wxT("transfer"));
    }
    else 
    {
        gridTable->SetValue(i, 1, wxT("contract call"));
    }

    gridTable->SetValue(i, 2, wxString(tx.getFrom().toString().c_str(), wxConvUTF8));
    gridTable->SetValue(i, 3, wxString(tx.getTo().toString().c_str(), wxConvUTF8));

    std::ostringstream oss;
    oss << tx.getAmount();
    gridTable->SetValue(i, 4, wxString(oss.str().c_str(), wxConvUTF8));

    text = wxString::Format(wxT("%" PRIu64 ""), tx.getFee());
    gridTable->SetValue(i, 5, text);
    text = wxString::Format(wxT("%u"), tx.getTimestamp().getSeconds());
    gridTable->SetValue(i, 6, text);
  
    gridTable->SetValue(i, 7, tx.getData().isEmpty() ? wxT("<empty>") : wxString(base::toHex(tx.getData()).c_str(), wxConvUTF8));
    gridTable->SetValue(i, 8, tx.checkSign() ? wxT("verified") : wxT("bad signature"));
  }
  
  _txsGrid->AutoSize();
}

