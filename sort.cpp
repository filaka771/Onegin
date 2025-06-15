#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <sys/resource.h>

#include "sort.h"

// TODO: documentation
// _____________________ Quick sort implementation _____________________

/*
  static int comp(void* a,void* b){
  int* el1 = (int*)a;
  int* el2 = (int*)b;

    if(*el1 < *el2)
    return -1;
    if(*el1 == *el2)
    return 0;
    else return 1;
    }
 */
static void swap(void* el1, void* el2, size_t el_size){
    char* temp = (char*)alloca(el_size);
    memcpy(temp, el1, el_size);
    memcpy(el1, el2, el_size);
    memcpy(el2, temp, el_size);
}

static int hoare_partition(void* base, void* l, void* r, size_t el_size, int(*comp)(const void* el1, const void* el2)){
    char* left        = (char*)l;
    char* right       = (char*)r;
    char* pivot       = left;
    char* array_start = (char*) base;
    
    // comp (left, right) <= 0
    while (left <= right) {
        while (left <= right && comp((const void*)left, (const void*)pivot) <= 0) {
            left += el_size;
        }
        while (left <= right && comp((const void*)right, (const void*)pivot) >= 0) {
            right -= el_size;
        }
        if (left < right) {
            swap(left, right, el_size);
            left += el_size;
            right -= el_size;
        }
    }
    
    // Swap pivot into its final position
    swap(pivot, right, el_size);
    return (right - array_start) / el_size;
}

void quick_sort(void* base, size_t nmemb, size_t el_size, int(*comp)(const void *el1, const void* el2)){
    if (nmemb <= 1) return;

    char* char_base = (char*)base;
    char* left      = (char*)char_base;
    char* right     = char_base + (nmemb - 1) * el_size;

    int pivot_index = hoare_partition(base, left, right, el_size, comp);

    quick_sort(base, pivot_index, el_size, comp);
    quick_sort(char_base + (pivot_index + 1) * el_size, nmemb - pivot_index - 1, el_size, comp);
}

// _____________________ Bubble sort implementation _____________________

void bubble_sort(void* base, size_t nmemb, size_t el_size, int(*comp)(const void* el1, const void* el2)){
    if (nmemb <= 1) return;
    bool finish_flag = 0; // Verify, that a list one element surfaces on current iteration
    char* char_base  = (char*)base;
    char* end        = char_base + (nmemb - 1) * el_size;


    while(end - char_base > 1 && finish_flag == 0){
        finish_flag = 1;
        char* current = char_base;
        while(end - current > 1){
            if(comp(current, current + el_size) == 1){
                swap(current, current + el_size, el_size);
                current += el_size;
                finish_flag = 0;
            }
            else current += el_size;
        }
        end -= el_size;
    }
}

/*
  int main(){
  int num[] = {3, 1, 2, 3,1,1, 6,44,11};
  bubble_sort(num, 9, sizeof(int), comp);

    printf("FINAL:\n");
    for(int i = 0; i <= 8; i ++){
    if (i <= 7){
    printf("%i ", num[i]);
    }
    else {printf("%i\n", num[i]);}

    }
    return 0;

}
 */
