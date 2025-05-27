// Sistema de Arquivos como Árvore em C++
// Patric Federissi Vanin 202791, Robsin Donoin 202658

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <map>
#include <unistd.h> // Para getcwd

namespace fs = std::filesystem;

using namespace std;

// Estrutura que representa cada nó da árvore (arquivo ou diretório)
struct Node {
    string name;               // Nome do arquivo ou diretório
    string path;               // Caminho completo
    bool is_dir;              // Verdadeiro se for diretório
    uintmax_t size;           // Tamanho do arquivo ou acumulado dos filhos
    vector<Node*> children;   // Filhos do nó (apenas se for diretório)
};

// Função para calcular o tamanho total de uma pasta de forma recursiva
uintmax_t calcular_tamanho(Node* node) {
    if (!node->is_dir) return node->size;
    uintmax_t total = 0;
    for (auto child : node->children) {
        total += calcular_tamanho(child);
    }
    node->size = total;
    return total;
}

// Função para carregar a estrutura do diretório de forma recursiva
Node* carregar_estrutura(const fs::path& path) {
    if (!fs::exists(path)) return nullptr;

    Node* node = new Node;
    node->name = path.filename().string();
    node->path = path.string();
    node->is_dir = fs::is_directory(path);
    node->size = 0;

    if (node->is_dir) {
        for (const auto& entry : fs::directory_iterator(path)) {
            // Considerar apenas arquivos regulares e pastas
            if (fs::is_regular_file(entry) || fs::is_directory(entry)) {
                Node* child = carregar_estrutura(entry);
                if (child) node->children.push_back(child);
            }
        }
        calcular_tamanho(node);
    } else {
        node->size = fs::file_size(path);
    }

    return node;
}

// Impressão recursiva da árvore com indentação 
void exibir_arvore(Node* node, int nivel = 0) {
    if (!node) return;

    // Indentação
    for (int i = 0; i < nivel; ++i) cout << "│   ";

    if (node->is_dir) {
        // Para pastas exibir: Nome, Quantidade de filhos, Tamanho total acumulado
        cout << "├── " << node->name << " (" << node->children.size() << " filhos, " << node->size << " bytes)\n";
        for (auto child : node->children) exibir_arvore(child, nivel + 1);
    } else {
        // Para arquivos exibir: Nome e Tamanho em bytes
        cout << "├── " << node->name << " (" << node->size << " bytes)\n";
    }
}

void menu(Node* raiz) {
    int op;
    do {
        cout << "\n===== MENU =====\n";
        cout << "1. Exibir árvore\n"; 
        cout << "0. Sair\n> ";
        cin >> op;

        if (op == 1) {
            exibir_arvore(raiz);
        } else if (op != 0) {
            cout << "Opção inválida. Tente novamente.\n";
        }
    } while (op != 0);
}

// Função principal
int main(int argc, char* argv[]) {
    
    string path = (argc > 1) ? argv[1] : fs::current_path().string();
    cout << "Carregando estrutura de: " << path << "\n"; 
    Node* raiz = carregar_estrutura(path);
    if (!raiz) {
        cerr << "Erro ao carregar estrutura.\n";
        return 1;
    }
    menu(raiz); 
    return 0;
}
