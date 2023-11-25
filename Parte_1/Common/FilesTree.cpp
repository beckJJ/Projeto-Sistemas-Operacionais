#include "FilesTree.hpp"
#include "defines.hpp"
#include <cstdlib>
#include <stack>
#include <iostream>
#include <libgen.h>
#include <dirent.h>

BaseNode::BaseNode() {}

BaseNode::BaseNode(off_t size, time_t mtime, time_t atime, time_t ctime, const char _name[MAX_NAME_SIZE])
    : size(size), mtime(mtime), atime(atime), ctime(ctime)
{
    strncpy(name, _name, MAX_NAME_SIZE);
}

BaseNode::~BaseNode() {}

DirNode::DirNode() {}

int DirNode::add_child(Node *node)
{
    // Se já existe nodo de mesmo nome não adiciona
    if (get_child(node->name))
    {
        return ERROR_ALREADY_EXIST;
    }

    children.push_back(node);

    return SUCCESS;
}

Node *DirNode::get_child(const char *children_name)
{
    for (auto child : children)
    {
        if (!strncasecmp(child->name, children_name, MAX_NAME_SIZE))
        {
            return child;
        }
    }

    return NULL;
}

void DirNode::remove_child(const char *childr_name)
{
    // Não usamos get_child pois precisamos do índice para remover o item
    for (size_t i = 0; i < children.size(); i++)
    {
        if (!strncasecmp(children[i]->name, childr_name, MAX_NAME_SIZE))
        {
            delete children[i];
            children.erase(children.begin() + i);
            return;
        }
    }
}

void DirNode::remove_all_children(void)
{
    for (auto child : children)
    {
        delete child;
    }

    children.clear();
}

DirNode::~DirNode()
{
    remove_all_children();
}

FilesTree::FilesTree() {}

void FilesTree::clear_root(void)
{
    root.internalNode.dir.remove_all_children();
}

int FilesTree::build_from_fs(const char *path)
{
    // first -> path em FilesTree
    // second -> path no sistema de arquivos
    std::stack<std::pair<std::string, std::string>> dir_to_walk;

    struct dirent *entrada;
    struct stat info;

    dir_to_walk.push(std::make_pair(std::string(""), std::string(path)));

    // Enquanto houver diretórios para caminhar
    while (dir_to_walk.size())
    {
        std::string ft_path = dir_to_walk.top().first;
        std::string fs_path = dir_to_walk.top().second;

        dir_to_walk.pop();

        DIR *dir = opendir(fs_path.c_str());

        if (!dir)
        {
            std::cerr << "Erro ao abrir o diretorio: " << fs_path << ", ignorando." << std::endl;
            continue;
        }

        // Lê entradas do diretório
        while ((entrada = readdir(dir)) != NULL)
        {
            // Ignora . e ..
            if (!strcmp(entrada->d_name, ".") || !strcmp(entrada->d_name, ".."))
            {
                continue;
            }

            // Gera path dos arquivos/diretórios
            std::string new_fs_path = fs_path;
            std::string new_ft_path = ft_path;

            new_ft_path.append("/");
            new_ft_path.append(entrada->d_name);
            new_fs_path.append("/");
            new_fs_path.append(entrada->d_name);

            // Obtém informações sobre o arquivo/diretório
            if (stat(new_fs_path.c_str(), &info) == 0)
            {
                switch (entrada->d_type)
                {
                case DT_REG:
                    // Adiciona nodo de arquivo
                    add_node(ft_path.c_str(), FILE_TYPE, info.st_size, info.st_mtime, info.st_atime, info.st_ctime, entrada->d_name);
                    break;
                case DT_DIR:
                    // Adiciona nodo de diretório
                    add_node(ft_path.c_str(), DIR_TYPE, info.st_size, info.st_mtime, info.st_atime, info.st_ctime, entrada->d_name);

                    // Diretório deve ser scaneado depois
                    dir_to_walk.push(std::make_pair(new_ft_path, new_fs_path));

                    break;
                default:
                    std::cerr << "Arquivo " << new_fs_path << " nao eh arquivo regular ou diretorio, ignorando." << std::endl;
                }
            }
            else
            {
                std::cerr << "Erro ao tentar obter stat de " << new_fs_path << std::endl;
            }
        }
    }

    return SUCCESS;
}

int FilesTree::add_node(const char *path, NodeType type, off_t size, time_t mtime, time_t atime, time_t ctime, const char name[256])
{
    Node *node = get_node_at(path);

    if (node == NULL)
    {
        return ERROR_NOT_FOUND;
    }

    switch (node->type)
    {
    case FILE_TYPE:
        // Só pode adicionar arquivos em diretorios
        return ERROR_NODE_IS_FILE;
    case DIR_TYPE:
        // Cria novo novo para adicioná-lo
        Node *newNode = NULL;

        switch (type)
        {
        case FILE_TYPE:
            newNode = new Node(FileNode(), node, size, mtime, atime, ctime, name);
            break;
        case DIR_TYPE:
            newNode = new Node(DirNode(), node, size, mtime, atime, ctime, name);
            break;
        }

        return node->internalNode.dir.add_child(newNode);
    }

    return SUCCESS;
}

Node *FilesTree::get_node_at(const char *path)
{
    char *_path = strdup(path);
    const char *delim = "/";
    char *tok = NULL;
    Node *current_node = &root;

    // Caminha da raíz por '/'s
    while ((tok = strtok(_path, delim)))
    {
        _path = NULL;

        // TODO: Processar .. corretamente ou continuar ignorando? Teria algum acesso com ..?
        // Ignora strings vazia, mesmo diretorio e diretorio anterior
        if (!strcmp(tok, "") || !strcmp(tok, ".") || !strcmp(tok, ".."))
        {
            continue;
        }

        switch (current_node->type)
        {
        case FILE_TYPE:
            // Ainda há path para ser processado, porém nodo atual é um arquivo
            free(_path);
            return NULL;
        case DIR_TYPE:
            current_node = current_node->internalNode.dir.get_child(tok);

            // Não encontrou arquivo no diretorio
            if (!current_node)
            {
                free(_path);
                return NULL;
            }

            break;
        }
    }

    free(_path);

    // Retorna nodo encontrado
    return current_node;
}

int FilesTree::remove_node(const char *path)
{
    Node *node = get_node_at(path);

    if (!node)
    {
        return ERROR_NOT_FOUND;
    }

    // Atualmente está no nodo root, não é possível removê-lo
    if (!node->parent)
    {
        return ERROR_REMOVE_ROOT;
    }

    // Por definição apenas diretórios podem ter filhos
    DirNode *parent = &(node->parent->internalNode.dir);

    parent->remove_child(node->name);

    return SUCCESS;
}

void FilesTree::print_tree(void)
{
    // first -> DirNode to walk
    // second -> prefix do caminho
    std::stack<std::pair<DirNode *, std::string>> dir_to_walk;

    dir_to_walk.push(std::make_pair(&root.internalNode.dir, std::string("")));

    std::cout << "Tipo\t";
    printf("%-50.50s\t", "Path");
    std::cout << "Size\tM-time\t\t\tA-time\t\t\tC-time" << std::endl;

    // Enquanto houver diretórios para visitar
    while (dir_to_walk.size())
    {
        DirNode *current_dir = dir_to_walk.top().first;
        std::string prefix = dir_to_walk.top().second;

        dir_to_walk.pop();

        // Exibe todos os filhos do diretório atual, diretórios serão adicionados na pilha para serem visitados
        for (auto child : current_dir->children)
        {
            std::string child_path = prefix;
            child_path.append("/");
            child_path.append(child->name);

            std::cout << (child->type == FILE_TYPE ? "F\t" : "D\t");
            // Limita tamanho do path sendo exibido
            printf("%-50.50s\t", child_path.c_str());
            std::cout << child->size << "\t";
            print_formatted_time(&(child->mtime));
            std::cout << "\t";
            print_formatted_time(&(child->atime));
            std::cout << "\t";
            print_formatted_time(&(child->ctime));
            std::cout << std::endl;

            if (child->type == DIR_TYPE)
            {
                dir_to_walk.push(std::make_pair(&(child->internalNode.dir), child_path));
            }
        }
    }
}

void FilesTree::print_formatted_time(time_t *time)
{
    char buffer[BUFFER_SIZE];

    strftime(buffer, BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", localtime(time));

    std::cout << buffer;
}

std::vector<FilesTreeFlat> FilesTree::get_flatten_tree(void)
{
    // A lista de instruções é gerada usando depth-first

    // Diretórios e seus índices em que devem ser visitados
    std::stack<DirNodeIndex> dir_to_walk;
    // Instruções para reconstruir a árvore
    std::vector<FilesTreeFlat> flat;

    // Adiciona nodo inicial na pilha
    dir_to_walk.push((DirNodeIndex){
        .node = &root.internalNode.dir,
        .index = 0});

    // Enquanto houver diretórios não visitados
    while (dir_to_walk.size())
    {
        // O for interno pode terminar antes ao encontrar um diretório que deva ser visitado, então
        //   pode não ser necessário emitir um comando POP ao final do while
        bool ignore_pop = false;

        // Diretório atual
        DirNodeIndex current_dir = dir_to_walk.top();
        dir_to_walk.pop();

        // Visita todos os filhos do diretório atual
        for (size_t i = current_dir.index; i < current_dir.node->children.size(); i++)
        {
            auto child = current_dir.node->children[i];

            // Adiciona nodo na lista de instruções
            flat.push_back(
                FilesTreeFlat(
                    child->size,
                    child->mtime,
                    child->atime,
                    child->ctime,
                    child->name,
                    child->type,
                    // Ao encontrar um diretório, ele será explorado imeadiatamente, então emitidos
                    //   PUSH para diretórios
                    (child->type == FILE_TYPE ? CONTINUE : PUSH)));

            // Armazena o progresso atual na pilha e no topo ficará o diretório recém encontrado
            if (child->type == DIR_TYPE)
            {
                dir_to_walk.push((DirNodeIndex){
                    .node = current_dir.node,
                    .index = i + 1});
                dir_to_walk.push((DirNodeIndex){
                    .node = &(child->internalNode.dir),
                    .index = 0});
                // Indica que não deve ser gerada intrução POP, visto que ainda não terminados de
                //   explorar o diretório atual
                ignore_pop = true;
                break;
            }
        }

        // Emite comando POP caso necessário
        if (!ignore_pop)
        {
            flat.push_back(FilesTreeFlat(0, 0, 0, 0, "", FILE_TYPE, POP));
        }
    }

    return flat;
}

int FilesTree::build_from_flat(struct FilesTreeFlat *flat)
{
    std::stack<std::string> prefixes;
    std::string prefix("");

    prefixes.push(prefix);

    // Critério de parada é dar POP em root
    while (prefixes.size())
    {
        // Apenas POP não adiciona nodos
        if (flat->action == PUSH || flat->action == CONTINUE)
        {
            add_node(prefix.c_str(), flat->type, flat->size, flat->mtime, flat->atime, flat->ctime, flat->name);
        }

        if (flat->action == PUSH)
        {
            // PUSH altera o prefixo para conter o nome do diretorio recém adicionado
            std::string prefix_copy = prefix;
            prefixes.push(prefix_copy);

            prefix.append("/");
            prefix.append(flat->name);
        }

        if (flat->action == POP)
        {
            // POP retrocede ao diretorio pai
            prefix = prefixes.top();
            prefixes.pop();
        }

        // Avança para próxima instrução
        flat++;
    }

    return 0;
}

FilesTree::~FilesTree() {}
