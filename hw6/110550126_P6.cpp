#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <sys/time.h>
#include <string.h>
#include <openssl/sha.h>
#include <filesystem>
#include <unordered_map>

using namespace std;
char *directory;
unordered_map<string, string> mp;

std::string get_SHA1(const std::string& filePath) {
    SHA_CTX shaContext;
    SHA1_Init(&shaContext);

    ifstream file(filePath, ios::binary);

    byte buffer[4096];
    while (!file.eof()) {
        file.read(reinterpret_cast<char*>(buffer), sizeof(buffer));
        SHA1_Update(&shaContext, buffer, file.gcount());
    }

    file.close();

    unsigned char sha1Hash[SHA_DIGEST_LENGTH];
    SHA1_Final(sha1Hash, &shaContext);

    stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << hex << setw(2) << setfill('0') << (int)sha1Hash[i];
    }

    return ss.str();
}

void dfs(string directory){
    for (const auto & entry : filesystem::directory_iterator(directory)){
        if (entry.is_directory()){
            dfs(entry.path());
        }
        else{
            string sha1  = get_SHA1(entry.path());
            if(mp.find(sha1) == mp.end()){
                mp[sha1] = entry.path();
            }
            else{
                unlink(entry.path().c_str());
                link(mp[sha1].c_str(),entry.path().c_str());
            }
        }
    }

}
int main(int argc, char* argv[]){
    // struct timeval start, end?;
    // gettimeofday(&start, 0);
    dfs(string(argv[1]));   
    // gettimeofday(&end, 0);
    // cout << "elapsed " << (end.tv_sec - start.tv_sec) << " seconds " << (end.tv_usec - start.tv_usec) << " microseconds" << endl;
}