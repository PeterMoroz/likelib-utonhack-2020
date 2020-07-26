#include "update_thread.h"
#include "mainframe.h"
#include "data_model.h"

#include "client/config.hpp"

#include <cassert>
#include <inttypes.h>

UpdateThread::UpdateThread(wxFrame* frame)
  : _frame(dynamic_cast<MainFrame*>(frame))
{
  assert(_frame != nullptr);
}

wxThread::ExitCode UpdateThread::Entry()
{
  _running = true;
  while (_running)
  {

    bool needCreateClient = false;
    wxString endpoint = _frame->getEndpoint();
    wxString protocol = _frame->getProtocol();

    {
      if (endpoint != _endpoint || protocol != _protocol || !_rpcClient)
      {
        needCreateClient = true;
      }
    }

    if (needCreateClient)
    {
      if (protocol.Cmp(wxT("HTTP")) == 0)
      {
        _rpcClient = rpc::createRpcClient(rpc::ClientMode::HTTP, endpoint.ToStdString());        
        _endpoint = endpoint;
        _protocol = protocol;
      }
      else if (_protocol.Cmp(wxT("GRPC")) == 0)
      {
        _rpcClient = rpc::createRpcClient(rpc::ClientMode::GRPC, endpoint.ToStdString());
        _endpoint = endpoint;
        _protocol = protocol;
      }
      else
      {
        _endpoint.Clear();
        _protocol.Clear();
        continue;
      }
    }

    assert(_rpcClient);
    DataModel& dataModel = _frame->getDataModel();

    try
    {
      auto nodeInfo = _rpcClient->getNodeInfo();

      if (config::API_VERSION == nodeInfo.api_version)
      {
        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::SET_BLOCK_HASH_ID);
          event.SetString(wxString(nodeInfo.top_block_hash.toHex().c_str(), wxConvUTF8));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::SET_BLOCK_NUMBER_ID);
          event.SetString(wxString::Format(wxT("%" PRIu64 ""), nodeInfo.top_block_number));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::CONNECTION_STATUS_CHANGED_ID);
          event.SetString(wxString::Format(wxT("%s|%s|%s"), _endpoint, _protocol, wxT("Available")));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        if (dataModel.getTopBlockNumber() < nodeInfo.top_block_number)
        {
          dataModel.setTopBlockNumber(nodeInfo.top_block_number);
          loadDataModelFromNode();
        }
      }
      else
      {

        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::SET_BLOCK_HASH_ID);
          event.SetString(wxT("<no hash>"));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::SET_BLOCK_NUMBER_ID);
          event.SetString(wxT("<no number>"));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::CONNECTION_STATUS_CHANGED_ID);
          event.SetString(wxString::Format(wxT("%s|%s|%s"), _endpoint, _protocol, wxT("Not Available")));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        dataModel.setTopBlockNumber(0);
      }
    }
    catch (const std::exception& ex)
    {
        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::SET_BLOCK_HASH_ID);
          event.SetString(wxT("<no hash>"));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::SET_BLOCK_NUMBER_ID);
          event.SetString(wxT("<no number>"));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

        {
          wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::CONNECTION_STATUS_CHANGED_ID);
          event.SetString(wxString::Format(wxT("%s|%s|%s"), _endpoint, _protocol, wxT("Not Available")));
          // _frame->GetEventHandler()->QueueEvent(&event);
          _frame->GetEventHandler()->AddPendingEvent(event);
        }

      dataModel.setTopBlockNumber(0);
    }

    Sleep(1000);
  }
  return 0;
}

void UpdateThread::loadDataModelFromNode()
{
  DataModel& dataModel = _frame->getDataModel();
  dataModel.Clear();

  std::uint64_t topBlockNumber = dataModel.getTopBlockNumber();
  for (std::uint64_t blockNumber = 0; blockNumber < topBlockNumber; blockNumber++)
  {
    try
    {
      lk::ImmutableBlock block = _rpcClient->getBlock(blockNumber);

      // Don't know what does it mean and why the such block treated as not valid.
      // Just copied from client/actions.cpp ActionGetBlock::execute() .
      if (block.getTimestamp().getSeconds() == 0 && block.getDepth() == lk::BlockDepth(-1))
      {
        break;
      }
      dataModel.AppendBlock(block);

      for (const auto& tx : block.getTransactions())
      {
        dataModel.AppendTransaction(tx);
      }
    }
    catch (const std::exception& ex)
    {
      std::cerr << "An exception when loading the data model from the node: " << ex.what() << std::endl;
      return;
    }
  }

  wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, MainFrame::UPDATE_TABLES_ID);
  _frame->GetEventHandler()->AddPendingEvent(event);
  // _frame->GetEventHandler()->QueueEvent(&event);
}

