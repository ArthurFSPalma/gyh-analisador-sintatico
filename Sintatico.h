#ifndef SINTATICO_H
#define SINTATICO_H

#include <memory>
#include <string>

#include "Erros.h"
#include "Lexico.h"
#include "Token.h"

/*
 * Analisador sintatico descendente recursivo, sem backtracking, para a GYH.
 * Um metodo por nao terminal da gramatica transformada (sem recursao a
 * esquerda e fatorada). Os nao terminais com linha (ExpA', TermoA', ListaCmd'
 * etc.) viraram lacos while.
 *
 * Gramatica implementada (sintaxe do slide, vocabulario do Lexico da AP1):
 *
 *   Programa   -> Delim PCDec ListaDecl Delim PCProg ListaCmd
 *   ListaDecl  -> Decl ListaDecl'            ListaDecl' -> Decl ListaDecl' | e
 *   Decl       -> Var Delim TipoVar
 *   TipoVar    -> PCInt | PCReal
 *   ListaCmd   -> Cmd ListaCmd'              ListaCmd'  -> Cmd ListaCmd' | e
 *   Cmd        -> CmdAtrib | CmdLer | CmdImp | CmdSe | CmdEnqto | CmdBloco
 *   CmdAtrib   -> Var Atrib ExpA
 *   CmdLer     -> PCLer Var
 *   CmdImp     -> PCImprimir (Var | Cadeia)
 *   CmdSe      -> PCSe ExpR PCEntao Cmd [PCSenao Cmd]
 *   CmdEnqto   -> PCEnqto ExpR [PCEntao] Cmd
 *   CmdBloco   -> PCIni ListaCmd PCFim
 *   ExpR       -> ExpRel ExpR'               ExpR'   -> OpBool ExpRel ExpR' | e
 *   ExpRel     -> ExpA ExpRel'               ExpRel' -> OpRel ExpA | e
 *   ExpA       -> TermoA ExpA'               ExpA'   -> (+|-) TermoA ExpA' | e
 *   TermoA     -> FatorA TermoA'             TermoA' -> (*|/) FatorA TermoA' | e
 *   FatorA     -> NumInt | NumReal | Var | AbrePar ExpR FechaPar
 *
 * O parenteses de FatorA contem uma ExpR (e nao so uma ExpA) para evitar o
 * conflito LL(1) entre "( ExpA )" e "( ExpR )" da gramatica original. Para
 * continuar aceitando exatamente a mesma linguagem, cada funcao de expressao
 * devolve o TIPO do que leu (aritmetica ou relacional) e o parser rejeita as
 * combinacoes que a gramatica original nao permite.
 */
class Sintatico {
private:
    // Pontos que dependem da resposta do professor.
    // O slide 5 mostra ENQTO sem ENTAO; o PDF da gramatica exige o then.
    static constexpr bool ENQTO_EXIGE_ENTAO = false;
    // Se verdadeiro, nao mostra erro sintatico em linha que ja teve erro
    // lexico (ele costuma ser so consequencia do token descartado).
    static constexpr bool SUPRIMIR_SINTATICO_EM_LINHA_COM_ERRO_LEXICO = true;

    enum class TipoExp { Aritmetica, Relacional };

    Lexico& lexico;
    Erros& erros;
    Token atual;
    int idUltimoErro;

    // ---------------------------------------------------------------
    // Utilitarios
    // ---------------------------------------------------------------

    TipoToken tipo() const {
        return atual.getTipo();
    }

    static bool ehOpRel(TipoToken t) {
        return t == TipoToken::OpRelMenor || t == TipoToken::OpRelMenorIgual ||
               t == TipoToken::OpRelMaior || t == TipoToken::OpRelMaiorIgual ||
               t == TipoToken::OpRelIgual || t == TipoToken::OpRelDif;
    }

    // FIRST(Cmd)
    static bool ehInicioComando(TipoToken t) {
        return t == TipoToken::Var || t == TipoToken::PCLer ||
               t == TipoToken::PCImprimir || t == TipoToken::PCSe ||
               t == TipoToken::PCEnqto || t == TipoToken::PCIni;
    }

    // Texto usado na parte "esperado ..." da mensagem de erro.
    static std::string descricao(TipoToken t) {
        switch (t) {
            case TipoToken::Delim: return "delimitador ':'";
            case TipoToken::Atrib: return "atribuicao ':='";
            case TipoToken::AbrePar: return "'('";
            case TipoToken::FechaPar: return "')'";
            case TipoToken::Var: return "variavel";
            case TipoToken::Cadeia: return "cadeia";
            case TipoToken::FimArquivo: return "fim de arquivo";
            default: return nomeTipoToken(t);
        }
    }

    // Le o proximo token do lexico. Erros lexicos sao registrados e o token
    // invalido e descartado: o lexico ja consumiu o lexema ofensor antes de
    // lancar a excecao, entao a proxima chamada continua do ponto certo.
    void avanca() {
        if (tipo() == TipoToken::FimArquivo) {
            return;
        }

        while (true) {
            try {
                std::unique_ptr<Token> proximo(lexico.proximoToken());
                if (proximo) {
                    atual = *proximo;
                } else {
                    atual = Token(TipoToken::FimArquivo, "", atual.getId() + 1,
                                  atual.getLinha());
                }
                return;
            } catch (const ErroLexico& e) {
                erros.adicionarLexico(e.getLinha(), e.what());
            }
        }
    }

    // Registra no maximo um erro por token, para evitar cascata de mensagens
    // sobre o mesmo ponto do programa.
    void erroEsperado(const std::string& esperado) {
        if (atual.getId() == idUltimoErro) {
            return;
        }
        idUltimoErro = atual.getId();

        if (SUPRIMIR_SINTATICO_EM_LINHA_COM_ERRO_LEXICO &&
            erros.temErroLexicoNaLinha(atual.getLinha())) {
            return;
        }

        erros.adicionarSintatico(
            "Erro sintatico na linha " + std::to_string(atual.getLinha()) +
            ": esperado " + esperado + " antes de " +
            nomeTipoToken(atual.getTipo())
        );
    }

    // Se o token e o esperado, consome. Senao, registra o erro e NAO avanca,
    // como se o token faltante tivesse sido inserido (recuperacao local).
    void consome(TipoToken esperado) {
        if (tipo() == esperado) {
            avanca();
        } else {
            erroEsperado(descricao(esperado));
        }
    }

    // Erro de tipo de expressao (aritmetica x relacional). Usa a linha do
    // inicio da expressao ofensora, que e onde o usuario precisa olhar.
    void erroTipo(const std::string& esperada, const std::string& encontrada,
                  int linha) {
        if (atual.getId() == idUltimoErro) {
            return;
        }
        idUltimoErro = atual.getId();

        if (SUPRIMIR_SINTATICO_EM_LINHA_COM_ERRO_LEXICO &&
            erros.temErroLexicoNaLinha(linha)) {
            return;
        }

        erros.adicionarSintatico(
            "Erro sintatico na linha " + std::to_string(linha) +
            ": esperado expressao " + esperada +
            ", encontrada expressao " + encontrada
        );
    }

    void exigeAritmetica(TipoExp t, int linha) {
        if (t != TipoExp::Aritmetica) {
            erroTipo("aritmetica", "relacional", linha);
        }
    }

    void exigeRelacional(TipoExp t, int linha) {
        if (t != TipoExp::Relacional) {
            erroTipo("relacional", "aritmetica", linha);
        }
    }

    // ---------------------------------------------------------------
    // Expressoes
    // ---------------------------------------------------------------

    TipoExp fatorA() {
        switch (tipo()) {
            case TipoToken::NumInt:
            case TipoToken::NumReal:
            case TipoToken::Var:
                avanca();
                return TipoExp::Aritmetica;

            case TipoToken::AbrePar: {
                avanca();
                TipoExp t = expR();
                consome(TipoToken::FechaPar);
                return t;
            }

            default:
                erroEsperado("operando (variavel, numero ou '(')");
                return TipoExp::Aritmetica;
        }
    }

    // TermoA -> FatorA TermoA'
    TipoExp termoA() {
        int linhaEsq = atual.getLinha();
        TipoExp t = fatorA();

        while (tipo() == TipoToken::OpAritMult || tipo() == TipoToken::OpAritDiv) {
            exigeAritmetica(t, linhaEsq);
            avanca();
            int linhaDir = atual.getLinha();
            exigeAritmetica(fatorA(), linhaDir);
            t = TipoExp::Aritmetica;
        }

        return t;
    }

    // ExpA -> TermoA ExpA'
    TipoExp expA() {
        int linhaEsq = atual.getLinha();
        TipoExp t = termoA();

        while (tipo() == TipoToken::OpAritSoma || tipo() == TipoToken::OpAritSub) {
            exigeAritmetica(t, linhaEsq);
            avanca();
            int linhaDir = atual.getLinha();
            exigeAritmetica(termoA(), linhaDir);
            t = TipoExp::Aritmetica;
        }

        return t;
    }

    // ExpRel -> ExpA ExpRel'
    TipoExp expRel() {
        int linhaEsq = atual.getLinha();
        TipoExp t = expA();

        if (ehOpRel(tipo())) {
            exigeAritmetica(t, linhaEsq);
            avanca();
            int linhaDir = atual.getLinha();
            exigeAritmetica(expA(), linhaDir);
            return TipoExp::Relacional;
        }

        return t;
    }

    // ExpR -> ExpRel ExpR'
    TipoExp expR() {
        int linhaEsq = atual.getLinha();
        TipoExp t = expRel();

        while (tipo() == TipoToken::OpBoolE || tipo() == TipoToken::OpBoolOu) {
            exigeRelacional(t, linhaEsq);
            avanca();
            int linhaDir = atual.getLinha();
            exigeRelacional(expRel(), linhaDir);
            t = TipoExp::Relacional;
        }

        return t;
    }

    // Condicao de SE e ENQTO: precisa ser relacional.
    void condicao() {
        TipoExp t = expR();
        if (t != TipoExp::Relacional) {
            erroEsperado("operador relacional");
        }
    }

    // ---------------------------------------------------------------
    // Comandos
    // ---------------------------------------------------------------

    void comandoAtribuicao() {
        consome(TipoToken::Var);
        consome(TipoToken::Atrib);
        int linhaExp = atual.getLinha();
        exigeAritmetica(expA(), linhaExp);
    }

    void comandoEntrada() {
        consome(TipoToken::PCLer);
        consome(TipoToken::Var);
    }

    void comandoSaida() {
        consome(TipoToken::PCImprimir);

        if (tipo() == TipoToken::Var || tipo() == TipoToken::Cadeia) {
            avanca();
        } else {
            erroEsperado("variavel ou cadeia");
        }
    }

    void comandoCondicao() {
        consome(TipoToken::PCSe);
        condicao();
        consome(TipoToken::PCEntao);
        comando();

        // Else pendente: o SENAO pertence ao SE mais proximo.
        if (tipo() == TipoToken::PCSenao) {
            avanca();
            comando();
        }
    }

    void comandoRepeticao() {
        consome(TipoToken::PCEnqto);
        condicao();
        if (ENQTO_EXIGE_ENTAO) {
            consome(TipoToken::PCEntao);
        }
        comando();
    }

    void subAlgoritmo() {
        consome(TipoToken::PCIni);
        listaComandos(true);
        consome(TipoToken::PCFim);
    }

    void comando() {
        switch (tipo()) {
            case TipoToken::Var: comandoAtribuicao(); break;
            case TipoToken::PCLer: comandoEntrada(); break;
            case TipoToken::PCImprimir: comandoSaida(); break;
            case TipoToken::PCSe: comandoCondicao(); break;
            case TipoToken::PCEnqto: comandoRepeticao(); break;
            case TipoToken::PCIni: subAlgoritmo(); break;
            default: erroEsperado("comando"); break;
        }
    }

    // Descarta tokens ate encontrar um ponto seguro para recomecar: uma
    // palavra que inicia comando, FIM (dentro de bloco) ou EOF. Uma variavel
    // so conta como inicio de comando se estiver em linha posterior a do erro,
    // senao "a > b > c ENTAO" recomecaria em "c" e geraria erro em cascata.
    void ressincroniza(bool dentroDeBloco) {
        int linhaErro = atual.getLinha();
        avanca();  // sempre consome pelo menos um token: garante progresso

        while (tipo() != TipoToken::FimArquivo &&
               !(dentroDeBloco && tipo() == TipoToken::PCFim) &&
               !(ehInicioComando(tipo()) &&
                 (tipo() != TipoToken::Var || atual.getLinha() > linhaErro))) {
            avanca();
        }
    }

    // ListaCmd -> Cmd ListaCmd'. No nivel do programa termina em EOF; dentro
    // de INI ... FIM termina em FIM.
    // Invariante: todo comando chamado aqui comeca em um token de FIRST(Cmd),
    // que ele consome, entao cada iteracao do laco avanca pelo menos um token.
    void listaComandos(bool dentroDeBloco) {
        bool houveComandoOuErro = false;

        while (true) {
            if (ehInicioComando(tipo())) {
                comando();
                houveComandoOuErro = true;
                continue;
            }

            if (tipo() == TipoToken::FimArquivo ||
                (dentroDeBloco && tipo() == TipoToken::PCFim)) {
                break;
            }

            erroEsperado("comando");
            ressincroniza(dentroDeBloco);
            houveComandoOuErro = true;
        }

        if (!houveComandoOuErro) {
            erroEsperado("comando");
        }
    }

    // ---------------------------------------------------------------
    // Declaracoes e programa
    // ---------------------------------------------------------------

    void tipoVar() {
        if (tipo() == TipoToken::PCInt || tipo() == TipoToken::PCReal) {
            avanca();
        } else {
            erroEsperado("tipo (PCInt ou PCReal)");
        }
    }

    void declaracao() {
        consome(TipoToken::Var);
        consome(TipoToken::Delim);
        tipoVar();
    }

    void listaDeclaracoes() {
        declaracao();

        while (tipo() == TipoToken::Var) {
            declaracao();
        }
    }

    void programa() {
        consome(TipoToken::Delim);
        consome(TipoToken::PCDec);
        listaDeclaracoes();
        consome(TipoToken::Delim);
        consome(TipoToken::PCProg);
        listaComandos(false);
    }

public:
    Sintatico(Lexico& lex, Erros& err)
        : lexico(lex),
          erros(err),
          atual(TipoToken::PCDec, "", 0, 1),  // provisorio: avanca() troca
          idUltimoErro(-1) {
        avanca();
    }

    // Analisa o programa inteiro. Os erros ficam em Erros.
    void analisar() {
        programa();
    }
};

#endif
