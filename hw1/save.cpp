#include<iostream>
#include<stdio.h>
#include<string>
#include<vector>
#include<unistd.h>
#include<sys/wait.h>
#include<fcntl.h>


using namespace std;
const int maxlength = 512;
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
        }
        int n = buffer.size();

        bool back = false; // NOT TO WAIT
        if(n>1 && buffer[n-1]=="&"){
            back = true;
            n--;
            signal(SIGCHLD,SIG_IGN);
        }      
        int fd;	
        bool redir = false;
        int original_stdout = dup(STDOUT_FILENO);
        
        if(n>2 && buffer[n-2]==">"){ //IO redirection
            char *file = (char*)buffer[n-1].c_str();
            redir = true;
            fd = open(file, O_RDWR|O_CREAT|O_TRUNC,0666);
            dup2(fd,1);
            n-=2;
            }
        bool is_pipe = false;
        char* argv[n+1];
        char* argv2[n+1];
        int pipe_index = -1;
        for(int i =0;i<n;i++){
            if(buffer[i] ==string("|")){
                is_pipe = true;
                pipe_index = i;
                break;
            }
        }
        // cout << "is_pipe:"<<is_pipe << endl;
        // start put to argv
        if(is_pipe){
            int i =0;
            while (i < pipe_index) {
                argv[i] = (char *)buffer[i].c_str();
                
                // cout << buffer[i] << endl;
            i++;
            }
            argv[i] = NULL;
            i++;

            int temp  =i;
            while(i<n){
                argv2[i-temp] =(char*)buffer[i].c_str();
                // cout << buffer[i]<<endl;
                i++;	
            }
            argv[i-temp] = NULL;
        }
        else{
            for(int i = 0; i < n; i++){
                argv[i] = (char*)buffer[i].c_str();
            }
            argv[n] = NULL;
        }
        // finish put to argv
        if(is_pipe){
            int fds[2];
            pipe(fds);
            pid_t pid = fork();
            if (fork() == 0) {
            // Reassign stdin to fds[0] end of pipe.
                dup2(fds[0], STDIN_FILENO);
                close(fds[1]);
                close(fds[0]);
                pid_t pid2 = fork();
                if (pid2 == 0) {
                    // Reassign stdout to fds[1] end of pipe.
                    dup2(fds[1], STDOUT_FILENO);
                    close(fds[0]);
                    close(fds[1]);
                    // Execute the first command.
                    execvp(argv[0], argv);
                }
                wait(NULL);
                execvp(argv2[0], argv2);
            }else if(pid > 0){
                close(fds[1]);
                close(fds[0]);
                wait(NULL);
            }else{
                cout << "Fork Failed" << endl;
            }
        }else{
            pid_t pid = fork();
            if(pid < 0){ //error
                cout << "Fork Failed" << endl;
            }
            else if(pid == 0){//child
                execvp(argv[0],argv);
            }
            else{//parent
                if(!back)
                    waitpid(pid,NULL,0);
            }
        }
        if(redir){
            int back_to_cmd = dup2(original_stdout,1);
            if(back_to_cmd == -1){
                cout << "error in revert output to cmd"<<endl;
                close(fd);
                return 1;
            }
            close(fd);
        }
            buffer.clear();
        }
}
