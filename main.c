#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bplustree.h"
#include "utils.h"

uint32_t get_next_new_num_unord()
{
    static FILE* funord = NULL;
    if (!funord)
        funord = fopen("/home/simone/bplustree/unord.txt", "r");
    char      buf[11];
    int       count = 0;
    char      c;
    uint32_t  res;
    fread(&c, 1, 1, funord);
    while (c != '\n' && !feof(funord))
    {
        buf[count++] = c;
        fread(&c, 1, 1, funord);
    }
    buf[count++] = '\0';
    sscanf(buf, "%u", &res);
    return count != 0 ? res : -1;
}

///////////////////////////////////////////////////////////////////////////////

void        print_ord(bplustree* data);
uint32_t    get_next_key(bplustree* data, uint32_t* row);
void        check_order(bplustree* data);

int main(const int argc, const char** argv)
{
    init_values();
    print_value("PAGE_SIZE");
    print_value("NODE_INTERNAL_NUM_CELLS");
    print_value("NODE_LEAF_NUM_CELLS");
    FILE* fout = fopen("/home/simone/bplustree/print_tree1.txt", "w");
    set_print_output(fout);
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    bplustree *data = malloc(sizeof(bplustree));
    bplustree_init(data);

    // uint32_t i;
    // for (i = 0; i < 32000; ++i)
    // {
    //     const uint32_t key = get_next_new_num_unord();
    //     //LOG("insert n. %u) key: %u", i, key);
    //     if (key == 20007)
    //     {
    //         uint8_t simo = 1;
    //     }
    //     if (insert(data, key, "Sim", 4) != INSERT_SUCCESS)
    //         break;
    //     check_order(data);
    // }
    // LOG("Inserted: %u", i);

    // print_tree(data);
    // fclose(fout);
    // fout = fopen("/home/simone/bplustree/print_tree.txt", "w");
    // print_tree2(data, data->root_node, 0, fout);
    // print_types(data);

    // save_db(data, "/home/simone/bplustree/mydb.db");

    Row* row = malloc(sizeof(Row));
    serialize_row(row, 0, "Simone", "Description Simo");
    insert(data, 0, row, sizeof(Row));
    serialize_row(row, 1, "Sara", "Description Sara");
    insert(data, 1, row, sizeof(Row));
    serialize_row(row, 2, "SalvoLucia", "Description SalvoLucia");
    insert(data, 2, row, sizeof(Row));
    free(row);

    save_db(data, "/home/simone/bplustree/mydb.db");

    print_ord(data);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void check_order(bplustree* data)
{
    uint32_t row;
    uint32_t last_key = get_next_key(data, &row);
    //LOG("get_next_key(%u)", last_key);
    while (last_key != -1)
    {
        uint32_t this_key = get_next_key(data, &row);
        if(this_key == -1)
            break;
        //LOG("get_next_key(%u)", this_key);
        if (last_key >= this_key)
        {
            LOG("CHECK ORDER FAILED. last_key(%u) >= this_key(%u)", last_key, this_key);
            FILE* fout = fopen("/home/simone/bplustree/print_tree1.txt", "w");
            set_print_output(fout);
            print_tree(data);
            exit(EXIT_FAILURE);
        }
        last_key = this_key;
    }
}

uint32_t get_next_key(bplustree* data, uint32_t* row)
{
    static uint8_t  flag = 0;
    static void*    root_node;
    static void*    node;
    static uint32_t node_num;
    if(!flag)
    {
        root_node = data->nodes[data->root_node];
        node      = root_node;
        while (get_node_type(node) == INTERNAL_NODE)
        {
            node = data->nodes[*internal_node_leftchild(node, 0)];
        }
        *row = 0;
        flag = 1;
    }
    static uint32_t i = 0;
    if (i < *node_num_cells(node))
    {
        ++*row;
        ++i;
        return *leaf_node_key(node, i - 1);
    }

    if ((node_num = *node_rightchild(node)) != -1)
        node = data->nodes[node_num];
    else
    {
        i = 0;
        flag = 0;
        return -1;
    }

    i = 0;
    if (i < *node_num_cells(node))
    {
        ++*row;
        ++i;
        return *leaf_node_key(node, i - 1);
    }
    i = 0;
    flag = 0;
    return -1;
}

void print_ord(bplustree* data)
{
    FILE* fout      = fopen("/home/simone/bplustree/print_ord.txt", "w");
    void* root_node = data->nodes[data->root_node];
    void* node      = root_node;
    while (get_node_type(node) == INTERNAL_NODE)
    {
        node = data->nodes[*internal_node_leftchild(node, 0)];
    }
    uint32_t row = 1;
    uint32_t node_num;
    while (1)
    {
        for (uint32_t i = 0; i < *node_num_cells(node); ++i)
            fprintf(fout, "%u\t%u\n", row++, *leaf_node_key(node, i));

        if ((node_num = *node_rightchild(node)) != -1)
            node = data->nodes[node_num];
        else
            break;
    }
}