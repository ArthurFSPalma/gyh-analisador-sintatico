#ifndef LEITOR_ARQUIVO_H
#define LEITOR_ARQUIVO_H

#include <fstream>
#include <stdexcept>
#include <string>

class LeitorArquivo {
private:
    std::ifstream arquivo;
    std::string caminho;
    int proximaLinha;
    int linhaUltimoCaracter;
    int ultimoCaracter;
    bool existeCaracterDevolvido;
    bool podeDevolver;
    bool erroLeitura;

public:
    explicit LeitorArquivo(const std::string& caminhoArquivo)
        : arquivo(caminhoArquivo, std::ios::binary),
          caminho(caminhoArquivo),
          proximaLinha(1),
          linhaUltimoCaracter(1),
          ultimoCaracter(std::char_traits<char>::eof()),
          existeCaracterDevolvido(false),
          podeDevolver(false),
          erroLeitura(false) {
        if (!arquivo.is_open()) {
            throw std::runtime_error(
                "Erro ao abrir o arquivo: \"" + caminhoArquivo + "\""
            );
        }
    }

    int lerProxCaracter() {
        int caractere;

        if (existeCaracterDevolvido) {
            caractere = ultimoCaracter;
            existeCaracterDevolvido = false;
        } else {
            caractere = arquivo.get();

            if (caractere == std::char_traits<char>::eof()) {
                erroLeitura = arquivo.bad();
                podeDevolver = false;
                return std::char_traits<char>::eof();
            }

            // Normaliza CRLF e CR para uma unica quebra de linha.
            if (caractere == '\r') {
                if (arquivo.peek() == '\n') {
                    arquivo.get();
                }
                caractere = '\n';
            }

            ultimoCaracter = caractere;
        }

        linhaUltimoCaracter = proximaLinha;
        if (caractere == '\n') {
            proximaLinha++;
        }

        podeDevolver = true;
        return caractere;
    }

    void devolverCaracter() {
        if (!podeDevolver || existeCaracterDevolvido) {
            return;
        }

        if (ultimoCaracter == '\n') {
            proximaLinha--;
        }

        existeCaracterDevolvido = true;
        podeDevolver = false;
    }

    int getLinhaAtual() const {
        return proximaLinha;
    }

    int getLinhaUltimoCaracter() const {
        return linhaUltimoCaracter;
    }

    bool houveErroLeitura() const {
        return erroLeitura;
    }

    const std::string& getCaminho() const {
        return caminho;
    }
};

#endif
