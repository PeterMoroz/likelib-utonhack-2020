#pragma once

#include <wx/wx.h>

#include "rpc/rpc.hpp"

class MainFrame;

class UpdateThread : public wxThread
{

public:
  explicit UpdateThread(wxFrame* frame);
  
  ExitCode Entry() override;

  void stop() { _running = false; }

private:
  void loadDataModelFromNode();

private:
  MainFrame* _frame = nullptr;
  bool _running = false;

  std::unique_ptr<rpc::BaseRpc> _rpcClient;
  wxString _endpoint;
  wxString _protocol;
};

