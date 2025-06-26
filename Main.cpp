//Sistema de Arquivos como Árvore em C++
//Patric Federissi Vanin 202791, Robson Donin 202658

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <map>
#include <unistd.h> 

namespace fs = std::filesystem;

using namespace std;

//Estrutura que representa cada nó da árvore (arquivo ou diretório)
struct Node {
    string name;              //Nome do arquivo ou diretório
    string path;              //Caminho completo
    bool is_dir;              //Verdadeiro se for diretório
    uintmax_t size;           //Tamanho do arquivo ou acumulado dos filhos
    vector<Node*> children;   //Filhos do nó (apenas se for diretório)
};

//Função para calcular o tamanho total de uma pasta de forma recursiva
uintmax_t calcular_tamanho(Node* node) {    //uintmax_t é um tipo de dado inteiro sem sinal
    if (!node->is_dir) return node->size;
    uintmax_t total = 0;
    for (auto child : node->children) {
        total += calcular_tamanho(child);
    }
    node->size = total;
    return total;
}

//Função para carregar a estrutura do diretório de forma recursiva
Node* carregar_estrutura(const fs::path& path) {
    if (!fs::exists(path)) return nullptr;  //exists(path verificar se um arquivo ou diretório existe

    Node* node = new Node;
    node->name = path.filename().string();
    node->path = path.string();
    node->is_dir = fs::is_directory(path);
    node->size = 0;

    if (node->is_dir) {
        for (const auto& entry : fs::directory_iterator(path)) {
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

//Impressão recursiva da árvore com indentação
void exibir_arvore(Node* node, int nivel = 0) {
    if (!node) return;

    // Indentação
    for (int i = 0; i < nivel; ++i) cout << "│   ";

    if (node->is_dir) {
        cout << "├── " << node->name << " (" << node->children.size() << " filhos, " << node->size << " bytes)\n";
        for (auto child : node->children) exibir_arvore(child, nivel + 1);
    } else {
        cout << "├── " << node->name << " (" << node->size << " bytes)\n";
    }
}

//Exportação para HTML
void exportar_html(Node* node, ofstream& out, int nivel = 0) {
    if (!node) return;
    string indent(nivel * 4, ' ');

    out << indent << "<div>";
    if (node->is_dir) {
        out << node->name << " (" << node->children.size() << " filhos, " << node->size << " bytes)</div>\n";
        for (auto child : node->children) exportar_html(child, out, nivel + 1);
    } else {
        out << node->name << " (" << node->size << " bytes)</div>\n";
    }
}

//Função para encontrar o maior arquivo
void maior_arquivo(Node* node, uintmax_t& max_tam, vector<Node*>& maiores) {
    if (!node) return;
    if (!node->is_dir) {
        if (node->size > max_tam) {
            max_tam = node->size;
            maiores.clear(); maiores.push_back(node);
        } else if (node->size == max_tam) {
            maiores.push_back(node);
        }
    }
    for (auto child : node->children) maior_arquivo(child, max_tam, maiores);
}

// Arquivos maiores que N bytes
void arquivos_maiores_que(Node* node, uintmax_t n) {
    if (!node) return;
    if (!node->is_dir && node->size > n) {
        cout << node->path << " (" << node->size << " bytes)\n";
    }
    for (auto child : node->children) arquivos_maiores_que(child, n);
}

//Pasta com mais arquivos diretamente
void pasta_com_mais_arquivos(Node* node, Node*& resultado, size_t& max_filhos) {
    if (!node || !node->is_dir) return;
    if (node->children.size() > max_filhos) {
        max_filhos = node->children.size();
        resultado = node;
    }
    for (auto child : node->children) pasta_com_mais_arquivos(child, resultado, max_filhos);
}

//Buscar arquivos por extensão
void arquivos_por_extensao(Node* node, const string& ext) {
    if (!node) return;
    if (!node->is_dir && node->name.size() > ext.size() &&
        node->name.substr(node->name.size() - ext.size()) == ext) {
        cout << node->path << " (" << node->size << " bytes)\n";
    }
    for (auto child : node->children) arquivos_por_extensao(child, ext);
}

//Listar pastas vazias
void pastas_vazias(Node* node) {
    if (!node || !node->is_dir) return;
    if (node->children.empty()) {
        cout << node->path << "\n";
    }
    for (auto child : node->children) pastas_vazias(child);
}

//Menu principal
void menu(Node* raiz) {
    int op;
    do {
        cout << "\n===== MENU =====\n";
        cout << "1- Exibir Arvore\n";
        cout << "2- Exportar para HTML\n";
        cout << "3- Pesquisar\n";
        cout << "0- Sair\n> ";
        cin >> op;

        if (op == 1) {
            exibir_arvore(raiz);
        } else if (op == 2) {
            ofstream out("arvore.html");
            out << "<html><body><pre>\n";
            exportar_html(raiz, out);
            out << "</pre></body></html>";
            out.close();
            cout << "Arquivo 'arvore.html' gerado.\n";
        } else if (op == 3) {
            cout << "\n1. Maior arquivo\n";
            cout << "2. Arquivos > N bytes\n";
            cout << "3. Pasta com mais arquivos\n";
            cout << "4. Arquivos por extensao\n";
            cout << "5. Pastas vazias\n> ";
            int subop; cin >> subop;
            if (subop == 1) {
                uintmax_t max_tam = 0;
                vector<Node*> maiores;
                maior_arquivo(raiz, max_tam, maiores);
                for (auto n : maiores) cout << n->path << " (" << n->size << " bytes)\n";
            } else if (subop == 2) {
                uintmax_t n;
                cout << "Digite N: "; cin >> n;
                arquivos_maiores_que(raiz, n);
            } else if (subop == 3) {
                Node* resultado = nullptr;
                size_t max_filhos = 0;
                pasta_com_mais_arquivos(raiz, resultado, max_filhos);
                if (resultado) cout << resultado->path << " (" << resultado->children.size() << " filhos)\n";
            } else if (subop == 4) {
                string ext;
                cout << "Digite a extensao (ex: .txt): "; cin >> ext;
                arquivos_por_extensao(raiz, ext);
            } else if (subop == 5) {
                pastas_vazias(raiz);
            }
        }
    } while (op != 0);
}

//Função principal
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

/*
EXECUÇÃO
cd "c:\Users\PFV18\Downloads\Trabalho\Trabalho"

C:\TDM-GCC-64\bin\g++.exe -std=c++17 -Wall -Wextra -g3 main.cpp -o output\main.exe

.\output\main.exe
*/
