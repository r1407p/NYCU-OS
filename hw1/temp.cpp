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
            signal (SIGCHLD,SIG_IGN);
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
    cout << "is_pipe:"<<is_pipe << endl;
    // start put to argv
	if(is_pipe){
	    int i =0;
	     while (i < pipe_index) {
            argv[i] = (char *)buffer[i].c_str();
            i++;
            cout << argv[i] << endl;
        }
        argv[i] = NULL;
        i++;

	    int temp  =i;
	    while(i<n){
		    argv2[i-temp] =(char*)buffer[i].c_str();
            cout << argv2[i-temp]<<endl;
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
    for(auto i:argv){
        cout << i << endl;
    }
    cout <<"dsfasdf\n";
    for(auto i:argv2){
        cout << i << endl;
    }
    // end put to argvs
    pid_t pid = fork();
    
    if(pid < 0){ //error
        cout << "Fork Failed" << endl;
    }
    else if(pid == 0){
        if (is_pipe) {
            int pipe_fd[2];
            pipe(pipe_fd);

            pid_t child_pid = fork();
            if (child_pid < 0) {
                cout << "Fork Failed" << endl;
            } else if (child_pid == 0) {
                // Child process: First command
                close(pipe_fd[0]);  // Close reading end of pipe
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);  // Close writing end of pipe

                execvp(argv[0], argv);
                perror("execvp failed");
                exit(1);
            } else {
                // Parent process: Second command
                close(pipe_fd[1]);  // Close writing end of pipe
                dup2(pipe_fd[0], STDIN_FILENO);
                close(pipe_fd[0]);  // Close reading end of pipe

                execvp(argv2[0], argv2);
                perror("execvp failed");
                exit(1);
            }
        }else{
            execvp(argv[0],argv);
        }
    }
    else{
        if(!back)
            waitpid(pid,NULL,0);
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