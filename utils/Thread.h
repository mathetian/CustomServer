#ifndef _THREAD_WIN_POS_H
#define _THREAD_WIN_POS_H

#ifdef __WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace utils
{

#ifdef __WIN32
    typedef HANDLE id_type;
    typedef unsigned(WINAPI *Task)(void *);
#define THRINIT NULL
#else
    typedef pthread_t id_type;
    typedef void *(*Task)(void *);
#define THRINIT -1
#endif

class Thread
{
public:
    Thread(Task task = NULL, void * args = NULL) : m_task(task), m_args(args), m_tid(THRINIT) { }
    Thread(const Thread & thr) : m_task(thr.m_task), m_args(thr.m_args), m_tid(thr.m_tid) { }
    id_type  run();
    void     join();
    void     cancel();
    static id_type  getIDType();

private:
    id_type  m_tid;
    Task   m_task;
    void * m_args;
};

class CondVar;

class Mutex
{
public:
    Mutex();
    virtual ~Mutex();

public:
    void lock();
    void unlock();
    int  trylock();

private:
#ifdef __WIN32
    CRITICAL_SECTION m_mutex;
#else
    pthread_mutex_t m_mutex;
#endif
    friend class CondVar;
};

class CondVar
{
public:
    CondVar(Mutex* mutex = NULL);
    ~CondVar();

public:
    void wait();
    void signal();
    void signalAll();

private:
#ifdef __WIN32
    HANDLE m_cond;
#else
    pthread_cond_t m_cond;
#endif
    Mutex * m_mutex;
};

class Noncopyable
{
protected:
    Noncopyable() { }
    ~Noncopyable() { }

private:
    Noncopyable( const Noncopyable& );
    Noncopyable& operator=( const Noncopyable& );
};

class SingletonMutex : Noncopyable
{
public:
    static SingletonMutex& getInstance();

private:
    SingletonMutex() {}

public:
    Mutex m;
};

class ScopeMutex : Noncopyable
{
public:
    ScopeMutex(Mutex * pmutex);
    ~ScopeMutex();

private:
    Mutex * m_pmutex;
};

/**constructor can't be empty parameter.**/
class RWLock
{
public:
    RWLock(Mutex * mutex = NULL);

public:
    void readLock();
    void readUnlock();
    void writeLock();
    void writeUnlock();

private:
    int  m_nReader;
    int  m_nWriter;
    int  m_wReader;
    int  m_wWriter;

private:
    Mutex * m_mutex;
    CondVar m_condRead;
    CondVar m_condWrite;
};

/**Todo list**/
class ReentrantLock
{
public:
    ReentrantLock() : m_id(-1), m_cond(&m_tmplock), m_time(0) { }
    void  lock();
    void  unlock();
    bool  trylock();
private:
    Mutex m_lock;
    id_type m_id;
    Mutex m_tmplock;
    CondVar m_cond;
    int   m_time;
};

class Atomic
{
public:
    Atomic(int r) : obj(r) { }

    operator int()
    {
        return obj;
    }

    Atomic & operator=( int val )
    {
        obj = val;
        __sync_synchronize();
        return *this;
    }

    int operator++()
    {
        return __sync_add_and_fetch( &obj, 1 );
    }

    int operator++( int )
    {
        return __sync_fetch_and_add( &obj, 1 );
    }

    int operator+=( int val )
    {
        return __sync_add_and_fetch( &obj, val );
    }

    int operator--()
    {
        return __sync_sub_and_fetch( &obj, 1 );
    }

    int operator--( int )
    {
        return __sync_fetch_and_sub( &obj, 1 );
    }

    int operator-=( int val )
    {
        return __sync_sub_and_fetch( &obj, val );
    }

    int addAndGet( int val)
    {
        return __sync_add_and_fetch(&obj, val);
    }

    // Perform an atomic CAS operation
    // returning the value before the operation
    int exchange( int oldval, int newval )
    {
        return __sync_val_compare_and_swap( &obj, oldval, newval );
    }

private:
    int obj;
};

};

#endif