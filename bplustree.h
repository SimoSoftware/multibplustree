#ifndef __BPLUSTREE_H
#define __BPLUSTREE_H

#include <stdint.h>
#include <stddef.h>

/* Celle composte da chiave-valore (o per quelle interne da chiave-riferimento pagina sottostante) */
//#define NUM_CELLS 3
#define PAGE_SIZE 8192  // 8KB
#define MAX_PAGES 62000


/* GENERAL NODE HEADER */
/* [ ID | IS_ROOT | TYPE | PARENT_NODE | NUM_CELLS | RIGHTCHILD ] */

#define NODE_NUM_TYPE                       uint32_t
#define NODE_KEY_TYPE                       uint32_t
//#define ROW_TYPE                            uint32_t

#define NODE_NUM_SIZE                       (sizeof(NODE_NUM_TYPE))
#define NODE_KEY_SIZE                       (sizeof(NODE_KEY_TYPE))
#define ROW_SIZE                            (sizeof(struct Row))



#define NODE_HEADER_OFFSET                  (0)

#define NODE_HEADER_ID_OFFSET               (0)
#define NODE_HEADER_ID_SIZE                 NODE_NUM_SIZE
#define NODE_HEADER_IS_ROOT_OFFSET          (NODE_HEADER_ID_OFFSET + NODE_HEADER_ID_SIZE)
#define NODE_HEADER_IS_ROOT_SIZE            (sizeof(uint32_t))              /* Poi cambiare in 8 che comunque e' uno spreco visto che basterebbe un solo bit. */
#define NODE_HEADER_TYPE_OFFSET             (NODE_HEADER_IS_ROOT_OFFSET + NODE_HEADER_IS_ROOT_SIZE) 
#define NODE_HEADER_TYPE_SIZE               (sizeof(uint32_t))              /* Idem basterebbe solo un bit */
#define NODE_HEADER_PARENT_NODE_OFFSET      (NODE_HEADER_TYPE_OFFSET + NODE_HEADER_TYPE_SIZE)
#define NODE_HEADER_PARENT_NODE_SIZE        NODE_NUM_SIZE
#define NODE_HEADER_NUM_CELLS_OFFSET        (NODE_HEADER_PARENT_NODE_OFFSET + NODE_HEADER_PARENT_NODE_SIZE)
#define NODE_HEADER_NUM_CELLS_SIZE          NODE_NUM_SIZE
#define NODE_HEADER_RIGHTCHILD_OFFSET       (NODE_HEADER_NUM_CELLS_OFFSET + NODE_HEADER_NUM_CELLS_SIZE)
#define NODE_HEADER_RIGHTCHILD_SIZE         NODE_NUM_SIZE

#define NODE_HEADER_SIZE                    (NODE_HEADER_ID_SIZE + NODE_HEADER_IS_ROOT_SIZE + NODE_HEADER_TYPE_SIZE + NODE_HEADER_PARENT_NODE_SIZE + NODE_HEADER_NUM_CELLS_SIZE + NODE_HEADER_RIGHTCHILD_SIZE)



/* INTERNAL NODE BODY */
/* [ LEFT_CHILD | KEY ] ... */

#define NODE_INTERNAL_BODY_OFFSET                   (NODE_HEADER_OFFSET + NODE_HEADER_SIZE)

#define NODE_INTERNAL_LEFT_CHILD_CELL_OFFSET        (0)
#define NODE_INTERNAL_LEFT_CHILD_CELL_SIZE          NODE_NUM_SIZE
#define NODE_INTERNAL_KEY_CELL_OFFSET               (NODE_INTERNAL_LEFT_CHILD_CELL_OFFSET + NODE_INTERNAL_LEFT_CHILD_CELL_SIZE)
#define NODE_INTERNAL_KEY_CELL_SIZE                 NODE_KEY_SIZE

#define NODE_INTERNAL_CELL_SIZE                     (NODE_INTERNAL_LEFT_CHILD_CELL_SIZE + NODE_INTERNAL_KEY_CELL_SIZE)
//#define NODE_INTERNAL_BODY_SIZE                     (NODE_INTERNAL_CELL_SIZE * NUM_CELLS)

#define NODE_INTERNAL_NUM_CELLS                     ((PAGE_SIZE - NODE_HEADER_SIZE) / NODE_INTERNAL_CELL_SIZE)

#define NODE_INTERNAL_BODY_SIZE                     (NODE_INTERNAL_CELL_SIZE * NODE_INTERNAL_NUM_CELLS)



/* LEAF NODE BODY */
/* [ KEY | VALUE ] ... */

#define NODE_LEAF_BODY_OFFSET                   (NODE_HEADER_OFFSET + NODE_HEADER_SIZE)

#define NODE_LEAF_KEY_CELL_OFFSET               (0)
#define NODE_LEAF_KEY_CELL_SIZE                 NODE_NUM_SIZE
#define NODE_LEAF_VALUE_CELL_OFFSET             (NODE_LEAF_KEY_CELL_OFFSET + NODE_LEAF_KEY_CELL_SIZE)
#define NODE_LEAF_VALUE_CELL_SIZE               ROW_SIZE

#define NODE_LEAF_CELL_SIZE                     (NODE_LEAF_KEY_CELL_SIZE + NODE_LEAF_VALUE_CELL_SIZE)
//#define NODE_LEAF_BODY_SIZE                     (NODE_LEAF_CELL_SIZE * NUM_CELLS)

#define NODE_LEAF_NUM_CELLS                     ((PAGE_SIZE - NODE_HEADER_SIZE) / NODE_LEAF_CELL_SIZE)

#define NODE_LEAF_BODY_SIZE                     (NODE_LEAF_CELL_SIZE * NODE_LEAF_NUM_CELLS)


//#define PAGE_SIZE                       (NODE_HEADER_SIZE + NODE_LEAF_BODY_SIZE)


#define NAME_LENGTH     64
#define DESCR_LENGTH    256


/* Questa struttura e' un B+Tree. Consente di avere un albero sempre equilibrato.
 * Rispetto a un BTree normale il B+Tree ha nelle foglie tutti i valori.
 * Ogni nodo e' una pagina. I nodi interni conterranno chiavi e riferimenti ai nodi sottostanti,
 * mentre le foglie conterranno coppie di chiave-valore. 
 */
typedef struct
{
    void**    nodes;
    uint32_t  size_pages;
    uint32_t  num_pages;
    uint32_t  root_node;
} bplustree;

typedef struct
{
    bplustree*  data;
    uint32_t    node;
    uint32_t    cell;
} Cursor;

typedef struct Row
{
    uint32_t  id;
    char      name[NAME_LENGTH];
    char      descr[DESCR_LENGTH];
} Row;

typedef enum { LEAF_NODE, INTERNAL_NODE }node_type;
typedef enum { INSERT_SUCCESS, DUPLICATE_KEY }ExecuteResult;



/* Metodi per le operazioni di r/w sulle pagine. */

/* [ ID | IS_ROOT | TYPE | PARENT_NODE | NUM_CELLS | RIGHTCHILD ] */

NODE_NUM_TYPE*  node_id(void* node);                                            // 75%
uint8_t         is_root_node(void* node);                                       // 75%
void            set_root_node(void* node, uint8_t root);                        // 75%
node_type       get_node_type(void* node);                                      // 75%
void            set_node_type(void* node, node_type type);                      // 75%
NODE_NUM_TYPE*  node_parent(void* node);                                        // 75%
NODE_NUM_TYPE*  node_num_cells(void* node);                                     // 75%
NODE_NUM_TYPE*  node_rightchild(void* node);                                    // 75%

/* [ LEFT_CHILD | KEY ] ... */

void*           internal_node_cell(void* node, uint32_t cell);                  // 75%
NODE_NUM_TYPE*  internal_node_leftchild(void* node, uint32_t cell);             // 75%
NODE_KEY_TYPE*  internal_node_key(void* node, uint32_t cell);                   // 75%
NODE_NUM_TYPE*  internal_node_rightchild(void* node, uint32_t cell);            // 50%

/* [ KEY | VALUE ] ... */

void*           leaf_node_cell(void* node, uint32_t cell);                                              // 75%
NODE_KEY_TYPE*  leaf_node_key(void* node, uint32_t cell);                                               // 75%
NODE_NUM_TYPE*  leaf_node_value(void* node, uint32_t cell);                                             // 75%

///////////////////////////////////////////////////////////////////////////////

/* Returns the cell number of the value searched.
 * If is equal the cell will be the same.
 * If less then the cell is the most near. (if 0 it's impossibile to return -1, so it will be 0).
 * If gr the cell is the next. */
NODE_NUM_TYPE   internal_node_binary_search(void* node, uint32_t key);                                                              // 0%
void            internal_node_insert_key(void* node, uint32_t cell, uint32_t key);                                                  // 0%
void            split_internal(bplustree* data, uint32_t new_internal, uint32_t node_num);                                          // 0%

NODE_NUM_TYPE   leaf_node_binary_search(void* node, uint32_t key);                                                                  // 0%

void            leaf_node_insert_key(void* node, uint32_t cell, uint32_t key);                                                      // 0%
void            split_leaf(bplustree* data, uint32_t new_leaf_num, uint32_t old_leaf_num);                                          // 0%
void            create_root(bplustree* data, uint32_t num_new, uint32_t num_old, uint32_t min_key_right);                           // 100%

void            update_parent(bplustree* data, uint32_t old_leaf);                                                                  // 50%
void            update_childs(bplustree* data, uint32_t new_internal);                                                              // 75%

// 00% - non capisco quando questa funzione realmente serva.
void update_key_parent(bplustree* data, uint32_t node_num, uint32_t last_min_key, uint32_t min_key);                                // 0%

// 0%
void award_new_min_key(bplustree* data, uint32_t parent, uint32_t old_num, const uint32_t new_num, const uint32_t key_to_award);    // 0%

///////////////////////////////////////////////////////////////////////////////
// More external interfaces ///////////////////////////////////////////////////

void            bplustree_init(bplustree* data);                                         // 97%
uint32_t        get_unused_node(bplustree* data);                                               // 97%
void*           get_node(bplustree* data, uint32_t num_page);                                   // 65%
ExecuteResult   insert(bplustree* data, uint32_t key, const void* row_buf, size_t size);        // 0%

void            initialize_node(void* node, uint32_t id, node_type type);                       // 97%

ExecuteResult   leaf_insert(Cursor* cur, uint32_t key, const void* row_buf, size_t size);       // 80%

void            save_db(bplustree* data, const char* file);




///////////////////////////////////////////////////////////////////////////////
// Cursor /////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Cursor* new_cursor(bplustree* data);                                        // 97%
void    cursor_find(Cursor* cur, uint32_t key);                             // 0%

///////////////////////////////////////////////////////////////////////////////
// Row ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void serialize_row(Row* row, uint32_t id, char name[], char descr[]);
void deserialize_row(Row* row, uint32_t id, char* name, size_t sname, char descr, size_t sdescr);


#endif /* __BPLUSTREE_H */
