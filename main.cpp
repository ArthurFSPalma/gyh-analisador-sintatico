// AP2 - Analisador Sintatico GYH - Maria Eduarda Cardoso de Oliveira (RA 2533880) e Arthur Francisco da Silva Palma (RA 2369060)

#include <exception>
#include <iostream>

#include "Erros.h"
#include "Lexico.h"
#include "Sintatico.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo-fonte>" << std::endl;
        return 1;
    }

    try {
        Lexico lexico(argv[1]);
        Erros erros;
        Sintatico sintatico(lexico, erros);

        sintatico.analisar();

        if (erros.vazio()) {
            std::cout << "Analise concluida: nenhum erro encontrado." << std::endl;
            return 0;
        }

        erros.imprimir(std::cout);
        std::cout << erros.total() << " erro(s) encontrado(s)." << std::endl;
        return 3;
    } catch (const std::exception& erro) {
        std::cerr << erro.what() << std::endl;
        return 2;
    }
}
