﻿/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/


#include "Init.h"
#include "Memory.h"


Memory::Memory()
{
  Memory::Reset();
}


Memory::~Memory()
{
  CloseDebugFiles();
  FreeMemory();
}


void Memory::Reset()
{
  nThreads = 0;
}


void Memory::ResetThread(const unsigned thrId)
{
  if(memory[thrId] == nullptr || memory[thrId]->transTable == nullptr) return;
  memory[thrId]->transTable->ResetMemory(TT_RESET_FREE_MEMORY);
  memory[thrId]->memUsed = Memory::MemoryInUseMB(thrId);
}


void Memory::ReturnThread(const unsigned thrId)
{
  if(memory.size() >= thrId || memory[thrId] == nullptr || memory[thrId]->transTable == nullptr) return;
  memory[thrId]->transTable->ReturnAllMemory();
  memory[thrId]->memUsed = Memory::MemoryInUseMB(thrId);
  delete memory[thrId]->transTable;
  delete memory[thrId];
}


void Memory::Resize(
  const unsigned n,
  const TTmemory flag,
  const int memDefault_MB,
  const int memMaximum_MB)
{
  if (nThreads == n)
    return;

  if (nThreads > n)
  {
    // Downsize.
    for (unsigned i = n; i < nThreads; i++)
    {
      memory[i]->transTable->ReturnAllMemory();
      delete memory[i]->transTable;
      delete memory[i];
    }
    memory.resize(static_cast<unsigned>(n));
    threadSizes.resize(static_cast<unsigned>(n));
  }
  else
  {
    // Upsize.
    memory.resize(n);
    threadSizes.resize(n);
    for (unsigned i = nThreads; i < n; i++)
    {
      memory[i] = new ThreadData;
      if (flag == DDS_TT_SMALL)
      {
        memory[i]->transTable = new TransTableS;
        threadSizes[i] = "S";
      }
      else
      {
        memory[i]->transTable = new TransTableL;
        threadSizes[i] = "L";
      }

      memory[i]->transTable->SetMemoryDefault(memDefault_MB);
      memory[i]->transTable->SetMemoryMaximum(memMaximum_MB);

      memory[i]->transTable->MakeTT();
    }
  }

  nThreads = n;
}


int Memory::NumThreads() const
{
  return static_cast<int>(nThreads);
}


ThreadData * Memory::GetPtr(const unsigned thrId)
{
  if (thrId >= nThreads)
  {
    return NULL;
  }
  return memory[thrId];
}


double Memory::MemoryInUseMB(const unsigned thrId) const
{
  if(memory[thrId] == nullptr || memory[thrId]->transTable == nullptr) return -101;
  return memory[thrId]->transTable->MemoryInUse() +
    8192. * sizeof(relRanksType) / static_cast<double>(1024.);
}


void Memory::ReturnAllMemory()
{
  Memory::Resize(0, DDS_TT_SMALL, 0, 0);
}


string Memory::ThreadSize(const unsigned thrId) const
{
  return threadSizes[thrId];
}

