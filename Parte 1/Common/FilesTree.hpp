#ifndef _FILES_TREE_H_
#define _FILES_TREE_H_

#include <string.h>
#include <string>
#include <cstdint>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <vector>

// Buffer usado para formatação de tempo com strftime
#define BUFFER_SIZE     80
// Tamanho máximo de um nodo
#define MAX_NAME_SIZE   256

// Erros
#define ERROR_NOT_FOUND     1
#define ERROR_ALREADY_EXIST 2
#define ERROR_REMOVE_ROOT   3
#define ERROR_NODE_IS_FILE  4

// status&NODE_EQUALNESS_MASK, o nodo é igual ou há diferenças
#define NODE_EQUALNESS_MASK 0b00000001
#define NODE_EQUAL          0b00000000
#define NODE_DIFFERENT      0b00000001

// status&NODE_EXISTANCE_MASK, diferença na existência do nodo
#define NODE_EXISTANCE_MASK 0b00000110
#define NODE_SAME           0b00000000
#define NODE_MISSING        0b00000010
#define NODE_EXTRA          0b00000100
#define NODE_CHANGE_TYPE    0b00000110

// status&NODE_MAC_MASK, diferença em MAC time
#define NODE_MAC_MASK       0b00011000
#define NODE_MAC_SAME       0b00000000
#define NODE_MAC_OLDER      0b00001000
#define NODE_MAC_NEWER      0b00010000

// status&NODE_SIZE_MASK, há diferença de tamanho?
#define NODE_SIZE_MASK      0b00100000
#define NODE_SIZE_SAME      0b00000000
#define NODE_SIZE_DIFFERENT 0b00100000

// Um nodo pode ser arquivo ou diretório
enum NodeType {
    FILE_TYPE,
    DIR_TYPE,
};

// Comandos para a reconstrução da árvore de arquivos
enum FilesTreeFlatAction {
    PUSH,
    POP,
    CONTINUE,
};

class Node;

// Nodo base, define campos comuns para arquivos e diretórios
class BaseNode {
public:
    // Tamanho do arquivo/nodo
    off_t size = 0;
    // MAC-time
    time_t mtime = 0;
    time_t atime = 0;
    time_t ctime = 0;
    // Nome do nodo
    char name[MAX_NAME_SIZE] { };

    BaseNode();
    BaseNode(off_t size, time_t mtime, time_t atime, time_t ctime, const char _name[MAX_NAME_SIZE]);
    ~BaseNode();
};

// Nodo de diretório
class DirNode {
public:
    // Filhos do diretório
    std::vector<Node *> children { };

    DirNode();
    ~DirNode();

    // Funções para adição, remoção e pesquisa de filhos

    int add_child(Node *node);
    Node *get_child(const char *children_name);
    void remove_child(const char *child_name);
    void remove_all_children(void);
};

// Nodo de arquivo
class FileNode { };

// Nodo da árvore, pode ser arquivo ou diretório, checar campo type
class Node: public BaseNode {
public:
    // Tipo do nodo atual
    NodeType type;
    // Nodo raíz não tem pai
    Node *parent = NULL;

    // Dados internos dependentes do tipo do nodo
    union InternalNode {
        FileNode file;
        DirNode dir;

        InternalNode(FileNode file): file(file) { }
        InternalNode(DirNode dir): dir(dir) { }
        ~InternalNode() { }
    } internalNode;

    Node(FileNode node) : BaseNode(), type(FILE_TYPE), internalNode(node) { }
    Node(FileNode node, Node *parent, off_t size, time_t mtime, time_t atime, time_t ctime, const char _name[MAX_NAME_SIZE])
        : BaseNode(size, mtime, atime, ctime, _name), type(FILE_TYPE), parent(parent), internalNode(node) { }

    Node(DirNode node) : BaseNode(), type(DIR_TYPE), internalNode(node) { }
    Node(DirNode node, Node *parent, off_t size, time_t mtime, time_t atime, time_t ctime, const char _name[MAX_NAME_SIZE])
        : BaseNode(size, mtime, atime, ctime, _name), type(DIR_TYPE), parent(parent), internalNode(node) { }
};

// Usado durante a construção da versão "flat" da árvore de arquivos
struct DirNodeIndex {
    DirNode *node;
    size_t index;
};

// Nodo da versão flat, determina ações para reconstruir a árvore original
struct FilesTreeFlat: public BaseNode {
    // Tipo do nodo
    NodeType type;
    // Ação que deve ser tomada
    FilesTreeFlatAction action;

    FilesTreeFlat(off_t size, time_t mtime, time_t atime, time_t ctime, const char _name[MAX_NAME_SIZE], NodeType type, FilesTreeFlatAction action)
        : BaseNode(size, mtime, atime, ctime, _name), type(type), action(action) { }
};

// Árvore de arquivos
class FilesTree
{
private:
    // Nodo raíz não tem informação, é apenas um diretório
    Node root = Node(DirNode());

    Node *get_node_at(const char *path);
    void print_formatted_time(time_t *time);

public:
    FilesTree();

    // Remove todos os filhos do nodo raíz, limpando a árvore
    void clear_root(void);
    // Constrói árvore de arquivos apartir de um path do sistema
    int build_from_fs(const char *path);
    // Adiciona um nodo em path com nome name
    int add_node(const char *path, NodeType type, off_t size, time_t mtime, time_t atime, time_t ctime, const char name[MAX_NAME_SIZE]);
    // Remove nodo qualquer
    int remove_node(const char *path);
    // Exibe árvore de arquivos em stdout
    void print_tree(void);
    // Obtém vetor contendo instruções flat para gerar a árvore atualmente armazenada
    std::vector<FilesTreeFlat> get_flatten_tree(void);
    // Constrói árvore de arquivos apartir de uma sequência de instruções flat
    int build_from_flat(FilesTreeFlat *flat);

    ~FilesTree();
};

#endif
