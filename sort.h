#include <stdio.h>
#ifndef SORT_H
#define SORT_H

void bubble_sort(void* base, size_t nmemb, size_t el_size, int(*comp)(const void* el1, const void* el2));
void quick_sort(void* base, size_t nmemb, size_t el_size, int(*comp)(const void* el1, const void* el2));

#endif
