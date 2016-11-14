#include <boost/filesystem.hpp>
#include <lmdb/lmdb.h>
#include <cstdio>


class dbwrapper_error : public std::runtime_error
{
public:
    dbwrapper_error(const std::string& msg) : std::runtime_error(msg) {}
};

class CDBWrapper;

class CDBWrapper 
{
private:
  MDB_env *env;
  MDB_dbi dbi;
  MDB_val key, data;
  MDB_txn *ptxn;
  MDB_cursor *cursor;
public:
  CDBWrapper(const boost::filesystem::path& path, size_t nCacheSize, bool fMemory = false, bool fWipe = false, bool obfuscate = false, bool compression = false, int maxOpenFiles = 64, int mapSize = 10485760, int maxReaders = 126);
  ~CDBWrapper();
  bool Write(const char *key, const char *value, bool fSync = false); 
  bool Read(char *key, char *value); 
  bool IsEmpty();
};
namespace dbwrapper_private {
  void HandleError(int error)
  {
    if(error != 0) {
      printf("error num was: %d\n", error);
      throw dbwrapper_error("Database could not be accessed");
    } 
  }
}
 
CDBWrapper::CDBWrapper(const boost::filesystem::path& path, size_t nCacheSize, bool fMemory, bool fWipe, bool obfuscate, bool compression, int maxOpenFiles, int mapSize, int maxReaders)
{
  printf("Opening LMDB in %s\n", path.c_str());
  dbwrapper_private::HandleError(mdb_env_create(&env)); 

  //set options
  dbwrapper_private::HandleError(mdb_env_set_mapsize(env, mapSize)); 
  dbwrapper_private::HandleError(mdb_env_set_maxreaders(env, maxReaders)); 

  dbwrapper_private::HandleError(mdb_env_open(env, path.c_str(), 0, 0644)); 

  dbwrapper_private::HandleError(mdb_txn_begin(env, NULL, 0, &ptxn)); 

  dbwrapper_private::HandleError(mdb_dbi_open(ptxn, NULL, MDB_CREATE, &dbi)); 
  printf("address of: %p\n", &ptxn);  
  printf("Opened LMDB successfully\n");
}

CDBWrapper::~CDBWrapper() {
  if (dbi) {
    mdb_dbi_close(env, dbi);
  }
  if (ptxn) {
    mdb_txn_commit(ptxn);
  }
  if (env) {
    mdb_env_close(env);
  }
  printf("Closed LMDB successfully\n");
}

bool CDBWrapper::IsEmpty()
{
  bool ret = false;
  MDB_cursor *cur;
  MDB_val key, data;
  char val[32];
  dbwrapper_private::HandleError(mdb_cursor_open(ptxn, dbi, &cur)); 
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

bool CDBWrapper::Write(const char *skey, const char *svalue, bool fSync) 
{
  MDB_txn *txn;
  MDB_val key, data;
  key.mv_size = sizeof(skey);
  key.mv_data = (char *)skey;
  data.mv_size = sizeof(svalue);
  data.mv_data = (char *)svalue;
  dbwrapper_private::HandleError(mdb_txn_begin(env, ptxn, 0, &txn)); 
  dbwrapper_private::HandleError(mdb_put(txn, dbi, &key, &data, 0)); 
  dbwrapper_private::HandleError(mdb_txn_commit(txn)); 
}

bool CDBWrapper::Read(char *skey, char *svalue)
{
  MDB_txn *txn;
  MDB_val key, data;
  dbwrapper_private::HandleError(mdb_txn_begin(env, ptxn, 0, &txn)); 
  dbwrapper_private::HandleError(mdb_get(txn, dbi, &key, &data)); 
  mdb_txn_abort(txn); 
}

int main(int argc, char* argv[]) {
  CDBWrapper *Wrapper = new CDBWrapper("./lmdb/testtestdb", 0);
  printf("Database is empty? %s\n", Wrapper->IsEmpty() ? "true" : "false");
  const char key[] = "key";
  const char value[] = "value"; 
  Wrapper->Write(key, value);
  printf("Database is empty? %s\n", Wrapper->IsEmpty() ? "true" : "false");
  char readKey[4], readValue[6];
  Wrapper->Read(readKey, readValue);
  delete Wrapper;
  return 0;
}
