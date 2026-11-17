#ifndef LEXICO_H
#define LEXICO_H

#include <stdexcept>
#include <string>

#include "LeitorArquivo.h"
#include "Token.h"

class ErroLexico : public std::runtime_error {
private:
    int linha;
    std::string lexema;

public:
    ErroLexico(int linha, const std::string& descricao, const std::string& lexema)
        : std::runtime_error(
              "Erro lexico na linha " + std::to_string(linha) + ": " +
              descricao + " \"" + Token::escapar(lexema) + "\""
          ),
          linha(linha),
          lexema(lexema) {}

    int getLinha() const {
        return linha;
    }

    const std::string& getLexema() const {
        return lexema;
    }
};

class Lexico {
private:
    LeitorArquivo arq;
    int proximoId;
    bool fimEmitido;

    static bool eLetraMinuscula(int caractere) {
        return caractere >= 'a' && caractere <= 'z';
    }

    static bool eLetraMaiuscula(int caractere) {
        return caractere >= 'A' && caractere <= 'Z';
    }

    static bool eLetra(int caractere) {
        return eLetraMinuscula(caractere) || eLetraMaiuscula(caractere);
    }

    static bool eDigito(int caractere) {
        return caractere >= '0' && caractere <= '9';
    }

    static bool eEspaco(int caractere) {
        return caractere == ' ' || caractere == '\t' || caractere == '\n' ||
               caractere == '\v' || caractere == '\f';
    }

    int ler() {
        int caractere = arq.lerProxCaracter();

        if (caractere == std::char_traits<char>::eof() && arq.houveErroLeitura()) {
            throw std::runtime_error(
                "Erro de leitura no arquivo: \"" + arq.getCaminho() + "\""
            );
        }

        return caractere;
    }

    void devolverSeNaoForFim(int caractere) {
        if (caractere != std::char_traits<char>::eof()) {
            arq.devolverCaracter();
        }
    }

    Token* criarToken(TipoToken tipo, const std::string& lexema, int linha) {
        return new Token(tipo, lexema, proximoId++, linha);
    }

    static bool obterTipoPalavra(const std::string& lexema, TipoToken& tipo) {
        if (lexema == "DEC") tipo = TipoToken::PCDec;
        else if (lexema == "PROG") tipo = TipoToken::PCProg;
        else if (lexema == "INT") tipo = TipoToken::PCInt;
        else if (lexema == "REAL") tipo = TipoToken::PCReal;
        else if (lexema == "LER") tipo = TipoToken::PCLer;
        else if (lexema == "IMPRIMIR") tipo = TipoToken::PCImprimir;
        else if (lexema == "SE") tipo = TipoToken::PCSe;
        else if (lexema == "ENTAO") tipo = TipoToken::PCEntao;
        else if (lexema == "SENAO") tipo = TipoToken::PCSenao;
        else if (lexema == "ENQTO") tipo = TipoToken::PCEnqto;
        else if (lexema == "INI") tipo = TipoToken::PCIni;
        else if (lexema == "FIM") tipo = TipoToken::PCFim;
        else if (lexema == "E") tipo = TipoToken::OpBoolE;
        else if (lexema == "OU") tipo = TipoToken::OpBoolOu;
        else return false;

        return true;
    }

    Token* reconhecerPalavra(int primeiro, int linha) {
        std::string lexema(1, static_cast<char>(primeiro));
        int caractere = ler();

        while (eLetra(caractere) || eDigito(caractere)) {
            lexema += static_cast<char>(caractere);
            caractere = ler();
        }

        devolverSeNaoForFim(caractere);

        TipoToken tipo;
        if (obterTipoPalavra(lexema, tipo)) {
            return criarToken(tipo, lexema, linha);
        }

        if (eLetraMinuscula(primeiro)) {
            return criarToken(TipoToken::Var, lexema, linha);
        }

        throw ErroLexico(linha, "Desconhecido", lexema);
    }

    Token* reconhecerNumero(int primeiro, int linha) {
        std::string lexema(1, static_cast<char>(primeiro));
        int caractere = ler();

        while (eDigito(caractere)) {
            lexema += static_cast<char>(caractere);
            caractere = ler();
        }

        if (caractere != '.') {
            devolverSeNaoForFim(caractere);
            return criarToken(TipoToken::NumInt, lexema, linha);
        }

        lexema += '.';
        caractere = ler();

        while (eDigito(caractere)) {
            lexema += static_cast<char>(caractere);
            caractere = ler();
        }

        devolverSeNaoForFim(caractere);
        return criarToken(TipoToken::NumReal, lexema, linha);
    }

    Token* reconhecerCadeia(int linha) {
        std::string lexema(1, '"');

        while (true) {
            int caractere = ler();

            if (caractere == std::char_traits<char>::eof()) {
                throw ErroLexico(linha, "Cadeia nao finalizada", lexema);
            }

            if (caractere == '\n') {
                throw ErroLexico(linha, "Cadeia nao finalizada", lexema);
            }

            lexema += static_cast<char>(caractere);

            if (caractere == '"') {
                return criarToken(TipoToken::Cadeia, lexema, linha);
            }
        }
    }

public:
    explicit Lexico(const std::string& arquivo)
        : arq(arquivo), proximoId(1), fimEmitido(false) {}

    Token* proximoToken() {
        if (fimEmitido) {
            return nullptr;
        }

        int caractere;

        // Etapa de scanning: ignora separadores e comentarios.
        while (true) {
            caractere = ler();

            if (caractere == std::char_traits<char>::eof()) {
                fimEmitido = true;
                return criarToken(TipoToken::FimArquivo, "", arq.getLinhaAtual());
            }

            if (eEspaco(caractere)) {
                continue;
            }

            if (caractere == '#') {
                do {
                    caractere = ler();
                } while (caractere != '\n' &&
                         caractere != std::char_traits<char>::eof());
                continue;
            }

            break;
        }

        int linha = arq.getLinhaUltimoCaracter();

        if (eLetra(caractere)) {
            return reconhecerPalavra(caractere, linha);
        }

        if (eDigito(caractere)) {
            return reconhecerNumero(caractere, linha);
        }

        switch (caractere) {
            case '+': return criarToken(TipoToken::OpAritSoma, "+", linha);
            case '-': return criarToken(TipoToken::OpAritSub, "-", linha);
            case '*': return criarToken(TipoToken::OpAritMult, "*", linha);
            case '/': return criarToken(TipoToken::OpAritDiv, "/", linha);
            case '(': return criarToken(TipoToken::AbrePar, "(", linha);
            case ')': return criarToken(TipoToken::FechaPar, ")", linha);
            case '"': return reconhecerCadeia(linha);

            case ':': {
                int proximo = ler();
                if (proximo == '=') {
                    return criarToken(TipoToken::Atrib, ":=", linha);
                }
                devolverSeNaoForFim(proximo);
                return criarToken(TipoToken::Delim, ":", linha);
            }

            case '<': {
                int proximo = ler();
                if (proximo == '=') {
                    return criarToken(TipoToken::OpRelMenorIgual, "<=", linha);
                }
                devolverSeNaoForFim(proximo);
                return criarToken(TipoToken::OpRelMenor, "<", linha);
            }

            case '>': {
                int proximo = ler();
                if (proximo == '=') {
                    return criarToken(TipoToken::OpRelMaiorIgual, ">=", linha);
                }
                devolverSeNaoForFim(proximo);
                return criarToken(TipoToken::OpRelMaior, ">", linha);
            }

            case '=': {
                int proximo = ler();
                if (proximo == '=') {
                    return criarToken(TipoToken::OpRelIgual, "==", linha);
                }
                devolverSeNaoForFim(proximo);
                throw ErroLexico(linha, "Desconhecido", "=");
            }

            case '!': {
                int proximo = ler();
                if (proximo == '=') {
                    return criarToken(TipoToken::OpRelDif, "!=", linha);
                }
                devolverSeNaoForFim(proximo);
                throw ErroLexico(linha, "Desconhecido", "!");
            }

            default:
                throw ErroLexico(
                    linha,
                    "Desconhecido",
                    std::string(1, static_cast<char>(caractere))
                );
        }
    }
};

#endif
