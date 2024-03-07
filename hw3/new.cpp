#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <queue>
#include <vector>
#include <cmath>
using namespace std;

ifstream fin("input.txt");
int n;
int layer = 4;
int* ori_list;
int* target_array;
bool finish[(int)pow(2,4)] = {false};
bool distribute[(int)pow(2,4)] = {false};
vector<vector<int>> seg;

sem_t undo_jobs;
sem_t need_distribute;
sem_t mutex;
int threads_num;
priority_queue<int,vector<int>,greater<int>> jobs;
void bubble_sort(int *arr, int from,int to){
    for(int i = from;i<to;i++){
        for(int j = to-1;j>from;j--){
            if(arr[j]<arr[j-1]){
                int tmp = arr[j];
                arr[j] = arr[j-1];
                arr[j-1] = tmp;
            }
        }
    }
}
void merge(int *arr, int from, int middle, int to){
    int *tmp = new int[to-from];
    int i = from, j = middle, k = 0;
    while(i<middle && j<to){
        if(arr[i]<arr[j]){
            tmp[k++] = arr[i++];
        }else{
            tmp[k++] = arr[j++];
        }
    }
    while(i<middle){
        tmp[k++] = arr[i++];
    }
    while(j<to){
        tmp[k++] = arr[j++];
    }
    for(int i =0;i<to-from;i++){
        arr[from+i] = tmp[i];
    }
    delete[] tmp;
}

void* dispatcher(void* arg){
    while(1){
        //wait for job
        sem_wait(&need_distribute);
        //init
        for(int i =pow(2,3);i<pow(2,4);i++){
            if(distribute[i] == false){
		sem_wait(&mutex);
                jobs.push(i);
                distribute[i] = true;
		sem_post(&mutex);
                sem_post(&undo_jobs);
		
            }
        }
        // for merge_sort
        for(int i = 1;i<pow(2,3);i++){
            if(distribute[i] == false){
                if(finish[i*2]&&finish[i*2+1]){
		    sem_wait(&mutex);
                    jobs.push(i);
                    distribute[i] = true;
		    sem_post(&mutex);
                    sem_post(&undo_jobs);
                }
            }
        }
        //finish all jobs
        if(finish[1]){
	    sem_wait(&mutex);
            for(int i = 1;i<=threads_num;i++){
                jobs.push(0);
                sem_post(&undo_jobs);
            }
            sem_post(&mutex);
            pthread_exit(NULL);
        }
        // cout << "dispatcher end\n";
    }
}
void* worker(void* arg){
    while(1){
        //wait for job
        sem_wait(&undo_jobs);
        //critical part prevent mutiple thread get same job
        sem_wait(&mutex);
        int current_job = jobs.top();
        jobs.pop();
        sem_post(&mutex);

        //finish all jobs and exit
        if(current_job == 0){
            pthread_exit(NULL);
        }

        //do job
        vector<int> tmp = seg[current_job];
        if(tmp.size()==2){
            bubble_sort(target_array,tmp[0],tmp[1]);
        }else if(tmp.size()==3){
            merge(target_array,tmp[0],tmp[1],tmp[2]);
        }else{
            perror("job error");
        }
        finish[current_job] = true;
        // cout << "finish job "<<current_job<<endl;
        sem_post(&need_distribute);
    }
}
void clear(){
    for(int i =0;i<n;i++){
        target_array[i] = ori_list[i];
    }
    for(int i =0;i<pow(2,4);i++){
        finish[i] = false;
        distribute[i] = false;
    }
    sem_init(&undo_jobs,0,0);
    sem_init(&need_distribute,0,0);
    sem_init(&mutex,0,1);
}
void solve(){
    clear();
    pthread_t threads[threads_num+1];

    struct timeval start, end;
    gettimeofday(&start, 0);
    // for(int i =0;i<=threads_num;i++){
    //     pthread_attr_init(&threads[i]);
    // }
    pthread_create(&threads[0],NULL,dispatcher,NULL);
    for(int i =1;i<=threads_num;i++){
        pthread_create(&threads[i],NULL,worker,NULL);
    }
    sem_post(&need_distribute);

    for(int i =0;i<=threads_num;i++){
        pthread_join(threads[i],NULL);
    }
    sem_destroy(&undo_jobs);
    sem_destroy(&need_distribute);
    sem_destroy(&mutex);
    
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    cout << "worker thread #"<<threads_num<<", elapsed time: "<< sec*1000+(usec/1000.0)<<" ms\n";
    char file_name[] = "outputX.txt";
    file_name[6] = '0'+threads_num;
    ofstream fout(file_name);
    for(int i =0;i<n;i++){
        fout << target_array[i]<<" ";
    }fout <<endl;
}
void dfs(int now,int from,int to){
    if(now>=pow(2,3)){
        seg[now] = {from,to};
        return;
    }else{
        int middle = (from+to)/2;
        dfs(now*2,from,middle);
        dfs(now*2+1,middle,to);
        seg[now] = {from,middle,to};
    }
}

int main(){
    char c;
    fin >>c >> n;
    seg.resize(pow(2,3+1));
    dfs(1,0,n);
    ori_list = new int[n];
    target_array = new int[n];
    for(int i =0;i<n;i++){
        fin >> ori_list[i];
    }
    for(int i =1;i<=8;i++){
        threads_num = i;
        solve();
    }
}
