#include "Operators.h"

namespace Soda
{

    std::string operatorName(UnaryOperator op)
    {
        switch (op) {
        case UOP_POS:
            return "+";
        case UOP_NEG:
            return "-";
        case UOP_PREINC:
            return "++";
        case UOP_PREDEC:
            return "--";
        case UOP_POSTINC:
            return "++";
        case UOP_POSTDEC:
            return "--";
        case UOP_COMPL:
            return "~";
        case UOP_NOT:
            return "!";
        case UOP_DEREF:
            return "*";
        }
        return "__unknown__";
    }

    std::string operatorName(BinaryOperator op)
    {
        switch (op) {
        case BOP_ADD:
            return "+";
        case BOP_SUB:
            return "-";
        case BOP_MUL:
            return "*";
        case BOP_DIV:
            return "/";
        case BOP_MOD:
            return "%";
        case BOP_LOG_AND:
            return "&&";
        case BOP_LOG_OR:
            return "||";
        case BOP_BIT_AND:
            return "&";
        case BOP_BIT_OR:
            return "|";
        case BOP_BIT_XOR:
            return "^";
        case BOP_LSHIFT:
            return "<<";
        case BOP_RSHIFT:
            return ">>";
        case BOP_EQ:
            return "==";
        case BOP_NE:
            return "!=";
        case BOP_LT:
            return "<";
        case BOP_GT:
            return ">";
        case BOP_LE:
            return "<=";
        case BOP_GE:
            return ">=";
        case BOP_ASSIGN:
            return "=";
        case BOP_IADD:
            return "+=";
        case BOP_ISUB:
            return "-=";
        case BOP_IMUL:
            return "*=";
        case BOP_IDIV:
            return "/=";
        case BOP_IMOD:
            return "%=";
        case BOP_IAND:
            return "&=";
        case BOP_IOR:
            return "|=";
        case BOP_IXOR:
            return "^=";
        case BOP_ILSHIFT:
            return "<<=";
        case BOP_IRSHIFT:
            return ">>=";
        }
        return "";
    }

} // namespace Soda
