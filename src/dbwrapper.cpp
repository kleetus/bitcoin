// Copyright (c) 2012-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbwrapper.h"

#include "util.h"
#include "random.h"

#include <boost/filesystem.hpp>

#include <stdint.h>

CDBWrapper::CDBWrapper(const boost::filesystem::path& path, size_t nCacheSize, bool fMemory, bool fWipe, bool obfuscate, bool compression, int maxOpenFiles, int mapSize, int maxReaders)
{
  TryCreateDirectory(path);
  LogPrintf("Opening LMDB in %s\n", path.string());

  dbwrapper_private::HandleError(mdb_env_create(&env)); 

  //set options
  dbwrapper_private::HandleError(mdb_env_set_mapsize(env, mapSize)); 
  dbwrapper_private::HandleError(mdb_env_set_maxreaders(env, maxReaders)); 

  dbwrapper_private::HandleError(mdb_env_open(env, path.c_str(), 0, 0644)); 
  dbwrapper_private::HandleError(mdb_txn_begin(env, NULL, 0, &ptxn)); 
  dbwrapper_private::HandleError(mdb_dbi_open(ptxn, NULL, MDB_CREATE, &dbi)); 

  LogPrintf("Opened LMDB successfully");
}

CDBWrapper::~CDBWrapper()
{
  if (dbi) {
    mdb_dbi_close(env, dbi);
  }
  if (txn) {
    mdb_txn_commit(txn);
  }
  if (env) {
    mdb_env_close(env);
  }
  LogPrintf("Closed LMDB successfully");
}

bool CDBWrapper::WriteBatch(CDBBatch& batch, bool fSync)
{

}

  template <typename K, typename V>
bool CDBWrapper::Write(const K& key, const V& value, bool fSync)
{
  MDB_txn *txn;
  MDB_val key, data;
  key.mv_size = sizeof(skey);
  key.mv_data = (char *)skey;
  data.mv_size = sizeof(svalue);
  data.mv_data = (char *)svalue;
  dbwrapper_private::HandleError(mdb_txn_begin(this->env, ptxn, 0, &txn)); 
  dbwrapper_private::HandleError(mdb_put(txn, dbi, &key, &data, 0)); 
  dbwrapper_private::HandleError(mdb_txn_commit(txn)); 
}

bool CDBWrapper::IsEmpty()
{
  bool ret = false;
  MDB_cursor *cur;
  MDB_val key, data;
  char val[32];
  dbwrapper_private::HandleError(mdb_cursor_open(ptxn, dbi, &cur)); 
  printf("address of: %p\n", &ptxn);  
  key.mv_size = sizeof(int);
  key.mv_data = val;
  data.mv_size = sizeof(val);
  data.mv_data = val;
  int res = mdb_cursor_get(cur, &key, &data, MDB_FIRST); 
  if (res == MDB_NOTFOUND) {
    ret = true;
  }
  mdb_cursor_close(cur);
  return ret;
}

CDBIterator::~CDBIterator() {}
bool CDBIterator::Valid() {}
void CDBIterator::SeekToFirst() {}
void CDBIterator::Next() {}

namespace dbwrapper_private {

void HandleError(int error)
{
  if(error != 0) {
    printf("error num was: %d\n", error);
    throw dbwrapper_error("Database could not accessed");
  } 
}

};
