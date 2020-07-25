#pragma once

#include "core/block.hpp"
#include "core/transaction.hpp"

#include <vector>

class DataModel final
{
public:
  DataModel() = default;
  ~DataModel() = default;

  DataModel(const DataModel&) = delete;
  DataModel& operator=(const DataModel&) = delete;

  void Clear()
  {
    _blocks.clear();
    _transactions.clear();
  }

  std::size_t GetNumberBlocks() const
  {
    return _blocks.size();
  }

  std::size_t GetNumberTransactions() const
  {
    return _transactions.size();
  }

  void AppendBlock(const lk::ImmutableBlock& block)
  {
    _blocks.push_back(block);
  }

  void AppendTransaction(const lk::Transaction& transaction)
  {
    _transactions.push_back(transaction);
  }

  const lk::ImmutableBlock& GetBlock(std::size_t idx) const
  {
    return _blocks[idx];
  }

  const lk::Transaction& GetTransaction(std::size_t idx) const
  {
    return _transactions[idx];
  }
  
private:
  std::vector<lk::ImmutableBlock> _blocks;
  std::vector<lk::Transaction> _transactions;
};

