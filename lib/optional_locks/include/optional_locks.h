#include <mutex>

template<bool DoLock>
struct optional_mutex_base;

template<>
struct optional_mutex_base<true> {
    std::mutex mtx;
};

template<>
struct optional_mutex_base<false>{};

template <bool DoLock, typename MutexType>
class optional_lock_guard {};


template <typename MutexType>
class optional_lock_guard<true> {
public:
    
    /// not copy constructible
    optional_lock_guard(const MutexType&) = delete;
    
    /// not copy assignable
    optional_lock_guard* operator=(const MutexType&) = delete;
    
    /// take ownership of a mutex and lock it
    explicit optional_lock_guard(MutexType& mtx) : mtx_(mtx){
        mtx.lock();
    }
    
    /// take ownership of a mutex without locking it
    optional_lock_guard(MutexType& mtx, std::adopt_lock_t){}
    
    /// release ownership of the mutex ( call unlock() )
    ~optional_lock_guard(){
        mtx_.unlock();
    }
private:
    
    MutexType& mtx_;
};

template <typename MutexType>
class optional_lock_guard<false> {
public:
    
    /// not copy constructible
    optional_lock_guard(const MutexType&) = delete;
    
    /// not copy assignable
    optional_lock_guard* operator=(const MutexType&) = delete;
    
    /// take ownership of a mutex and lock it
    explicit optional_lock_guard(MutexType& mtx) : mtx_(mtx){}
    
    /// take ownership of a mutex without locking it
    optional_lock_guard(MutexType& mtx, std::adopt_lock_t){}
    
    /// release ownership of the mutex ( call unlock() )
    ~optional_lock_guard(){}
    
};
