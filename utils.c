#include "utils.h"
#include <string.h>

struct values_s values[34];

void init_values()
{
    size_t i = 0;
    values[i++].name = "MAX_PAGES";
    values[i++].name = "NODE_NUM_SIZE";
    values[i++].name = "NODE_KEY_SIZE";
    values[i++].name = "NODE_HEADER_OFFSET";
    values[i++].name = "NODE_HEADER_ID_OFFSET";
    values[i++].name = "NODE_HEADER_ID_SIZE";
    values[i++].name = "NODE_HEADER_IS_ROOT_OFFSET";
    values[i++].name = "NODE_HEADER_IS_ROOT_SIZE";
    values[i++].name = "NODE_HEADER_TYPE_OFFSET";
    values[i++].name = "NODE_HEADER_TYPE_SIZE";
    values[i++].name = "NODE_HEADER_PARENT_NODE_OFFSET";
    values[i++].name = "NODE_HEADER_PARENT_NODE_SIZE";
    values[i++].name = "NODE_HEADER_NUM_CELLS_OFFSET";
    values[i++].name = "NODE_HEADER_NUM_CELLS_SIZE";
    values[i++].name = "NODE_HEADER_RIGHTCHILD_OFFSET";
    values[i++].name = "NODE_HEADER_RIGHTCHILD_SIZE";
    values[i++].name = "NODE_HEADER_SIZE";
    values[i++].name = "NODE_INTERNAL_BODY_OFFSET";
    values[i++].name = "NODE_INTERNAL_LEFT_CHILD_CELL_OFFSET";
    values[i++].name = "NODE_INTERNAL_LEFT_CHILD_CELL_SIZE";
    values[i++].name = "NODE_INTERNAL_KEY_CELL_OFFSET";
    values[i++].name = "NODE_INTERNAL_KEY_CELL_SIZE";
    values[i++].name = "NODE_INTERNAL_CELL_SIZE";
    values[i++].name = "NODE_INTERNAL_NUM_CELLS";
    values[i++].name = "NODE_INTERNAL_BODY_SIZE";
    values[i++].name = "NODE_LEAF_BODY_OFFSET";
    values[i++].name = "NODE_LEAF_KEY_CELL_OFFSET";
    values[i++].name = "NODE_LEAF_KEY_CELL_SIZE";
    values[i++].name = "NODE_LEAF_VALUE_CELL_OFFSET";
    values[i++].name = "NODE_LEAF_VALUE_CELL_SIZE";
    values[i++].name = "NODE_LEAF_CELL_SIZE";
    values[i++].name = "NODE_LEAF_NUM_CELLS";
    values[i++].name = "NODE_LEAF_BODY_SIZE";
    values[i++].name = "PAGE_SIZE";
    i = 0;
    values[i++].val = MAX_PAGES;
    values[i++].val = NODE_NUM_SIZE;
    values[i++].val = NODE_KEY_SIZE;
    values[i++].val = NODE_HEADER_OFFSET;
    values[i++].val = NODE_HEADER_ID_OFFSET;
    values[i++].val = NODE_HEADER_ID_SIZE;
    values[i++].val = NODE_HEADER_IS_ROOT_OFFSET;
    values[i++].val = NODE_HEADER_IS_ROOT_SIZE;
    values[i++].val = NODE_HEADER_TYPE_OFFSET;
    values[i++].val = NODE_HEADER_TYPE_SIZE;
    values[i++].val = NODE_HEADER_PARENT_NODE_OFFSET;
    values[i++].val = NODE_HEADER_PARENT_NODE_SIZE;
    values[i++].val = NODE_HEADER_NUM_CELLS_OFFSET;
    values[i++].val = NODE_HEADER_NUM_CELLS_SIZE;
    values[i++].val = NODE_HEADER_RIGHTCHILD_OFFSET;
    values[i++].val = NODE_HEADER_RIGHTCHILD_SIZE;
    values[i++].val = NODE_HEADER_SIZE;
    values[i++].val = NODE_INTERNAL_BODY_OFFSET;
    values[i++].val = NODE_INTERNAL_LEFT_CHILD_CELL_OFFSET;
    values[i++].val = NODE_INTERNAL_LEFT_CHILD_CELL_SIZE;
    values[i++].val = NODE_INTERNAL_KEY_CELL_OFFSET;
    values[i++].val = NODE_INTERNAL_KEY_CELL_SIZE;
    values[i++].val = NODE_INTERNAL_CELL_SIZE;
    values[i++].val = NODE_INTERNAL_NUM_CELLS;
    values[i++].val = NODE_INTERNAL_BODY_SIZE;
    values[i++].val = NODE_LEAF_BODY_OFFSET;
    values[i++].val = NODE_LEAF_KEY_CELL_OFFSET;
    values[i++].val = NODE_LEAF_KEY_CELL_SIZE;
    values[i++].val = NODE_LEAF_VALUE_CELL_OFFSET;
    values[i++].val = NODE_LEAF_VALUE_CELL_SIZE;
    values[i++].val = NODE_LEAF_CELL_SIZE;
    values[i++].val = NODE_LEAF_NUM_CELLS;
    values[i++].val = NODE_LEAF_BODY_SIZE;
    values[i++].val = PAGE_SIZE;
}

void print_all_values()
{
    for(uint32_t i = 0; i < sizeof(values) / sizeof(struct values_s); ++i)
        printf("%s = %u\n", values[i].name, values[i].val);
}

uint32_t get_value(const char* name)
{
    for (uint32_t i = 0; i < sizeof(values) / sizeof(struct values_s); ++i)
        if (strcmp(name, values[i].name) == 0)
            return values[i].val;
    return -1;
}

void print_value(const char* name)
{
    for (uint32_t i = 0; i < sizeof(values) / sizeof(struct values_s); ++i)
        if (strcmp(name, values[i].name) == 0)
            printf("%u\n", values[i].val);
}

///////////////////////////////////////////////////////////////////////////////
// PRINTS
///////////////////////////////////////////////////////////////////////////////

static FILE* fprint_output;

void set_print_output(FILE* file)
{
    fprint_output = file;
}

void print_cells(void* node)
{
    if (get_node_type(node) == INTERNAL_NODE)
    {
        for (uint32_t i = 0; i < *node_num_cells(node); ++i)
            fprintf(fprint_output, "| Cell %u (left_child - KEY): [ %u - %u ]\n", i, *internal_node_leftchild(node, i), *internal_node_key(node, i));
    }
    else
    {
        fprintf(fprint_output, "[ ");
        uint32_t num_cells = *node_num_cells(node);
        for (uint32_t i = 0; i < num_cells; ++i)
        {
            if (i != 0)
                fprintf(fprint_output, " | ");    
            fprintf(fprint_output, "%u", *leaf_node_key(node, i));
        }
        fprintf(fprint_output, " ]\n");
    }
}

void print_cell(void* node, uint32_t cell)
{
    if (get_node_type(node) == INTERNAL_NODE)
        fprintf(fprint_output, "| Cell %u (left_child - KEY): [ %u - %u ]\n", cell, *internal_node_leftchild(node, cell), *internal_node_key(node, cell));
    else
        fprintf(fprint_output, "| Cell %u (KEY - VALUE): [ %u - %u ]\n", cell, *leaf_node_key(node, cell), *leaf_node_value(node, cell));
}

void print_header(void* node)
{
    fprintf(fprint_output, "[PAGE %u]\n", *node_id(node));
    fprintf(fprint_output, "| root: %s", is_root_node(node) ? "TRUE" : "FALSE");
    fprintf(fprint_output, " | type: %s", get_node_type(node) == LEAF_NODE ? "LEAF" : "INERNAL NODE");
    fprintf(fprint_output, " | parent: %u", *node_parent(node));
    fprintf(fprint_output, " | cells: %u", *node_num_cells(node));
    fprintf(fprint_output, " | right_child: %u\n", *node_rightchild(node));
}

void print_page(void* node)
{
    print_header(node);
    print_cells(node);
    //for (uint32_t i = 0; i < *node_num_cells(node); ++i)
        //print_cell(node, i);  
    fprintf(fprint_output, "'------------------------------\n");
}

void print_tree(bplustree* data)
{
    for(uint32_t i = 0; i < data->num_pages; ++i)
        print_page(data->nodes[i]);
}

//////////////////

void print_types(bplustree* data)
{
    for (uint32_t i = 0; i < data->size_pages && data->nodes[i]; ++i)
    {
        node_type type = get_node_type(data->nodes[i]);
        fprintf(fprint_output, "%u)\t\ttype:%s\n", i, type == INTERNAL_NODE ? "INTERNAL_NODE" : "LEAF_NODE");
    }
}

///////////////////////////////////////////////////////////////////////////////

void indent(const uint32_t level, FILE* out)
{
  for (uint32_t i = 0; i < level; ++i)
    fprintf(out, "  ");
}

void print_values(void* node, const uint8_t is_leaf, FILE* out)
{
  const uint32_t num_keys = *node_num_cells(node);
  if (!is_leaf)
  {
    for (uint32_t i = 0; i < num_keys; ++i)
    {
      fprintf(out, "%d \"%d\"", *internal_node_leftchild(node, i), *internal_node_key(node, i));
      if (i != num_keys - 1)
        fprintf(out, " ");
    }
    fprintf(out, " %d", *node_rightchild(node));
  }
  else
  {
    for (uint32_t i = 0; i < num_keys; ++i)
    {
      fprintf(out, "\"%d\"", *leaf_node_key(node, i));
      if (i < num_keys - 1)
        fprintf(out, " ");
    }
  }
}

void print_tree2(bplustree* data, const uint32_t page_num, const uint32_t indentation_level, FILE* out)
{
    void*           node        = data->nodes[page_num];
    const node_type type        = get_node_type(node);
    const uint8_t   root        = is_root_node(node);
    const uint32_t  parent      = *node_parent(node);
    const uint32_t  num_keys    = *node_num_cells(node);
    const uint32_t  right_child = *node_rightchild(node);
    switch (type)
    {
    case LEAF_NODE:
        indent(indentation_level, out);
        /* TYPE + IS_ROOT + PARENT_PTR + NUM_KEYS + RIGHTCHILD. */
        fprintf(out, "[n. num: %d; type: %s; root: %s; parent: %d; num keys: %d; right child: %d; {", page_num, "LEAF NODE", root ? "TRUE" : "FALSE", parent, num_keys, right_child);
        print_values(node, 1, out);
        fprintf(out, "}]\n");
        break;
    case INTERNAL_NODE:
        indent(indentation_level, out);
        fprintf(out, "[n. num: %d; type: %s; root: %s; parent: %d; num keys: %d; {", page_num, "INTERNAL NODE", root ? "TRUE" : "FALSE", parent, num_keys);
        print_values(node, 0, out);
        fprintf(out, "}]{\n");
        for (uint32_t i = 0; i < num_keys; ++i)
            print_tree2(data, *internal_node_leftchild(node, i), indentation_level + 1, out);
        print_tree2(data, *node_rightchild(node), indentation_level + 1, out);
        indent(indentation_level, out);
        fprintf(out, "}\n");
        break;
    }
}