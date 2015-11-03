#pragma once

namespace tokLib 
{
namespace db 
{

const string LOG_DB("db");

template<typename T>
class database : private boost::noncopyable
{
private:
    T db_;
    mutex mutex_;
    map<short, pair<short, thread::id> > dbUsing_; //index, refcount, threadId

public:
    int create(dbConfig& config) 
    {
        for (short i=0; i<config.poolSize; ++i) 
        {
            thread::id tid;
            if (false == dbUsing_.insert(make_pair(i, make_pair(0, tid))).second)
            {
                writeFileLog(LOG_DB, "dbUsingMap insert error");
                return errDefault;
            }
        }
        return db_.create(config);
    }

    int terminate() 
    {
        return db_.terminate();
    }

    short get() 
    {
        unique_lock<mutex> lock(mutex_);
        auto iter =
            find_if(dbUsing_.begin(), dbUsing_.end(),
                    [this](const pair<short, pair<short, thread::id> >& entry)->bool
                    {
                        if (entry.second.second == this_thread::get_id())
                            return true;
                        return false;
                    });
        if (iter != dbUsing_.end()) 
        {
            ++(iter->second.first);
            return iter->first;
        } 
        else 
        {
            iter = find_if(dbUsing_.begin(), dbUsing_.end(),
                           [](const pair<short, pair<short, thread::id> >& entry)->bool
                           {
                               if (entry.second.first == 0) return true;
                               return false;
                           });
            if (iter != dbUsing_.end())
            {
                iter->second.first = 1;
                iter->second.second = this_thread::get_id();
                return iter->first;
            }
        }   
        return -1;
    }

    void release(short index) 
    {
        unique_lock<mutex> lock(mutex_);
        auto iter = dbUsing_.find(index);
        if (iter != dbUsing_.end()) 
        {
            --(iter->second.first);
            if (0 == iter->second.first)
                iter->second.second = thread::id();
        }
    }
    
    int query(short index, const string& qr, dbQueryResult& dqr)
    {
        return db_.query(index, qr, dqr);
    }
};

template<typename T>
class dbCaller : private boost::noncopyable
{
private:
    T& db_;
    short threadIndex_;
    int rowIndex_;
    dbQueryResult dqr_;
    boost::format format_;

public:
    dbCaller(T& db) : db_(db)
    {
        clear(false);
        threadIndex_ = db_.get();
    }
    ~dbCaller() 
    {
        clear();
    }
    void clear(bool release=true)
    {
        if (release) db_.release(threadIndex_);
        threadIndex_ = -1;
        rowIndex_ = 0;
        dqr_.clear();
        return;
    }
    
    template<typename V>
    void format(V value)
    {
        format_ % value;
    }
    template<typename V, typename ...FuncArgs>
    void format(V value, const FuncArgs&... args)
    {
        format_ % value;
        format(args...);
    }
    template<typename ...FuncArgs>
    void format(const FuncArgs&... args)
    {
        format(args...);
    }
    template<typename ...FuncArgs>
    int query(const string& qr, const FuncArgs&... args)
    {
        if (qr.empty()) return errDefault;
        if (threadIndex_ == -1)
        {
            clear(false);
            threadIndex_ = db_.get();
        }
        format_ = boost::format(qr);
        if (sizeof...(args))
            format(args...);
        if (db_.query(threadIndex_, format_.str(), dqr_) != errNone)
        {
            writeFileLog(LOG_DB, "db query error (" << format_.str() << ")");
            return errDefault;
        }
        return errNone;
    }
    int fetch()    //read row
    {
        if (rowIndex_ >= dqr_.rowLen)
            return errDefault;
        ++rowIndex_;
        return errNone;
    }
    template <typename V>
    int fetchValue(const string& fieldName, V& v)   //read row value
    {
        int index = rowIndex_ - 1;
        if (index < 0)
            return errDefault;
        
        auto data = dqr_.datas[index];
        auto iter = find_if(data.begin(), data.end(),
                            [&fieldName](const pair<string, string>& entry)->bool
                            {
                                if (!entry.first.compare(fieldName))
                                    return true;
                                return false;
                            });
        if (iter == data.end())
            return errDefault;
        
        v = boost::lexical_cast<V>(iter->second);
        
        return errNone;
    }
};
    
class dbQuery : private boost::noncopyable
{
private:
    map<int, string> query_;
    const string errQuery_;
    
    dbQuery();
    ~dbQuery();
public:
    static dbQuery& getInstance() { static dbQuery instance; return instance; }
    int parse(const string& query);
    const string& query(const int queryNumber);
};
    
}
}
