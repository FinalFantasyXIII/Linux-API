# **Linux Posix 线程 I**

## **_Introduction_**
    本章介绍linux 线程和pthread的使用
+
+
+

## **_API_**
>pthread 函数返回值都以 0 为成功，整数值为失败

#### **_Pthread API 数据类型_**

数据类型             | 含义
--------------------| ----------------------
pthread_t           | 线程ID
pthread_mutex_t     | 互斥对象（锁）
pthread_mutexattr_t | 互斥属性对象
pthread_cond_t      | 条件变量（condition variable)
pthread_key_t       | 线程特有数据的键
pthread_once_t      | 一次性初始化控制上下文（call once）
pthread_attr_t      | 线程的属性对象，决定了新线程的各种属性

#### **_Pthread API_**
+ 线程创建
    >**int** pthread_create(pthread_t* _**thread**_ , const pthread_attr_t* **_attr_** , void*(* **_start_**)(void*) , void* **_args_**);
    + thread : 线程标识，线程id
    + attr   : 线程属性，指定线程是否可加入
    + start  : 线程工作函数，返回值和参数都是void* 类型
    + args   : 传入start的参数
+ 线程终止
    >**void** pthread_exit(void* **_retval_**);
    + retval : 指定的线程返回值
    + retval 不应该是栈区变量，如果进程的主线程调用了pthread_exit而非exit或return，那么其他线程将继续执行
+ 线程ID
    >**pthread_t** pthread_self();
    + 线程获取自身线程ID
    + pthread_equal(phtread_t , pthread_t) 用于线程ID的比较
+ 等待线程
    >**int** pthread_join(pthread_t **_thread_** , void** **_retval_**);
    + thread : 要等待的线程ID
    + retval : 如果不为NULL,那么就会保存线程返回的状态
    + 这个函数的功能类似进程中的wait函数，都是用来回收等待对象的资源的。但线程有一点不同于进程，线程可以自己释放资源
+ 线程分离
    >**int** pthread_detach(pthread_t **_thread_**);
    + 使线程脱离出去，不再受其他线程管控，一般用于分离一些独立任务。
    + 当其他线程调用exit()或主线程 return时，属于当前进程的所有线程都会销毁
+ 线程的可加入和不可加入状态
    + 如果线程是joinable状态，当线程函数自己返回退出时或pthread_exit时都不会释放线程所占用堆栈和线程描述符。只有当你调用了pthread_join之后这些资源才会被释放。
    + 若是unjoinable状态的线程，这些资源在线程函数退出时或pthread_exit时自动会被释放。
    + pthread_detach(pthread_self())可将线程变为unjoinable状态
    + 在创建线程时指定attr参数也可以将线程变为unjoinable状态，不过过于麻烦一般不用
    
## **_DEMO_**
+ demo 1 线程的创建等待操作
```
struct person{
    string name;
    int age;
    double weight;
    double height;
};

void* PersonalInfo(void* args){
    sleep(10);
    person* p = (person*)args;
    cout << p->name << endl;
    cout << p->age << endl;
    cout << p->height << endl;
    cout << p->weight << endl;
    return (void*) nullptr;
}

int main() {
    pthread_t t;
    person p{"jack",24,62.8,169.5};
    int ret = pthread_create(&t, nullptr,PersonalInfo,(void*)(&p));
    if(ret != 0){
        cerr << "thread create failed..." << endl;
        return 0;
    }
    cout << pthread_self() <<":"<< "start to join" <<endl;
    pthread_join(t,nullptr);
    return 0;
}

```
+ demo 2 线程的可加入不可加入
    + t1 为正常需要等待才能保证不会成为僵尸线程
    + t2 做了detach操作，当线程退出自动回收资源
    + t3 线程函数内部有join(self)操作，线程退出时也会自动回收资源
```
void* Echo_1 (void* args){
    sleep(1);
    const char* s = (const char*)args;
    cout << s << endl;
    return (void*) nullptr;
}
void* Echo_2 (void* args){
    sleep(2);
    pthread_join(pthread_self(), nullptr);
    const char* s = (const char*)args;
    cout << s << endl;
    return (void*) nullptr;
}

int main() {
    pthread_t t1,t2,t3;
    const char* s = "hello,world";

    pthread_create(&t1, nullptr,Echo_1,(void*)s);
    pthread_create(&t2, nullptr,Echo_1,(void*)s);
    pthread_create(&t3, nullptr,Echo_2,(void*)s);

    pthread_join(t1,nullptr);
    pthread_detach(t2);
    sleep(5);
    return 0;
}
```
