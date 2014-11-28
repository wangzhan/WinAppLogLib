#pragma once
#include <windows.h>

class LockGuard
{
public:
    LockGuard() { InitializeCriticalSection(&m_lock); }
    ~LockGuard() { DeleteCriticalSection(&m_lock); }

    void Lock() { EnterCriticalSection(&m_lock); }
    void Unlock() { LeaveCriticalSection(&m_lock); }

private:
    CRITICAL_SECTION m_lock;
};

class AutoLock
{
public:
    AutoLock(LockGuard &lockGuard) : m_lock(lockGuard) { m_lock.Lock(); }
    ~AutoLock() { m_lock.Unlock(); }

private:
    LockGuard &m_lock;
};
