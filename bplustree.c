#include "bplustree.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

NODE_NUM_TYPE* node_id(void* node)
{
    return node + NODE_HEADER_ID_OFFSET;
}

uint8_t is_root_node(void* node)
{
    return *(uint8_t*)((uint32_t*)(node + NODE_HEADER_IS_ROOT_OFFSET));
}

void set_root_node(void* node, const uint8_t root)
{
    *(uint8_t*)((uint32_t*)(node + NODE_HEADER_IS_ROOT_OFFSET)) = root;
}

node_type get_node_type(void* node)
{
    return *(node_type*)(uint32_t*)(node + NODE_HEADER_TYPE_OFFSET);
}

void set_node_type(void* node, const node_type type)
{
    *(node_type*)(uint32_t*)(node + NODE_HEADER_TYPE_OFFSET) = type;
}

NODE_NUM_TYPE* node_parent(void* node)
{
    return node + NODE_HEADER_PARENT_NODE_OFFSET;
}

NODE_NUM_TYPE* node_num_cells(void* node)
{
    return node + NODE_HEADER_NUM_CELLS_OFFSET;
}

NODE_NUM_TYPE* node_rightchild(void* node)
{
    return node + NODE_HEADER_RIGHTCHILD_OFFSET;
}


// INTERNAL ///////////////////////////////////////////////////////////////////

void* internal_node_cell(void* node, const uint32_t cell)
{
    return node + NODE_INTERNAL_BODY_OFFSET + NODE_INTERNAL_CELL_SIZE * cell;
}

NODE_NUM_TYPE* internal_node_leftchild(void* node, const uint32_t cell)
{
    return node + NODE_INTERNAL_BODY_OFFSET + NODE_INTERNAL_CELL_SIZE * cell + NODE_INTERNAL_LEFT_CHILD_CELL_OFFSET;
}

NODE_KEY_TYPE* internal_node_key(void* node, const uint32_t cell)
{
    return internal_node_cell(node, cell) + NODE_INTERNAL_KEY_CELL_OFFSET;
}

// Automaticamente restituisco o il quello scritto nella riferimento di una cella (il sinistro della successiva), oppure il rightchild
// nell'header della pagina che viene utilizzato sempre come ultimo rightchild, in modo da avere sempre le celle piene e non celle
// formate solo dal leftchild.
NODE_NUM_TYPE* internal_node_rightchild(void* node, const uint32_t cell)
{
    if (cell == *node_num_cells(node) - 1)
        return node_rightchild(node);
    else
        return internal_node_leftchild(node, cell + 1);
}


// LEAF ///////////////////////////////////////////////////////////////////////

void* leaf_node_cell(void* node, const uint32_t cell)
{
    return node + NODE_LEAF_BODY_OFFSET + NODE_LEAF_CELL_SIZE * cell;
}

NODE_KEY_TYPE* leaf_node_key(void* node, const uint32_t cell)
{
    return node + NODE_LEAF_BODY_OFFSET + NODE_LEAF_CELL_SIZE * cell + NODE_LEAF_KEY_CELL_OFFSET;
}

NODE_NUM_TYPE* leaf_node_value(void* node, const uint32_t cell)
{
    return node + NODE_LEAF_BODY_OFFSET + NODE_LEAF_CELL_SIZE * cell + NODE_LEAF_VALUE_CELL_OFFSET;
}

///////////////////////////////////////////////////////////////////////////////

NODE_NUM_TYPE internal_node_binary_search(void* node, const uint32_t key)
{
    const uint32_t size = *node_num_cells(node);
    uint32_t start = 0, end = size;

    while (start != end)
    {
        const uint32_t pos = (end + start) / 2;
        if (key == *internal_node_key(node, pos))
            return pos;
        else if (key < *internal_node_key(node, pos))
            end = pos;
        else
            start = pos + 1;
    }
    return start;
}

void internal_node_insert_key(void* node, const uint32_t cell, const uint32_t key)
{
    const uint32_t num_cells = *node_num_cells(node);
    memmove(internal_node_cell(node, cell + 1), internal_node_cell(node, cell), (num_cells - cell) * NODE_INTERNAL_CELL_SIZE);
    *internal_node_key(node, cell) = key;
    ++*node_num_cells(node);
}

// 0%
void split_internal(bplustree* data, const uint32_t new_internal, const uint32_t node_num)
{
    void* new_internal_node = data->nodes[new_internal];
    void* old_node          = data->nodes[node_num];

    const uint32_t cell_remained  = NODE_INTERNAL_NUM_CELLS / 2;
    const uint32_t cell_to_move   = NODE_INTERNAL_NUM_CELLS - cell_remained - 1;  // The first is only to award upper, otherwise there will be 2 copies in internals.
    
    const uint32_t min_key_award  = *internal_node_key(old_node, cell_remained);    // The cell not copied.
    memcpy(internal_node_cell(new_internal_node, 0), internal_node_cell(old_node, NODE_INTERNAL_NUM_CELLS - cell_to_move), cell_to_move * NODE_INTERNAL_CELL_SIZE);

    // Update num_cells.
    *node_num_cells(new_internal_node)  = cell_to_move;
    *node_num_cells(old_node)           = cell_remained;

    // Metto a posto i riferimenti destri delle celle.
    *internal_node_rightchild(new_internal_node, cell_to_move - 1) = *node_rightchild(old_node);
    *node_rightchild(old_node) = *internal_node_leftchild(old_node, cell_remained);

    if (is_root_node(old_node))
    {
        create_root(data, new_internal, node_num, min_key_award);
    }
    else
    {
        const uint32_t parent = *node_parent(old_node);
        award_new_min_key(data, parent, node_num, new_internal, min_key_award);
    }
    *node_parent(new_internal_node) = *node_parent(old_node);  // Spitted leafs have always the same parent.
}

NODE_NUM_TYPE leaf_node_binary_search(void* node, const uint32_t key)
{
    const uint32_t  size  = *node_num_cells(node);
    uint32_t        start = 0;
    uint32_t        end   = size;

    while (start != end)
    {
        const uint32_t pos = (end + start) / 2;
        if (key == *leaf_node_key(node, pos))
            return pos;
        else if (key < *leaf_node_key(node, pos))
            end = pos;
        else
            start = pos + 1;
    }
    return start;
}

void leaf_node_insert_key(void* node, const uint32_t cell, const uint32_t key)
{
    const uint32_t num_cells = *node_num_cells(node);
    memmove(leaf_node_cell(node, cell + 1), leaf_node_cell(node, cell), (num_cells - cell) * NODE_LEAF_CELL_SIZE);
    *leaf_node_key(node, cell) = key;
    ++*node_num_cells(node);
}

void split_leaf(bplustree* data, const uint32_t new_leaf_num, const uint32_t old_leaf_num)
{
    void* new_node        = data->nodes[new_leaf_num];
    void* old_node        = data->nodes[old_leaf_num];
    void* new_cells_start = leaf_node_cell(new_node, 0);
    void* old_cells_start = leaf_node_cell(old_node, 0);

    const uint32_t cells_to_move  = NODE_LEAF_NUM_CELLS / 2;
    const uint32_t cells_remained = NODE_LEAF_NUM_CELLS - cells_to_move;
    // Copy the upper part in the new leaf.
    memcpy(new_cells_start, (uint8_t*)old_cells_start + cells_remained * NODE_LEAF_CELL_SIZE, cells_to_move * NODE_LEAF_CELL_SIZE);
    // Update number of cells.
    *node_num_cells(new_node) = cells_to_move;
    *node_num_cells(old_node) = cells_remained;
    // Update rightchilds.
    *node_rightchild(new_node) = *node_rightchild(old_node);
    *node_rightchild(old_node) = new_leaf_num;
    // Add new min_key of the new splitted leaf, as new key in parent.
    update_parent(data, old_leaf_num);
}

// 100%
void create_root(bplustree* data, const uint32_t num_new, const uint32_t num_old, const uint32_t min_key_right)
{
    void*           old_leaf_node = data->nodes[num_old];
    void*           new_leaf_node = data->nodes[num_new];

    const uint32_t  new_root      = get_unused_node(data);
    void*           new_root_node = data->nodes[new_root];
    initialize_node(new_root_node, new_root, INTERNAL_NODE);
    set_root_node(new_root_node, 1);
    data->root_node = new_root;

    // Setto key e i due child.
    ++*node_num_cells(new_root_node);
    *internal_node_key(new_root_node, 0)        = min_key_right;
    *internal_node_leftchild(new_root_node, 0)  = num_old;
    *internal_node_rightchild(new_root_node, 0) = num_new;

    // Aggiorno i parent delle due leaf.
    *node_parent(old_leaf_node) = new_root;
    *node_parent(new_leaf_node) = new_root;

    set_root_node(old_leaf_node, 0);
}

// 50%
void update_parent(bplustree* data, const uint32_t old_leaf)
{
    void*           old_node      = data->nodes[old_leaf];
    const uint32_t  new_leaf      = *node_rightchild(old_node);
    void*           new_node      = data->nodes[new_leaf];
    const uint32_t  min_key_right = *leaf_node_key(new_node, 0);    // Minimum key in the right node to be awarded above.

    if (is_root_node(old_node))
    {   // TESTING: IF OK
        create_root(data, new_leaf, old_leaf, min_key_right);
    }
    else
    {   // TESTING: ELSE DA TESTARE 
        const uint32_t parent = *node_parent(old_node);
        award_new_min_key(data, parent, old_leaf, new_leaf, min_key_right);
    }
    *node_parent(new_node) = *node_parent(old_node);  // TESTING: SERVE DAVVERO LA RIGA 246? // Spitted leafs have always the same parent.
}

// 66%
void update_childs(bplustree* data, uint32_t new_internal)
{
    void*           new_node = data->nodes[new_internal];
    //const uint32_t  num_cells_to_move = NUM_CELLS / 2 - 1;  // todo: To check this shit?
    const uint32_t  num_cells = *node_num_cells(new_node);

    if (num_cells > 0)
    {
        // Update childs to point the new parent.
        for (uint32_t i = 0; i < num_cells; ++i)
        {
            const uint32_t nn = *internal_node_leftchild(new_node, i);
            *node_parent(data->nodes[nn]) = new_internal;
        }
        const uint32_t nn = *internal_node_rightchild(new_node, num_cells - 1);
        *node_parent(data->nodes[nn]) = new_internal;
    }
}

// 0% - non capisco quando questa funzione realmente serva.
void update_key_parent(bplustree* data, const uint32_t node_num, const uint32_t last_min_key, const uint32_t min_key)
{
    const uint32_t parent_num = *node_parent(data->nodes[node_num]);
    const uint32_t index      = internal_node_binary_search(data->nodes[parent_num], last_min_key);
    if (*internal_node_rightchild(data->nodes[parent_num], index) == node_num)
        *internal_node_key(data->nodes[parent_num], index) = min_key;
}

// 0%
void award_new_min_key(bplustree* data, const uint32_t parent, const uint32_t old_num, const uint32_t new_num, const uint32_t key_to_award)
{
    if (key_to_award == 20007)
    {
        uint8_t simo = 1;
    }
    void* parent_node = data->nodes[parent];

    if (!(*node_num_cells(parent_node) < NODE_INTERNAL_NUM_CELLS))
    {
        // Should split internal node.
        const uint32_t new_internal = get_unused_node(data);
        initialize_node(data->nodes[new_internal], new_internal, INTERNAL_NODE);
        split_internal(data, new_internal, parent);
        update_childs(data, new_internal);  // Childs must points to the same (new) parent.

        const uint32_t parent = *node_parent(data->nodes[old_num]);
        parent_node = data->nodes[parent];      // todo node_num OR left_internal ?!?! IT'S THE SAME OF THE SIBLING.

        //const uint32_t last_right_child = *node_rightchild(parent_node);
        //*internal_node_leftchild(parent_node, *node_num_cells(parent_node) - 1) = last_right_child; // todo ?? CHECK.
    }
    const uint32_t index = internal_node_binary_search(parent_node, key_to_award);
    internal_node_insert_key(parent_node, index, key_to_award);
    *internal_node_leftchild(parent_node, index)    = old_num;
    *internal_node_rightchild(parent_node, index)   = new_num;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void bplustree_init(bplustree* data)
{
    data->size_pages = MAX_PAGES;
    data->nodes      = malloc(sizeof(uint32_t*) * MAX_PAGES);
    data->num_pages  = 0;
    data->root_node  = -1;
    for (uint32_t i = 0; i < MAX_PAGES; ++i)
        data->nodes[i] = NULL;
}

uint32_t get_unused_node(bplustree* data)
{
    // Cambiare in modo da utilizzare poi le pagine non usate.
    if (data->num_pages < MAX_PAGES)
    {
        get_node(data, data->num_pages);
        return data->num_pages - 1;
    }
    else
    {
        LOG("ERROR. Impossible to get new node. Max num node reached.");
        exit(EXIT_FAILURE);
    }
}

void* get_node(bplustree* data, const uint32_t num_page)
{
    if (!data->nodes[num_page])
    {
        void* node = malloc(PAGE_SIZE);
        initialize_node(node, num_page, LEAF_NODE);
        if (data->num_pages == 0)
        {
            set_root_node(node, 1);
            data->root_node = 0;
        }
        data->nodes[num_page] = node;
        ++data->num_pages;
    }
    return data->nodes[num_page];
}

ExecuteResult insert(bplustree* data, const uint32_t key, const void* row_buf, const size_t size)
{
    Cursor* cur = new_cursor(data);
    if (data->num_pages > 0)
    {
        cursor_find(cur, key);
    }
    else
    {
        const uint32_t node = get_unused_node(data);
        set_node_type(data->nodes[node], LEAF_NODE);
        set_root_node(data->nodes[node], 1);
        data->root_node = node;
        cur->node = node;
        cur->cell = 0;
    }
    return leaf_insert(cur, key, row_buf, size);
    free(cur);
}

void initialize_node(void* node, const uint32_t id, const node_type type)
{
    *node_id(node) = id;
    set_root_node(node, 0);
    set_node_type(node, type);
    *node_parent(node)      = -1;
    *node_num_cells(node)   = 0;
    *node_rightchild(node)  = -1;
}

// TESTING: 80%
ExecuteResult leaf_insert(Cursor* cur, const uint32_t key, const void* row_buf, const size_t size)
{
    bplustree*      data      = cur->data;
    const uint32_t  num_page  = cur->node;
    void*           node      = data->nodes[num_page];

    if (*leaf_node_key(node, cur->cell) == key && *node_num_cells(node) > 0)
    {   // TESTING: IF OK
        LOG("Duplicate key. Insert failed\n");
        return DUPLICATE_KEY;
    }
    else
    {   // TESTING: ELSE OK
        if (*node_num_cells(node) < NODE_LEAF_NUM_CELLS)  // C'e' spazio nella foglia, tutto va bene.
        {   // TESTING: IF OK
            // const uint32_t last_min_key = *leaf_node_key(node, 0);  // TODO: PROBLEMA! NESSUNO MI DICE CHE NEL CASO SIA VUOTA SIA INIZIALIZZATA A 0 E QUINDI SIA CORRETTO.
            leaf_node_insert_key(node, cur->cell, key);
            memcpy(leaf_node_value(node, cur->cell), row_buf, ROW_SIZE);
            // const uint32_t min_key = *leaf_node_key(node, 0);
            // if (!is_root_node(node) && last_min_key != min_key) // TESTING: IF ??
            //     update_key_parent(data, num_page, lanst_min_key, min_key);
        }
        else                                    // There's no more space in the leaf, I have to split it.
        {   // TESTING: ELSE OK
            const uint32_t new_page = get_unused_node(data);
            initialize_node(data->nodes[new_page], new_page, LEAF_NODE);
            split_leaf(data, new_page, num_page);

            // Recall insert. Now I'm sure there's a space without do splitting.
            insert(data, key, row_buf, ROW_SIZE);
        }
    }
    return INSERT_SUCCESS;
}

void save_db(bplustree* data, const char* file)
{
    const int fd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR );
    if (fd == -1)
    {
        LOG("Error opening file: %s\n", file);
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < data->num_pages; ++i)
    {
        const ssize_t byte = write(fd, data->nodes[i], PAGE_SIZE);
        if (byte != -1)
            printf("%u Bytes was written\n", (uint32_t)byte);
        else
            LOG("Error writing file\n");
    }
}




///////////////////////////////////////////////////////////////////////////////
// Cursor
///////////////////////////////////////////////////////////////////////////////

Cursor* new_cursor(bplustree* data)
{
    Cursor* cur = malloc(sizeof(Cursor));
    cur->data = data;
    cur->node = -1;
    cur->cell = -1;
}

void cursor_find(Cursor* cur, const uint32_t key)
{
    // Ricerca binaria all'interno di ogni singolo nodo.
    bplustree* data = cur->data;
    if (data->num_pages > 0)
    {
        uint32_t  num_page = data->root_node;
        void*     node     = data->nodes[num_page];
        while (get_node_type(node) == INTERNAL_NODE)
        {
            const uint32_t num_cell = internal_node_binary_search(node, key);
            if (num_cell < *node_num_cells(node))
            {
                const uint32_t key_in_cell = *internal_node_key(node, num_cell);
                if (key == key_in_cell)
                    num_page = *internal_node_rightchild(node, num_cell);
                else
                    num_page = *internal_node_leftchild(node, num_cell);
            }
            else
            {
                num_page = *node_rightchild(node);
            }
            node = data->nodes[num_page];
        }
        cur->node = num_page;
        cur->cell = leaf_node_binary_search(node, key);
    }
}

#define MIN(a, b) (a < b ? a : b)


void serialize_row(Row* row, uint32_t id, char name[], char descr[])
{
    if (row && name, descr)
    {
        row->id = id;
        strncpy(row->name, name, NAME_LENGTH);
        row->name[NAME_LENGTH - 1] = '\0';
        strncpy(row->descr, descr, DESCR_LENGTH);
        row->descr[DESCR_LENGTH - 1] = '\0';
    }
}

void deserialize_row(Row* row, uint32_t id, char* name, size_t sname, char descr, size_t sdescr)
{

}
