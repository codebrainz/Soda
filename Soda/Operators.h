#pragma once

#include <string>

namespace Soda
{

    enum UnaryOperator
    {
        UOP_POS,
        UOP_NEG,
        UOP_PREINC,
        UOP_PREDEC,
        UOP_POSTINC,
        UOP_POSTDEC,
        UOP_COMPL,
        UOP_NOT,
        UOP_DEREF,
    };

    enum BinaryOperator
    {
        BOP_ADD,
        BOP_SUB,
        BOP_MUL,
        BOP_DIV,
        BOP_MOD,

        BOP_LOG_AND,
        BOP_LOG_OR,

        BOP_BIT_AND,
        BOP_BIT_OR,
        BOP_BIT_XOR,

        BOP_LSHIFT,
        BOP_RSHIFT,

        BOP_EQ,
        BOP_NE,
        BOP_LT,
        BOP_GT,
        BOP_LE,
        BOP_GE,

        BOP_ASSIGN,
        BOP_IADD,
        BOP_ISUB,
        BOP_IMUL,
        BOP_IDIV,
        BOP_IMOD,
        BOP_IAND,
        BOP_IOR,
        BOP_IXOR,
        BOP_ILSHIFT,
        BOP_IRSHIFT,
    };

    std::string operatorName(UnaryOperator op);
    std::string operatorName(BinaryOperator op);

} // namespace Soda
