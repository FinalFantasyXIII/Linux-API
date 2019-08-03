# **_Linux Posix 线程 IIII_**
## _**Introduction**_
+ 线程取消，以及线程取消的属性，线程取消点
## **_API_**
+ 取消一个线程
	>**_int_** pthread_cancel(pthread_t **_thread_**);
        	+ 取消指定的thread的线程，当pthread_cancel调用后立即返回，取消的指令会传达给指定线程，接下来线程的走向由其他因素决定。    
+ 设置线程是否启用线程取消
	>**_int_** pthread_setcancelstate(int **_state_** , int* **_oldstate_**);
        	+ **_state_** ：需要设定的状态 ，**_oldstate_** ：老状态
        	+ PTHREAD_CANCEL_DISABLE ：禁用线程取消，如果线程需要一气呵成，则禁用
        	+ PTHREAD_CANCEL_ENABLE ：启用线程取消，这是默认缺省值
    	>**_int_** pthread_setcanceltype(int **_type_** , int* **_oldtype_**);
        	+ 在 _pthread_setcancelstate_ 中若设置了启用线程取消，则type可以取以下的值：
        	+ PTHREAD_CANCEL_DEFERED ：到达线程取消点取消，例如一些sleep printf的库函数
        	+ PTHREAD_CANCEL_ASYNCHRONOUS ：可以在任意点取消    
+ 制造线程取消点
	> void pthread_testcancel(void);
        	+ 为线程制造一个线程取消点，防止一些高密度耗时计算由于缺少线程取消点，在接收到线程取消指令后无法取消的问题。

## **_DEMO_**
+ 线程取消使用方法
```
// 由于线程取消点不清晰缘故，所以需要取消的线程函数中建议加上pthread_testcancel()
// 不用pthread_testcancel时也可以使用pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr)代替
void* test(void* args){
    //下面是开启禁用线程取消，默认是可以被取消的
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    //默认取消情况下，可以使用pthread_setcanceltype 来制定取消的方式
    for(int i= 0; ;i++){
        cout << "Heloo,wordld" << endl;
        sleep(2);
        pthread_testcancel();
    }
    return nullptr;
}

int main() {
    pthread_t t;
    pthread_create(&t,nullptr,test,(void*)nullptr);

    sleep(10);

    pthread_cancel(t);

    pthread_join(t, nullptr);

    system("pause");
    return 0;
}
```
