#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#define MAX_SIZE 20000



struct block {
    size_t size;
    int free;
    struct block *prev;
    struct block *next;
};
struct block *head = NULL;
int transfer_size(int ori){
    if( ori %32 == 0){
        return ori;
    }
    else{
        return (ori/32 + 1)*32;
    }
}
void output_res(int res){
    char output_str[35]; // Adjust the size accordingly
    sprintf(output_str, "Max Free Chunk Size = %d\n", res);
    write(STDOUT_FILENO, output_str, sizeof(output_str));
    munmap(head, MAX_SIZE);
}


void init(){
    head =(struct block*)mmap(NULL, MAX_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if(head == MAP_FAILED){
        output_res(-1);
        perror("mmap");
        return;
    }
    head->free = 1;
    head->prev = NULL;
    head->next = NULL;
    // head->size = MAX_SIZE - sizeof(block);
    head->size = MAX_SIZE - 32;
}
void* malloc(size_t size){
    if(head == NULL){ // start
        init();
    }
    struct block *curr = head;
    if(size == 0){ // end
        size_t res = 0;
        while(curr){
            if(curr -> free && curr->size > res){
                res = curr -> size;
	    }
            curr = curr -> next;
        }
        output_res(res);
        return NULL;
    }
    int true_size = transfer_size(size);
    while(curr){
        if(curr -> free && curr -> size >= true_size){
            if(curr -> size == true_size){
                curr -> free = 0;
            }
            else{
                struct block *next = curr + 1 + true_size/32;
                next -> free = 1;
                next -> size = curr -> size - true_size - 32;
                next -> prev = curr;
                next -> next = curr -> next;
                if(curr -> next){
                    curr -> next -> prev = next;
                }
                curr -> free = 0;
                curr -> size = true_size;
                curr -> next = next;
            }
            return (void*)(curr + 1);
        }
        curr = curr -> next;
    }
    return NULL;

}
void free(void *ptr){
	struct block *curr = (struct block*)ptr -1;
	
    curr -> free = 1;
    if(curr -> next && curr -> next -> free ){
        curr -> size += curr -> next -> size + 32;
        if( curr -> next->next){
            curr -> next->next -> prev = curr;
        }
        curr -> next = curr -> next -> next;
    }
    if(curr -> prev && curr -> prev -> free ){
        curr -> prev -> size += curr -> size + 32;
        curr -> prev -> next = curr -> next;
        if(curr -> next){
            curr -> next -> prev = curr -> prev;
        }
    }
}
