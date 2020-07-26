#include <wx/wx.h>
#include <wx/wxprec.h>
#include <wx/textctrl.h>
#include <wx/bookctrl.h>
#include <wx/grid.h>

#include "data_model.h"

#include <memory>
#include <thread>

class MainFrame : public wxFrame
{
public:
  static const int UPDATE_TABLES_ID = 100000;
  static const int SET_BLOCK_HASH_ID = 100001;
  static const int SET_BLOCK_NUMBER_ID = 100002;
  static const int CONNECTION_STATUS_CHANGED_ID = 100003;

public:
  explicit MainFrame(const wxString& title);

  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);

  void OnNodeEndpointChanged(wxCommandEvent& event);
  void OnProtocolChanged(wxCommandEvent& event);

  void OnUpdateTables(wxCommandEvent& event);
  void OnSetBlockHash(wxCommandEvent& event);
  void OnSetBlockNumber(wxCommandEvent& event);
  void OnConnectionStatusChanged(wxCommandEvent& event);

  wxString getEndpoint();
  wxString getProtocol();

  void setStatusText(int pos, const wxString& text);

  DataModel& getDataModel() { return _dataModel; }

private:
  void loadEndpointsFromFile(const wxString& filename);
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
  wxMutex _endpointMutex;
  wxMutex _protocolMutex;

  wxArrayString _nodesEndpoints;
  DataModel _dataModel;
 
private:
  DECLARE_EVENT_TABLE()
};

