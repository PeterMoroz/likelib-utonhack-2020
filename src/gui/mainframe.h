#include <wx/wx.h>
#include <wx/wxprec.h>
#include <wx/textctrl.h>
#include <wx/bookctrl.h>
#include <wx/grid.h>

#include "rpc/rpc.hpp"
#include "data_model.h"

#include <memory>


class MainFrame : public wxFrame
{
public:
  explicit MainFrame(const wxString& title);

  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);

  void OnNodeEndpointChanged(wxCommandEvent& event);
  void OnProtocolChanged(wxCommandEvent& event);
  void OnGridRangeSelect(wxGridRangeSelectEvent& event);

private:
  void loadEndpointsFromFile(const wxString& filename);
  void connectToNode();
  void loadDataModelFromNode();

  void updateBlocksTable();
  void updateTransactionsTable();
  
private:
  wxBookCtrl* _book = nullptr;
  wxTextCtrl* _textLog = nullptr;
  wxGrid* _blocksGrid = nullptr;
  wxGrid* _txsGrid = nullptr;

  wxStaticText* _textBlockNumber = nullptr;
  wxStaticText* _textBlockHash = nullptr;

  wxString _endpoint;
  wxString _protocol;

  wxArrayString _nodesEndpoints;
  std::unique_ptr<rpc::BaseRpc> _rpcClient;
  DataModel _dataModel;

private:
  DECLARE_EVENT_TABLE()
};

