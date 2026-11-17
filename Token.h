#ifndef TOKEN_H
#define TOKEN_H

#include <iomanip>
#include <sstream>
#include <string>

#include "TipoToken.h"

class Token {
private:
    TipoToken tipo;
    std::string lexema;
    int id;
    int linha;

public:
    Token(TipoToken tipo, const std::string& lexema, int id, int linha)
        : tipo(tipo), lexema(lexema), id(id), linha(linha) {}

    TipoToken getTipo() const {
        return tipo;
    }

    const std::string& getLexema() const {
        return lexema;
    }

    int getId() const {
        return id;
    }

    int getLinha() const {
        return linha;
    }

    static std::string escapar(const std::string& texto) {
        std::ostringstream saida;

        for (unsigned char caractere : texto) {
            switch (caractere) {
                case '\\': saida << "\\\\"; break;
                case '"': saida << "\\\""; break;
                case '\t': saida << "\\t"; break;
                case '\n': saida << "\\n"; break;
                default:
                    if (caractere >= 32 && caractere <= 126) {
                        saida << static_cast<char>(caractere);
                    } else {
                        saida << "\\x"
                              << std::uppercase << std::hex
                              << std::setw(2) << std::setfill('0')
                              << static_cast<int>(caractere)
                              << std::dec;
                    }
            }
        }

        return saida.str();
    }

    std::string toString() const {
        if (tipo == TipoToken::FimArquivo) {
            return "<EOF>";
        }

        return "<" + nomeTipoToken(tipo) + ", \"" + escapar(lexema) + "\">";
    }
};

#endif
