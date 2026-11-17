#ifndef ERROS_H
#define ERROS_H

#include <cstddef>
#include <iostream>
#include <set>
#include <string>
#include <vector>

// Guarda, em ordem de ocorrencia, os erros lexicos e sintaticos encontrados.
// Lembra tambem em quais linhas houve erro lexico, para o parser decidir se
// suprime o erro sintatico que e apenas consequencia dele.
class Erros {
private:
    std::vector<std::string> mensagens;
    std::set<int> linhasComErroLexico;

public:
    void adicionarLexico(int linha, const std::string& mensagem) {
        linhasComErroLexico.insert(linha);
        mensagens.push_back(mensagem);
    }

    void adicionarSintatico(const std::string& mensagem) {
        mensagens.push_back(mensagem);
    }

    bool temErroLexicoNaLinha(int linha) const {
        return linhasComErroLexico.count(linha) > 0;
    }

    bool vazio() const {
        return mensagens.empty();
    }

    std::size_t total() const {
        return mensagens.size();
    }

    void imprimir(std::ostream& saida) const {
        for (std::size_t i = 0; i < mensagens.size(); i++) {
            saida << mensagens[i] << std::endl;
        }
    }
};

#endif
