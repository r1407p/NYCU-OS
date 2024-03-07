#include <bits/stdc++.h>
using namespace std;

int main(){
    int n = 800;
    int process = 7;
    vector<pair<int,int>> need_column(process);
    int main_need = n/process;
    int remain = n%process;
    int now = 0;
    for(int i =0;i<process;i++){
        need_column[i].first = now;
        now += main_need;
        if(remain){
            now++;
            remain--;
        }
        need_column[i].second = now;
        cout << need_column[i].first << " " << need_column[i].second << endl;
    }
}