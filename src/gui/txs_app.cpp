#include <wx/wx.h>

#include "mainframe.h"

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
  return true;
}

