#include <iostream>
#include <fstream>
#include <vector>
#include <sys/time.h>
#include <string.h>

const int hash_base = 1009;
using namespace std;
char *filename;
class LFU_node{
public:
    int page;
    int freq;
    int seq;
    int current_idx;
    LFU_node(int page, int freq):page(page), freq(freq){}
    bool operator<(const LFU_node &other){
        if (freq < other.freq){
            return true;
        }else if(freq == other.freq){
            return seq < other.seq;
        }else{
            return false;
        }
    };
};
class Min_Heap{
private:
    int size;
    int mx_size;
    vector<LFU_node*> min_heap;
public:
    Min_Heap(int mx_size):mx_size(mx_size), size(0){
        min_heap.resize(mx_size);
    }
    LFU_node* operator[](int idx){
        return min_heap[idx];
    }
    void update_node(int idx){// update node to satisfy heap property (we only increase the value of node)
        int left = idx*2+1;
        int right = idx*2+2;
        int target;
        //no child
        if(left >= size){
            return;
        }
        // only left child
        else if(right == size){
            target = left;
        }
        else if(*min_heap[left] < *min_heap[right]){
            target = left;
        }else{
            target = right;
        }
        if(*min_heap[target]<*min_heap[idx]){
            swap(min_heap[target], min_heap[idx]);
            min_heap[target]->current_idx = target;
            min_heap[idx]->current_idx = idx;
            update_node(target);
        }
    }
    void pop(){
        swap(min_heap[0], min_heap[size-1]);
        min_heap[0]->current_idx = 0;
        size--;
        auto remove = min_heap[size];
        min_heap[size] = NULL;
        delete remove;
        // delete min_heap[size];
        update_node(0);
    }
    void insert(LFU_node *node){
        min_heap[size] = node;
        int idx = size;
        node->current_idx = idx;
        size++;
        while(idx > 0){
            int parent = (idx-1)/2;
            if(*min_heap[idx] < *min_heap[parent]){
                swap(min_heap[idx], min_heap[parent]);
                min_heap[idx]->current_idx = idx;
                min_heap[parent]->current_idx = parent;
                idx = parent;
            }else{
                break;
            }
        }
    }
    int get_size(){
        return size;
    }
};
void LFU(int buffer_size){
    ifstream fin;
    fin.open(filename);
    int hit = 0;
    int miss = 0;
    int total = 0;
    int page;
    vector<LFU_node*> hash_table[hash_base];
    Min_Heap heap(buffer_size);
    LFU_node *now = NULL;
    while(fin >> page){
        total++;
        bool isHit = false;
        for (int i = 0; i < hash_table[page%hash_base].size(); i++){
            if(hash_table[page%hash_base][i]->page == page){
                isHit = true;
                now = hash_table[page%hash_base][i];
                break;
            }
        }
        if(isHit){
            hit++;
            now->freq++;
            now->seq = total;
            heap.update_node(now->current_idx);
        }else{
            miss++;
            if(heap.get_size() == buffer_size){
                // remove min
                auto remove = heap[0];
                for(int i = 0; i < hash_table[remove->page%hash_base].size(); i++){
                    if(hash_table[remove->page%hash_base][i]->page == remove->page){
                        swap(hash_table[remove->page%hash_base][i], hash_table[remove->page%hash_base].back());
                        hash_table[remove->page%hash_base].pop_back();
                        break;
                    }
                }
                heap.pop();
            }
            // add to heap
            LFU_node *now = new LFU_node(page, 1);
            now->seq = total;
            heap.insert(now);
            hash_table[page%hash_base].push_back(now);
        }
    }
    printf("%d\t%d\t\t%d\t\t%.10f\n", buffer_size, hit, miss, (double)miss/total);
    fin.close();
}

struct LRU_node{
    int page;
    LRU_node *next;
    LRU_node *prev;
    LRU_node(int page):page(page), next(NULL), prev(NULL){}
};

void LRU(int buffer_size){
    ifstream fin;
    fin.open(filename);
    int hit = 0;
    int miss = 0;
    int total = 0;
    int page;
    vector<LRU_node*> hash_table[hash_base];
    // vector<int> hash_table_sizes(hash_base);
    int total_size = 0;
    LRU_node *head = NULL;
    LRU_node *tail = NULL;
    LRU_node *now = NULL;
    while(fin >> page){
        total++;
        bool isHit = false;
        for (int i = 0; i < hash_table[page%hash_base].size(); i++){
            if(hash_table[page%hash_base][i]->page == page){
                isHit = true;
                now = hash_table[page%hash_base][i];
                break;
            }
        }
        if(isHit){
            hit++;
            if(now != head){// if current is head not need to do any thing;
                // if current is tail, then move tail to prev node
                if(now == tail){
                    tail = tail->prev;
                    tail->next = NULL;
                    head->prev = now;
                    now->next = head;
                    now->prev = NULL;
                    head = now;
                }else{
                    now->prev->next = now->next;
                    now->next->prev = now->prev;
                    head->prev = now;
                    now->next = head;
                    now->prev = NULL;
                    head = now;
                }
            }
        }else{
            miss++;
            if(total_size == buffer_size){
                // remove tail
                auto remove = tail;
                tail = tail->prev;
                tail->next = NULL;
                for(int i = 0; i < hash_table[remove->page%hash_base].size(); i++){
                    if(hash_table[remove->page%hash_base][i]->page == remove->page){
                        swap(hash_table[remove->page%hash_base][i], hash_table[remove->page%hash_base].back());
                        hash_table[remove->page%hash_base].pop_back();
                        break;
                    }
                }
                total_size--;
            }
            // add to head
            LRU_node *now = new LRU_node(page);
            if(head == NULL){
                head = now;
                tail = now;
            }else{
                head->prev = now;
                now->next = head;
                head = now;
            }
            hash_table[page%hash_base].push_back(now);
            total_size++;
        }
    }
    printf("%d\t%d\t\t%d\t\t%.10f\n", buffer_size, hit, miss, (double)miss/total);
    fin.close();
}


int main(int argc, char* argv[]){
    struct timeval start, end;
    filename = strdup(argv[1]);
    int buffer_size[]= {64, 128, 256, 512};
    gettimeofday(&start, 0);
    cout << "LFU policy\n";
    cout << "Frame\tHit\t\tMiss\t\tPage fault ratio\n";
    for(int i =0; i < 4 ; i++ ){
        LFU(buffer_size[i]);
    }
    gettimeofday(&end, 0);
    printf("Total elapsed time %.4f sec\n", (end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec) / 1e6);

    gettimeofday(&start, 0);
    cout << "LRU policy\n";
    cout << "Frame\tHit\t\tMiss\t\tPage fault ratio\n";
    for(int i =0 ; i<4 ; i++){
        LRU(buffer_size[i]);
    }
    gettimeofday(&end, 0);
    printf("Total elapsed time %.4f sec\n", (end.tv_sec-start.tv_sec) + (end.tv_usec-start.tv_usec) / 1e6);
    


}