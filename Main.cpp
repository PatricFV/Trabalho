// Sistema de Arquivos como Árvore
// Patric Federissi Vanin 202791, Robsin Donoin 202658

#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <memory>
#include <algorithm>

using namespace std;

// Tipo para distinguir arquivos de diretórios
enum Tipo { ARQUIVO, PASTA };

// Estrutura de um nó da árvore
struct Node {
    string nome;
    string caminhoCompleto;
    Tipo tipo;
    long long tamanho;
    vector<shared_ptr<Node>> filhos;
};

// Função auxiliar para verificar se é um arquivo ou diretório regular
bool ehArquivoOuPasta(const struct stat& st) {
    return S_ISREG(st.st_mode) || S_ISDIR(st.st_mode);
}

// Função recursiva para construir a árvore
shared_ptr<Node> carregarArvore(const string& caminho) {
    struct stat info;
    if (stat(caminho.c_str(), &info) != 0 || !ehArquivoOuPasta(info))
        return nullptr;

    shared_ptr<Node> node = make_shared<Node>();
    node->nome = caminho.substr(caminho.find_last_of("/") + 1);
    node->caminhoCompleto = caminho;
    node->tamanho = S_ISREG(info.st_mode) ? info.st_size : 0;
    node->tipo = S_ISDIR(info.st_mode) ? PASTA : ARQUIVO;

    if (node->tipo == PASTA) {
        DIR* dir = opendir(caminho.c_str());
        if (!dir) return node;

        struct dirent* entrada;
        while ((entrada = readdir(dir)) != nullptr) {
            string nomeEntrada = entrada->d_name;
            if (nomeEntrada == "." || nomeEntrada == "..") continue;

            string caminhoFilho = caminho + "/" + nomeEntrada;
            shared_ptr<Node> filho = carregarArvore(caminhoFilho);
            if (filho) {
                node->tamanho += filho->tamanho;
                node->filhos.push_back(filho);
            }
        }
        closedir(dir);
    }

    return node;
}

