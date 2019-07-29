# **_Linux Posix 线程 II_**
## _**Introduction**_
+ 死锁的出现和解决方法
    + 不同函数对同一个锁进行了lock操作，然后出现在锁区域互相调用的情况。这是死锁常见的场景
    + 按照一定层次顺序加锁，如t1 t2 永远按照先t1后t2的形式进行加锁
    + 对第一个锁进行lock，后面的锁进行try_lock操作，如果后面任意一个锁lock失败，则主动unlock前面所有已经lock的锁
## **_API_**
>pthread 函数返回值都以 0 为成功，整数值为失败
+ 线程加锁
    > **_int_** pthread_mutex_lock(pthread_mutex_t* **_mutex_**);
    + mutex : 锁变量，一般对于被加锁的线程来说它是一个相对全局的变量
+ 线程解锁
    > **_int_** pthread_mutex_unlock(pthread_mutex_t* **_mutex_**);
    + unlock 和 lock相互成对
+ 锁初始化
    >**_int_** pthread_mutex_init(pthread_mutex_t* **_mutex_**,pthread_mutexattr_t* **_attr_**);
    + mutex : 需要初始化的锁对象
    + attr : 指定锁的属性
    + 需要调用该函数的锁的种类有:栈锁，堆锁，不使用默认属性的全局锁
+ 锁销毁
    >**_int_** pthread_mutex_destory(pthread_mutex_t* **_mutex_**);
    + 与锁的初始化配对
+ 条件变量发送信号
    >**_int_** pthread_cond_signal(pthread_cond_t* **_cond_**);
    
    >**_int_** pthread_cond_broadcast(pthread_cond_t* **_cond_**);
    + cond : 指定的条件变量
    + pthread_cond_signal 是唤醒一个线程让其获取cpu执行权
    + pthread_cond_broadcast 是唤醒所有线程让其竞争获取cpu执行权
    + n对一情况下用singal , n对n的情况下就用broadcast
+ 条件变量等待信号
    >**_int_** pthread_cond_wait(pthread_cond_t* **_cond_**,pthread_mutex_t* **_mutex_**);
    
    >**_int_** pthread_cond_timewait(pthread_cond_t* **_cond_**,pthread_mutex_t* **_mutex_**,const struct timespec* **_abstime_**);
    + pthread_cond_wait 和 pthread_cond_timewait 无其他差异，除了后者指定了时间限制
    + 当调用pthread_cond_wait后，顺序执行1.解锁互斥量mutex 2.阻塞线程，直至收到另一线程的cond信号 3.重新锁定mutex
    + pthread_cond_wait应包含于while条件判断的循环中。线程醒来后会执行处于条件判断后的指令，这时应重新检查条件，正确则执行循环外的，不正确应重新调用wait挂起
## **_DEMO_**
+ demo 1 : 创建1000个线程运行20秒，统计每个线程抢到锁的次数
```
static int status = 1;
static pthread_mutex_t  t = PTHREAD_MUTEX_INITIALIZER;
map<pthread_t,int> mp;

void* Print (void* args){
    while(status){
        pthread_mutex_lock(&t);
        mp[pthread_self()]++;
        pthread_mutex_unlock(&t);
    }
    return nullptr;
}

void* change (void* args){
    sleep(20);
    status = 0;
    return nullptr;
}

int main() {
    pthread_t t[1000];
    for (int i = 0;i<1000;++i){
        pthread_create(&t[i], nullptr,Print,(void*) nullptr);
    }
    pthread_t kt;
    pthread_create(&kt, nullptr, change,(void*) nullptr);
    pthread_detach(kt);

    for (int i = 0;i<1000;++i){
        pthread_join(t[i], nullptr);
    }

    for(const auto& i : mp)
        cout << i.first << " : " <<i.second <<endl;
    system("pause");
    return 0;
}
```
+ demo 2 : 共同lock同一个锁的函数出现 锁区域互相调用的死锁情况
```
static pthread_mutex_t  t = PTHREAD_MUTEX_INITIALIZER;
static const char* s = "hello,world";

void* Print (void* args){
    pthread_mutex_lock(&t); //会死锁在这
    cout << s <<endl;
    pthread_mutex_unlock(&t);
    return nullptr;
}

void* Modify (void* args){
    pthread_mutex_lock(&t);
    s = "hahhahaha";
    Print(nullptr);
    pthread_mutex_unlock(&t);
    return nullptr;
}

int main() {
    pthread_t t1,t2;
    pthread_create(&t2, nullptr,Modify,(void*)nullptr);
    pthread_create(&t1, nullptr,Print,(void*)nullptr);
    
    pthread_join(t2,nullptr);
    pthread_join(t1, nullptr);
    
    system("pause");
    return 0;
}
```
+ demo 3 : 生产者消费者
```
static pthread_mutex_t  mt = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  ct = PTHREAD_COND_INITIALIZER;
static int number = 0;

void* creator (void* args){
    while(true){
        pthread_mutex_lock(&mt);
        number += 2;
        cout << pthread_self() << ":" << number <<endl;
        pthread_mutex_unlock(&mt);
        int ret = pthread_cond_broadcast(&ct);
        if (ret != 0)
            return nullptr;
        sleep(1);
    }
}

void* consumer (void* args){
    while (true){
        pthread_mutex_lock(&mt);
        while(!number){
            int ret = pthread_cond_wait(&ct,&mt);
            if(ret != 0)
                return nullptr;
        }
        number--;
        cout << pthread_self() << ":" << number <<endl;
        pthread_mutex_unlock(&mt);
        sleep(1);
    }
}

int main() {
    pthread_t t1,t2,t3;
    pthread_create(&t1, nullptr,creator,(void*) nullptr);
    pthread_create(&t2, nullptr,consumer,(void*) nullptr);
    sleep(2);
    pthread_create(&t3, nullptr,consumer,(void*) nullptr);

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);
    pthread_join(t3, nullptr);
    system("pause");
    return 0;
}
```
+ demo 3 ：使用条件变量代替join
```
enum RS{
    RUN,
    STOP,
    JOINED
};

struct Attribute {
    thread t;
    RS s;
    int num;
};

static vector<Attribute> va(10);
static int unjoined = 0;
static int alive_num = 10;
static mutex mt;
static condition_variable cv;

void work_func(int index){
    this_thread::sleep_for(seconds(va[index].num));
    {
        lock_guard<mutex> lg(mt);
        cout << index << ":" << this_thread::get_id() << endl;
        va[index].s = RS::STOP;
        unjoined++;
    }
    cv.notify_one();
}

int main() {
    for(int i=0;i<va.size();i++){
        va[i].s = RS::RUN;
        va[i].num = i+10;
        va[i].t = thread{work_func,i};
    }
    sleep(3);
    cout << "start ..." <<endl;
    while(alive_num > 0){
        unique_lock<mutex> ul(mt);
        cv.wait(ul,[](){
            return unjoined;
        });
        for(auto& i : va){
            if(i.s == RS::STOP){
                i.t.join();
                i.s = RS::JOINED;
                alive_num--;
                unjoined--;
            }
        }
    }
    cout << "over" <<endl;
    system("pause");
    return 0;
}
```
