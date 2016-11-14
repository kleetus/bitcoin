// Copyright (c) 2012-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_DBWRAPPER_H
#define BITCOIN_DBWRAPPER_H

#include "clientversion.h"
#include "serialize.h"
#include "streams.h"
#include "util.h"
#include "utilstrencodings.h"
#include "version.h"

#include <boost/filesystem/path.hpp>
#include <lmdb/lmdb.h>

class dbwrapper_error : public std::runtime_error
{
public:
    dbwrapper_error(const std::string& msg) : std::runtime_error(msg) {}
};

class CDBWrapper;

namespace dbwrapper_private {
private:
  MDB_env *env = NULL;
  MDB_dbi dbi;
  MDB_val key, data;
  MDB_txn *ptxn = NULL;
  MDB_cursor *cursor = NULL;

void HandleError(int error);

};

class CDBBatch
    friend class CDBWrapper;

private:
    //Is this so you can have nested wrapper?
    const CDBWrapper &parent;

public:
    CDBBatch(const CDBWrapper &parent) : parent(parent) { };

    template <typename K, typename V>
    void Write(const K& key, const V& value)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;

        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        ssValue.reserve(ssValue.GetSerializeSize(value));
        ssValue << value;

    }

    template <typename K>
    void Erase(const K& key)
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;

    }
};

class CDBIterator
{
private:
    const CDBWrapper &parent;

public:

    CDBIterator(const CDBWrapper &parent, leveldb::Iterator *piterIn) :
        parent(parent), piter(piterIn) { };
    ~CDBIterator();

    bool Valid();

    void SeekToFirst();

    template<typename K> void Seek(const K& key) {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
    }

    void Next();

    template<typename K> bool GetKey(K& key) {
        leveldb::Slice slKey = piter->key();
        try {
            CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
            ssKey >> key;
        } catch (const std::exception&) {
            return false;
        }
        return true;
    }

    unsigned int GetKeySize() {
    }

    template<typename V> bool GetValue(V& value) {
        try {
            CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
            ssValue >> value;
        } catch (const std::exception&) {
            return false;
        }
        return true;
    }

    unsigned int GetValueSize() {
    }

};

class CDBWrapper
{
private:

public:
    CDBWrapper(const boost::filesystem::path& path, size_t nCacheSize, bool fMemory = false, bool fWipe = false, bool obfuscate = false, bool compression = false, int maxOpenFiles = 64, int mapSize = 10485760, int maxReaders = 126);

    ~CDBWrapper();

    template <typename K, typename V>
    bool Read(const K& key, V& value) const
    {
        MDB_txn *txn;
        dbwrapper_private::HandleError(mdb_txn_begin(env, ptxn, 0, &txn)); 

        MDB_val mkey, mdata;
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;
        mkey.mv_data = ssKey;
        mkey.mv_size = sizeof(ssKey);

        dbwrapper_private::HandleError(mdb_get(txn, dbi, &mkey, &mdata)); 

        char buffer[sizeof(mdata.mv_size)];
        std::string strValue = (std::string)memcpy(buffer, mdata.mv_data, sizeof(mdata.mv_size));
        mdb_txn_abort(txn); 
        try {
            CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
            ssValue >> value;
        } catch (const std::exception&) {
          return false;
        } 
        return true;
    }

    template <typename K, typename V>
    bool Write(const K& key, const V& value, bool fSync = false)
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

    template <typename K>
    bool Exists(const K& key) const
    {
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(ssKey.GetSerializeSize(key));
        ssKey << key;

        std::string strValue;
        return true;
    }

    template <typename K>
    bool Erase(const K& key, bool fSync = false)
    {
    }

    bool WriteBatch(CDBBatch& batch, bool fSync = false);

    bool Flush()
    {
        return true;
    }

    bool Sync()
    {
    }

    CDBIterator *NewIterator()
    {
        return new CDBIterator(*this, pdb->NewIterator(iteroptions));
    }

    bool IsEmpty();
};

#endif // BITCOIN_DBWRAPPER_H

