#include<iostream>
#include<stdio.h>
#include<string>
#include<vector>
#include<unistd.h>
#include<sys/wait.h>
#include<fcntl.h>


using namespace std;
const int maxlength = 512;
void run_script(char* argv[],int INPUT,int OUTPUT,int back){
    pid_t pid = fork();
    int original_stdin = dup(STDIN_FILENO);
    int original_stdout = dup(STDOUT_FILENO);
    
    if(pid ==0){ //child main process
        if(INPUT != STDIN_FILENO){
            dup2(INPUT,STDIN_FILENO);
            close(INPUT);
        }
        if(OUTPUT != STDOUT_FILENO){
            dup2(OUTPUT,STDOUT_FILENO);
            close(OUTPUT);
        }
        execvp(argv[0],argv);
        // finish execute
        perror("execvp error"); // normal will not reach here
        exit(1);

    }else if(pid > 0){ //parent main process
        if(!back){
            int status;
            waitpid(pid,&status,0);
        }else{
            signal(SIGCHLD,SIG_IGN);
        }
    }else{
        perror("fork error");
        exit(1);
    }
    dup2(original_stdin,STDIN_FILENO);
    dup2(original_stdout,STDOUT_FILENO);
}
int main(){
    char cwd[maxlength];
    getcwd(cwd,maxlength);
    while(1){
    //stage 1;
        cout << "> ";
        string input;
        vector<string> buffer;
        getline(cin,input); //read
        while(true){ // parse
            if(input.find(" ")==-1){
                buffer.push_back(input);
                break;
            }
	        buffer.push_back(input.substr(0,input.find(" ")));
            input = input.substr(input.find(" ")+1,input.length());
        }// end of parse
        int n = buffer.size();

        bool back = false; // NOT TO WAIT
        if(n>1 && buffer[n-1]=="&"){
            back = true;
            n--;
            signal (SIGCHLD,SIG_IGN);
        }   
        int state = 0; // 0:normal 1:pipe 2:output 3:input
        char* argv[n+1];
        char* argv2[n+1];
        int depart_index = -1;
        for(int i =0;i<n;i++){
            if (buffer[i] == "|"){
                state = 1;
                depart_index = i;
                break;
            }else if (buffer[i] == ">"){
                state = 2;
                depart_index = i;
                break;
            }else if (buffer[i] == "<"){
                state = 3;
                depart_index = i;
                break;
            }
        }
        if(state == 0){
            for(int i = 0; i < n; i++){
                argv[i] = (char*)buffer[i].c_str();
            }
            argv[n] = NULL;
        }else{
            for(int i =0; i<depart_index;i++){
                argv[i] = (char*)buffer[i].c_str();
            }
            argv[depart_index] = NULL;
            for(int i = depart_index+1; i<n;i++){
                argv2[i-depart_index-1] = (char*)buffer[i].c_str();
            }
            argv2[n-depart_index-1] = NULL;
        }
        // finish put to argv
        // start fork
        if(state==0){
            run_script(argv,STDIN_FILENO,STDOUT_FILENO,back);
        }else if(state==1){// pipe
            int fd[2];
            pipe(fd);
            run_script(argv,STDIN_FILENO,fd[1],back);
	        close(fd[1]);
            run_script(argv2,fd[0],STDOUT_FILENO,back);
        }else if(state==2){// output
            int fd = open(argv2[0], O_RDWR|O_CREAT|O_TRUNC,0666);
            run_script(argv,STDIN_FILENO,fd,back);
        }else if(state==3){// input
            int fd = open(argv2[0], O_RDONLY);
            run_script(argv,fd,STDOUT_FILENO,back);
        }else{
            cout << "error" << endl; // state should only 0~3
        }
    }
}
