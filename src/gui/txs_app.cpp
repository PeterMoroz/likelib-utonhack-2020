#include <wx/wx.h>

#include "mainframe.h"
#include "update_thread.h"

class TxsExplorer : public wxApp
{
public:
  bool OnInit() override;
};

IMPLEMENT_APP(TxsExplorer)

bool TxsExplorer::OnInit()
{
  MainFrame* frame = new MainFrame(wxT("Transactions explorer."));
  frame->Show(true);

  UpdateThread* thread = new UpdateThread(frame);
  wxThreadError err = thread->Create();
  if (err != wxTHREAD_NO_ERROR)
  {
    wxMessageBox(wxT("Could not create udpate thread."));
    return false;
  }

  err = thread->Run();
  if (err != wxTHREAD_NO_ERROR)
  {
    wxMessageBox(wxT("Could not run update thread."));
    return false;
  }
  
  return true;
}

