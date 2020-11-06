#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include "bplustree.h"

#define LOG(msg, ...) fprintf(stderr, "[%s:%d] " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)

struct values_s{ const char* name; uint32_t val; };
extern struct values_s values[34];

void     init_values();
void     print_all_values();
uint32_t get_value(const char* name);
void     print_value(const char* name);

///////////////////////////////////////////////////////////////////////////////
// PRINTS
///////////////////////////////////////////////////////////////////////////////

void set_print_output(FILE* file);
void print_tree(bplustree* data);
void print_page(void* node);
void print_header(void* node);
void print_cell(void* node, uint32_t cell);
void print_cells(void* node);

void print_types(bplustree* data);

void indent(const uint32_t level, FILE* out);
void print_values(void* node, const uint8_t is_leaf, FILE* out);
void print_tree2(bplustree* data, const uint32_t page_num, const uint32_t indentation_level, FILE* out);

#endif // __UTILS_H