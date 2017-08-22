#include "../concurrent.h"
#include "../debug.h"

qMutex qMutex_constructor(){
    qMutex mu;
    int errn;
    if(!(errn=pthread_mutex_init(&(mu.mu),NULL))){
        qPanic("MutexConstruct Error:POSIX call pthread_mutex_init failed with errno %d\n",errn);
    }
    mu.lock=q__Mutex_lock;
    mu.unlock=q__Mutex_unlock;
    return mu;
}

void q__Mutex_lock(qMutex mu){
    int errn;
    if(!(errn=pthread_mutex_lock(mu.mu))){
        qPanic("MutexAcquire Error:POSIX call pthread_mutex_lock failed with errno %d.\n",errn);
    }
}

void q__Mutex_unlock(qMutex mu){
    int errn;
    if(!(errn=pthread_mutex_unlock(mu.mu))){
        qPanic("MutexRelease Error:POSIX call pthread_mutex_unlock failed with errno %d.\n",errn);
    }
}

void qMutex__destructor(qMutex* mu){
    int errn;
    if(!(errn=pthread_mutex_destroy(mu->mu))){
        qPanic("MutexDestroy Error:POSIX call pthread_mutex_destroy failed with errno %d.\n",errn);
    }
    mu->mu=NULL;
}