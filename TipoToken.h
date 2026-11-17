#ifndef TIPO_TOKEN_H
#define TIPO_TOKEN_H

#include <string>

enum class TipoToken {
    PCDec,
    PCProg,
    PCInt,
    PCReal,
    PCLer,
    PCImprimir,
    PCSe,
    PCEntao,
    PCSenao,
    PCEnqto,
    PCIni,
    PCFim,

    OpAritMult,
    OpAritDiv,
    OpAritSoma,
    OpAritSub,

    OpRelMenor,
    OpRelMenorIgual,
    OpRelMaior,
    OpRelMaiorIgual,
    OpRelIgual,
    OpRelDif,

    OpBoolE,
    OpBoolOu,

    Delim,
    Atrib,
    AbrePar,
    FechaPar,
    Var,
    NumInt,
    NumReal,
    Cadeia,

    FimArquivo
};

inline std::string nomeTipoToken(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::PCDec: return "PCDec";
        case TipoToken::PCProg: return "PCProg";
        case TipoToken::PCInt: return "PCInt";
        case TipoToken::PCReal: return "PCReal";
        case TipoToken::PCLer: return "PCLer";
        case TipoToken::PCImprimir: return "PCImprimir";
        case TipoToken::PCSe: return "PCSe";
        case TipoToken::PCEntao: return "PCEntao";
        case TipoToken::PCSenao: return "PCSenao";
        case TipoToken::PCEnqto: return "PCEnqto";
        case TipoToken::PCIni: return "PCIni";
        case TipoToken::PCFim: return "PCFim";
        case TipoToken::OpAritMult: return "OpAritMult";
        case TipoToken::OpAritDiv: return "OpAritDiv";
        case TipoToken::OpAritSoma: return "OpAritSoma";
        case TipoToken::OpAritSub: return "OpAritSub";
        case TipoToken::OpRelMenor: return "OpRelMenor";
        case TipoToken::OpRelMenorIgual: return "OpRelMenorIgual";
        case TipoToken::OpRelMaior: return "OpRelMaior";
        case TipoToken::OpRelMaiorIgual: return "OpRelMaiorIgual";
        case TipoToken::OpRelIgual: return "OpRelIgual";
        case TipoToken::OpRelDif: return "OpRelDif";
        case TipoToken::OpBoolE: return "OpBoolE";
        case TipoToken::OpBoolOu: return "OpBoolOu";
        case TipoToken::Delim: return "Delim";
        case TipoToken::Atrib: return "Atrib";
        case TipoToken::AbrePar: return "AbrePar";
        case TipoToken::FechaPar: return "FechaPar";
        case TipoToken::Var: return "Var";
        case TipoToken::NumInt: return "NumInt";
        case TipoToken::NumReal: return "NumReal";
        case TipoToken::Cadeia: return "Cadeia";
        case TipoToken::FimArquivo: return "EOF";
    }

    return "Desconhecido";
}

#endif
