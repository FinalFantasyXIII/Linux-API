# **_Linux Posix 线程 III_**
## _**Introduction**_
+ 线程安全函数是指可以同时供给多个线程安全调用的函数
+ pthread_once函数和c++ 11提供的call_once函数可以解决一次性初始化问题
+ 线程特有数据可以在不改变原来的api的情况下实现线程安全
+ 线程特有数据就是为每个线程分配一个独立互不干扰的内存，让每个线程在这段内存内干活
## **_API_**
+ Pthread 的call_once 函数
    > **_int_** pthread_once(pthread_once_t* **_once_control_**,void(***_init_**)(void));
    + init : 资源初始化函数，在线程函数调用pthread_once确保所有线程只会调用一次init
+ 获取线程特有数据key
    > **_int_** pthread_key_create(pthread_key_t* **_key_**,void(***_destory_**)(void*));
    + key与线程的关系是 一对多
    + destory : 销毁线程特有数据的析构函数，参数是一个指向堆内存指针
    + 当线程返回时，只要key关联的所有线程的内存不为空，那就会调用destory销毁它们

+ 设置线程特有数据
    > int pthread_setspecific(pthread_key_t key,const void* buf);
    + 绑定key与buf

+ 获取线程特有数据
    > void* pthread_getspecific(pthread_key_t key);
    + 根据key获取绑定的buf
## **_DEMO_**
+ 使用案例
```
static pthread_key_t key;
static pthread_once_t once = PTHREAD_ONCE_INIT;
const int length = 256;

void destory(void* p){
    free(p);
}

void createKey(void){
    int ret = pthread_key_create(&key,destory);
    if (ret != 0){
        cerr << "pthread_key_create error ..." << endl;
        exit(-1);
    }
}

char* stdmyerr(int err){
    char* buf;
    int ret = pthread_once(&once,createKey);
    if (ret != 0){
        cerr << "pthread once err..." << endl;
        exit(-1);
    }
    buf = (char*)pthread_getspecific(key);
    if(buf == nullptr){
        buf = new char[length];
        if(!buf){
            cerr << "buf new err ..." << endl;
            exit(-1);
        }
        int ret = pthread_setspecific(key,(void*)buf);
        if (ret != 0){
            cerr << "pthread_setspecific err..." << endl;
            exit(-1);
        }
    }
    return buf;
}

int main() {

    system("pause");
    return 0;
}
```
