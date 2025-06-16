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
#define RESULT_FILE_NAME "result.txt"

typedef struct {
    wchar_t* bline;  
    wchar_t* eline;  
} Line;

typedef struct {
    size_t num_lines;
    wchar_t* text;   
    Line* lines;     
} Text;


// --------------------- Working with files ---------------------

Text read_file(const char* file_name) {
    Text text = {
        .num_lines = 0,
        .text      = NULL,
        .lines     = NULL
    };

    FILE* file = fopen(file_name, "r");
    if (!file) {
        perror("Cannot open file");
        return text;
    }

    struct stat st;
    if (stat(file_name, &st)) {
        perror("Cannot get info about file");
        fclose(file);
        return text;
    }

    size_t capacity = st.st_size / 80 + 2;
    wchar_t* str_text = (wchar_t*)calloc(st.st_size + 2, sizeof(wchar_t));
    if (!str_text) {
        perror("Memory allocation failed");
        fclose(file);
        return text;
    }

    wchar_t** line_starts = (wchar_t**)calloc(capacity, sizeof(wchar_t*));
    if (!line_starts) {
        perror("Memory allocation failed");
        free(str_text);
        fclose(file);
        return text;
    }

    size_t size = 0;
    size_t num_lines = 0;
    wchar_t str_line[512];

    while (fgetws(str_line, sizeof(str_line)/sizeof(wchar_t), file)) {
        size_t len = wcslen(str_line);

        if (len == 1 && str_line[0] == L'\n') continue;

        if (num_lines + 1 >= capacity) {
            capacity *= 2;
            wchar_t** new_starts = (wchar_t**)realloc(line_starts, capacity * sizeof(wchar_t*));
            if (!new_starts) {
                perror("Realloc failed");
                break;
            }
            line_starts = new_starts;
        }

        line_starts[num_lines++] = str_text + size;

        wcscpy(str_text + size, str_line);

        if (str_text[size + len - 1] == L'\n') {
            str_text[size + len - 1] = L'\0';
        }

        size += wcslen(str_text + size) + 1;  // Move past null-terminator
    }

    fclose(file);

    Line* lines = (Line*)calloc(num_lines, sizeof(Line));
    if (!lines) {
        perror("Memory allocation failed");
        free(str_text);
        free(line_starts);
        return text;
    }

    for (size_t i = 0; i < num_lines; i++) {
        lines[i].bline = line_starts[i];
        lines[i].eline = (i < num_lines - 1)
                         ? line_starts[i + 1] - 1
                         : str_text + size;
    }

    free(line_starts);

    text.num_lines = num_lines;
    text.text = str_text;
    text.lines = lines;

    return text;
}
static int write_file(Text block, FILE* file){
    if (file == NULL){
        perror("Cannot open file");
        return -1;
    }

    for (size_t i = 0; i <= block.num_lines - 1; i++){
        const wchar_t* line = block.lines[i].bline;
        if ((wint_t)fputws(line, file) == WEOF){
            perror("Error while writing file");
            fclose(file);
            return -1;
        }

        if((wint_t)fputws(L"\n", file) == WEOF){
            perror("Error while writing newline");
            fclose(file);
            return -1;
        }
    }

    if ((wint_t)fputws(L"\n\n\n\n\n", file) == WEOF){
        perror("Error while separating blocks");
        fclose(file);
    }
    return 0;
}

// --------------------- Comparators ---------------------

int pointer_compare(const void* line_1, const void* line_2){
    const Line* line1 = (const Line*)line_1;
    const Line* line2 = (const Line*)line_2;

    const wchar_t*str1 = line1->bline;
    const wchar_t*str2 = line2->bline;

    return (str1 <= str2)? -1 : 1;
}

int compare_wstrings(const void* line_1, const void* line_2, int direction) {
    const Line* line1 = (const Line*)line_1;
    const Line* line2 = (const Line*)line_2;

    const wchar_t* str1 = (direction == 1) ? line1->bline : line1->eline - 1;
    const wchar_t* str2 = (direction == 1) ? line2->bline : line2->eline - 1;

    const int step = (direction == 1) ? +1 : -1;

    while (1) {
        // Skip non-alpha chars
        while (str1 <= line1->eline && str1 >= line1->bline && !iswalpha(*str1)) {
            str1 += step;
        }
        while (str2 <= line2->eline && str2 >= line2->bline && !iswalpha(*str2)) {
            str2 += step;
        }

        if ((direction == 1 && (str1 > line1->eline || str2 > line2->eline)) ||
            (direction != 1 && (str1 < line1->bline || str2 < line2->bline))) {
            break;
        }

        wchar_t c1 = towlower(*str1);
        wchar_t c2 = towlower(*str2);

        if (c1 != c2) {
            return (c1 < c2) ? -1 : 1;
        }

        str1 += step;
        str2 += step;
    }

    if ((str1 > line1->eline && str2 <= line2->eline) ||
        (str1 < line1->bline && str2 >= line2->bline)) {
        return -1; 
    }
    if ((str2 > line2->eline && str1 <= line1->bline) ||
        (str2 < line2->bline && str1 >= line1->bline)) {
        return 1; 
    }

    return 0; 
}

int f_compare_wstrings(const void* line_1, const void* line_2){
    return compare_wstrings(line_1, line_2, 1);
}

int r_compare_wstrings(const void* line_1, const void* line_2){
    return compare_wstrings(line_1, line_2, -1);
}

// --------------------- Main ---------------------

int main() {

    //Open and read file
    setlocale(LC_ALL, "");
    Text text = read_file(FILE_NAME);
    
    if (text.num_lines == 0 || text.lines == NULL) {
        fprintf(stderr, "No lines read\n");
        return 1;
    }

    // Open file for writing results
    FILE* file = fopen(RESULT_FILE_NAME, "w");

    // Sorting text 
    quick_sort(text.lines, text.num_lines, sizeof(Line), f_compare_wstrings);
    write_file(text, file);
    quick_sort(text.lines,text.num_lines, sizeof(Line),r_compare_wstrings);
    write_file(text, file);
    quick_sort(text.lines,text.num_lines, sizeof(Line), pointer_compare);
    write_file(text, file);

    fclose(file);

    // Free
    free(text.text);
    free(text.lines);
    return 0;
}

