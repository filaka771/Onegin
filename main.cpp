#include <memory.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <sys/stat.h>

#include "sort.h"

#define FILE_NAME "onegin.txt"

typedef struct {
    size_t num_lines;
    wchar_t* text;
    wchar_t** flines;
    wchar_t** rlines;
} Text;

Text read_file(const char* file_name){
    Text text = {
        .num_lines = 0,
        .text      = NULL,
        .flines    = NULL,
        .rlines    = NULL
    };
    // Open file in read mode
    FILE* file = fopen(file_name, "r");
    if(file == NULL){
        perror("Cannot open file");
        return text;
    }

    // Get file size without using FILE stream
    struct stat st;
    if(stat(FILE_NAME, &st)){
        perror("Cannot get info about file");
        return text;
    }

    // Memory allocation
    size_t size = 1; // Size of text

    wchar_t* str_text = (wchar_t*)calloc(st.st_size + 2, sizeof(wchar_t));
    str_text[0] = L'\0';

    size_t capacity = st.st_size / 80; // Capacity of array with pointers on lines
    wchar_t** lines = (wchar_t**)calloc(capacity, sizeof(wchar_t*));
    lines[0] = str_text;


    wchar_t line[512];
    size_t num_lines = 1;


    //TODO: drop empty lines
    // Read file and replace \n with \0
    while(fgetws(line, 512, file)) {
        size_t line_len = wcslen(line);
        if (line[0] == L'\n'){
            continue;
        }

        if (num_lines == capacity - 1){
            capacity *= 1.5; //TODO: make coefficient defined constant
            lines = (wchar_t**)realloc((void*)lines, capacity * sizeof(wchar_t*));
        }

        wcscpy(str_text + size, line);
        if (str_text[size - 1] == L'\n'){
            str_text[size - 1] = L'\0';
            lines[num_lines] = str_text + size - 1;
            num_lines ++;
        }

        size += line_len;
    }

    wchar_t** flines = (wchar_t**)calloc(size, sizeof(wchar_t*));
    wchar_t** rlines = (wchar_t**)calloc(size, sizeof(wchar_t*));

    for (size_t i = 0; i <= num_lines - 1; i++){
        flines[i] = lines[i] + 1;
        rlines[i] = lines[i + 1] - 1;
    }

    free((void*)lines);

    text.num_lines = num_lines;
    text.text  = str_text;
    text.flines = flines;
    text.rlines = rlines;

    return text;
}


int write_file(size_t num_lines, wchar_t** lines, const char* file_name){
    FILE* file = fopen(file_name, "w");
    if (file == NULL){
        perror("Cannot open file");
        return -1;
    }

    for (size_t i = 0; i <= num_lines - 1; i++){
        if (fputws(lines[i], file) != -1){
            fputws(L"\n",file);
            continue;
        }         
        else {perror("Error while writing into file");
            return -1;
        }
    }

    fclose(file);
    return 0;
}

const wchar_t** split_lines(const wchar_t* text){
    size_t length = wcslen(text);
    size_t num_of_lines = 0;
    const wchar_t* sym = text;

    while(sym <= text + length){
        if (*sym == L'\0'){
            num_of_lines ++;
        }
        sym ++;    
    }

    const wchar_t** lines = (const wchar_t**)calloc(num_of_lines + 1, sizeof(sym));

    sym = text;
    size_t i = 0;
    while(sym <= text + length){
        if (*sym == L'\n'){
            lines[i] = sym;
            i ++;
        }
        sym ++;
    }
    lines[num_of_lines] = NULL;
    return lines;

}

int f_compare_wstrings(const void* a, const void* b){
    int direction = 1;
    wchar_t* str1 = *(wchar_t**)a;
    wchar_t* str2 = *(wchar_t**)b;

    while (*str1 != L'\0' || *str2 != L'\0'){
        while (towlower(*str1) == L' ') str1++;
        while (towlower(*str2) == L' ') str2++;
        if (towlower(*str1) == towlower(*str2)){
            str1 += direction;
            str2 += direction;
            continue;
        }

        if (towlower(*str1) < towlower(*str2)){
            return -1;
        }
        else if (towlower(*str1) > towlower(*str2)){
            return 1;
        }
        if (*str1 != L'\0') str1++;
        if (*str2 != L'\0') str2++;
    }
    return 0;
}

int r_compare_wstrings(const void* a, const void* b){
    int direction = -1;
    wchar_t* str1 = (wchar_t*)a;
    wchar_t* str2 = (wchar_t*)b;

    while (*str1 != L'\0' || *str2 != L'\0'){

        if ((*str1 == L' ' && *str2 == L' ') || (towlower(*str1) == towlower(*str2))){
            str1 += direction;
            str2 += direction;
            continue;
        }

        else if (towlower(*str1 == L' ')){
            str1 += direction;
            continue;
        }

        else if (towlower(*str2 == L' ')){
            str2 += direction;
            continue;
        }
        else if (towlower(*str1) < towlower(*str2)){
            return -1;
        }

        else return 1;
    }
    if (towlower(*str1) == L'\0'){
        return 1;
    }

    else if (towlower(*str2) == L'\0'){
        return -1;
    }
    return 0;
}

int main() {
    setlocale(LC_ALL, "");
    Text text = read_file(FILE_NAME);
    
    if (text.num_lines == 0 || text.flines == NULL) {
        fprintf(stderr, "No lines read\n");
        return 1;
    }


    // Sort lines alphabetically (skip first line if needed)
    quick_sort(text.flines, text.num_lines - 1, sizeof(wchar_t*), f_compare_wstrings);
    qsort(text.rlines, text.num_lines - 1, sizeof(wchar_t*), r_compare_wstrings);

    // Print sorted lines
    for (size_t i = 1; i < text.num_lines; i++) {
        wprintf(L"%ls\n", text.flines[i]);
    }

    //Write sorted lines into file
    write_file(text.num_lines, text.flines, "test.txt");

    // Cleanup
    free(text.text);
    free(text.flines);
    free(text.rlines);
    return 0;
}

