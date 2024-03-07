#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <iostream>
#include <iostream>
#include <vector>
#include <utility>

using namespace std;

unsigned int n;

inline int encoder(int i,int j){
    return i*n+j;
}
pair<int,int> decoder(int k){
    return make_pair(k/n,k%n);
}
int __solve(unsigned int process){
    cout << "Multiplying matrices using "<<process<< " process\n";
    unsigned int matrix_id[3];
    matrix_id[0] = shmget(IPC_PRIVATE,sizeof(unsigned int)*n*n,IPC_CREAT|0666);
    matrix_id[1] = shmget(IPC_PRIVATE,sizeof(unsigned int)*n*n,IPC_CREAT|0666);
    matrix_id[2] = shmget(IPC_PRIVATE,sizeof(unsigned int)*n*n,IPC_CREAT|0666);
    for(int i =0;i<3;i++){
        if(matrix_id[i] == -1){
            perror("shmget error");
            return 1;
        }
    }
    unsigned int* matrix[3];
    matrix[0] = (unsigned int*)shmat(matrix_id[0],NULL,0);
    matrix[1] = (unsigned int*)shmat(matrix_id[1],NULL,0);
    matrix[2] = (unsigned int*)shmat(matrix_id[2],NULL,0);
    for(int i =0;i<3;i++){
        if(matrix[i] == (void*)-1){
            perror("shmat error");
            return 1;
        }
    }
    for(int i =0;i<n;i++){
        for(int j =0;j<n;j++){
            matrix[0][encoder(i,j)] = i*n+j;
	    matrix[1][encoder(i,j)] = i*n+j;
        }
    }
    struct timeval start, end;
    vector<pair<int,int>> need_column(process);
    unsigned int main_need = n/process;
    unsigned int remain = n%process;
    unsigned int now = 0;
    for(int i =0;i<process;i++){
        need_column[i].first = now;
        now += main_need;
        if(remain){
            now++;
            remain--;
        }
        need_column[i].second = now;
        // cout << need_column[i].first << " " << need_column[i].second << endl;
    }
    gettimeofday(&start, 0);
    for(int i =0 ;i<process;i++){
        pid_t pid = fork();
        if(pid==0){//child
            int start = need_column[i].first;
            int end = need_column[i].second;
            for(int i = start;i<end;i++){
                for(int j = 0;j<n;j++){
                    matrix[2][encoder(i,j)] = 0;
                    for(int k = 0;k<n;k++){
                        matrix[2][encoder(i,j)] += matrix[0][encoder(i,k)]*matrix[1][encoder(k,j)];
                    }
                }
            }
            exit(0);
        }else if (pid<0){
            perror("fork error");
            return 1;
        }
    }
    for(int i =0 ; i<process;i++){
        int status;
        waitpid(-1,&status,0);
    }
    gettimeofday(&end, 0);
    unsigned int res = 0;
    for(int i=0;i<n;i++){
        for(int j =0;j<n;j++){
            res += matrix[2][encoder(i,j)];
        }
    }
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    cout << "Elapsed time " << sec+(usec/1000000.0) << " sec, Checksum: "<< res << endl; 
    for(int i =0;i<3;i++){
        shmdt(matrix_id[i]);
    }
    return 0;
    
}
int main(){
    cout <<"Input the matrix dimension:";
    cin >> n;
    for(int i =1 ;i<=16;i++){
        if(__solve(i)){
            return 1;
        }
    }
}
