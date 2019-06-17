#include<iostream>
#include<unistd.h>
#include<signal.h>
#include<string>
#include<vector>
#include<map>
#include<sys/types.h>
#include<assert.h>
#include <sys/wait.h>

using namespace std;

map<pid_t,string> mp;

int init(vector<string>& vgs){
    for(const auto& i : vgs){
        int ret = fork();
        if(ret < 0){
            return -1;
        }else if(ret == 0){
            execlp(i.data(),i.data(),NULL);
            exit(0);
        }else{
            mp[ret] = i ;
        }
    }
    return 0;
}

int restart(pid_t pid)
{
    int ret = fork();
    switch (ret)
    {
        case -1:
            return -1;
        case 0:
            execlp(mp[pid].data(),mp[pid].data(),NULL);
            exit(0);
        default:
            mp[ret] = mp[pid];
            mp.erase(mp.find(pid));
            break;
    }
    return 0;
}

void deal_signal_child(int num){
    pid_t pid;
    while((pid = waitpid(-1,NULL,WNOHANG)) > 0){
        cout << pid << endl;
        restart(pid);
    }
}

int main(int argc,char* argv[]){
    assert(argc > 1);
    daemon(1,1);
    signal(SIGCHLD,deal_signal_child);
    vector<string> vgs;
    for(int i=1;i < argc; ++i)
        vgs.emplace_back(argv[i]);
    int ret =  init(vgs);
    if(ret == -1)
        return ret;
    for (const auto& i: mp)
        cout << i .first  << "  " <<i.second <<endl;
    sleep(10);

    while(1);
}
