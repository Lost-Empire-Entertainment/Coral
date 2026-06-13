//Copyright(C) 2026 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <filesystem>
#include <iterator>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <cctype>

#include "log_utils.hpp"
#include "file_utils.hpp"
#include "core_utils.hpp"

#include "lexer.hpp"
#include "core.hpp"

using KalaHeaders::KalaLog::Log;
using KalaHeaders::KalaLog::LogType;

using KalaHeaders::KalaFile::ReadLinesFromFile;

using KalaHeaders::KalaCore::EnumHash;
using KalaHeaders::KalaCore::EnumToString;

using Coral::TokenType;
using Coral::TokenValue;
using Coral::Token_Operator;
using Coral::Token_Variable;
using Coral::Token_Return;
using Coral::Token_Start;
using Coral::Token_End;
using Coral::Token_Visibility;
using Coral::Token_Mutability;
using Coral::Token_Jump;

using std::string_view;
using std::to_string;
using std::filesystem::path;
using std::unordered_map;
using std::pair;
using std::holds_alternative;

constexpr string_view type_identifier = "identifier";
constexpr string_view type_value      = "value";
constexpr string_view type_operator   = "operator";
constexpr string_view type_variable   = "variable";
constexpr string_view type_return     = "return";
constexpr string_view type_start      = "start";
constexpr string_view type_end        = "end";
constexpr string_view type_visibility = "visibility";
constexpr string_view type_mutability = "mutability";
constexpr string_view type_jump       = "jump";

constexpr string_view operator_assign       = "=";
constexpr string_view operator_return       = ":";
constexpr string_view operator_and          = "&&";
constexpr string_view operator_equals       = "==";
constexpr string_view operator_not_equals   = "!=";
constexpr string_view operator_plus_equals  = "+=";
constexpr string_view operator_minus_equals = "-=";
constexpr string_view operator_multiply_eq  = "*=";
constexpr string_view operator_divide_eq    = "/=";
constexpr string_view operator_more_than    = ">";
constexpr string_view operator_less_than    = "<";
constexpr string_view operator_more_and_eq  = ">=";
constexpr string_view operator_less_and_eq  = "<=";
constexpr string_view operator_add          = "+";
constexpr string_view operator_subtract     = "-";
constexpr string_view operator_multiply     = "*";
constexpr string_view operator_divide       = "/";

constexpr string_view variable_int    = "int";
constexpr string_view variable_float  = "float";
constexpr string_view variable_double = "double";
constexpr string_view variable_bool   = "bool";

constexpr string_view return_int    = "int";
constexpr string_view return_float  = "float";
constexpr string_view return_double = "double";
constexpr string_view return_bool   = "bool";
constexpr string_view return_null   = "null";

constexpr string_view start_main  = "main";
constexpr string_view start_func  = "func";
constexpr string_view start_block = "block";
constexpr string_view start_while = "while";
constexpr string_view start_for   = "for";

constexpr string_view end_main  = "endmain";
constexpr string_view end_func  = "endfunc";
constexpr string_view end_block = "endblock";
constexpr string_view end_while = "endwhile";
constexpr string_view end_for   = "endfor";

constexpr string_view visibility_public  = "public";
constexpr string_view visibility_private = "private";

constexpr string_view mutability_readonly  = "readonly";
constexpr string_view mutability_locked    = "locked";

constexpr string_view jump_return   = "return";
constexpr string_view jump_break    = "break";
constexpr string_view jump_continue = "continue";

static const unordered_map<TokenType, string_view, EnumHash<TokenType>> types =
{
    { TokenType::T_IDENTIFIER, type_identifier },
    { TokenType::T_VALUE,      type_value },
    { TokenType::T_OPERATOR,   type_operator },
    { TokenType::T_VARIABLE,   type_variable },
    { TokenType::T_RETURN,     type_return },
    { TokenType::T_START,      type_start },
    { TokenType::T_END,        type_end },
    { TokenType::T_VISIBILITY, type_visibility },
    { TokenType::T_MUTABILITY, type_mutability },
    { TokenType::T_JUMP,       type_jump }
};

static const unordered_map<Token_Operator, string_view, EnumHash<Token_Operator>> operators =
{
    { Token_Operator::O_ASSIGN, operator_assign },
    { Token_Operator::O_RETURN, operator_return },

    { Token_Operator::O_AND,             operator_and },
    { Token_Operator::O_EQUALS,          operator_equals },
    { Token_Operator::O_NOT_EQUALS,      operator_not_equals },
    { Token_Operator::O_PLUS_EQUALS,     operator_plus_equals },
    { Token_Operator::O_MINUS_EQUALS,    operator_minus_equals },
    { Token_Operator::O_MULTIPLY_EQUALS, operator_multiply_eq },
    { Token_Operator::O_DIVIDE_EQUALS,   operator_divide_eq },

    { Token_Operator::O_MORE_THAN,      operator_more_than },
    { Token_Operator::O_LESS_THAN,      operator_less_than },
    { Token_Operator::O_MORE_AND_EQUAL, operator_more_and_eq },
    { Token_Operator::O_LESS_AND_EQUAL, operator_less_and_eq },

    { Token_Operator::O_ADD,      operator_add },
    { Token_Operator::O_SUBTRACT, operator_subtract },
    { Token_Operator::O_MULTIPLY, operator_multiply },
    { Token_Operator::O_DIVIDE,   operator_divide }
};

static const unordered_map<Token_Variable, string_view, EnumHash<Token_Variable>> variables =
{
    { Token_Variable::V_INT,    variable_int },
    { Token_Variable::V_FLOAT,  variable_float },
    { Token_Variable::V_DOUBLE, variable_double },
    { Token_Variable::V_BOOL,   variable_bool }
};

static const unordered_map<Token_Return, string_view, EnumHash<Token_Return>> returns =
{
    { Token_Return::R_INT,    return_int },
    { Token_Return::R_FLOAT,  return_float },
    { Token_Return::R_DOUBLE, return_double },
    { Token_Return::R_BOOL,   return_bool },

    { Token_Return::R_NULL,   return_null }
};

static const unordered_map<Token_Start, string_view, EnumHash<Token_Start>> starts =
{
    { Token_Start::S_MAIN,  start_main },
    { Token_Start::S_FUNC,  start_func },
    { Token_Start::S_BLOCK, start_block },

    { Token_Start::S_WHILE, start_while },
    { Token_Start::S_FOR,   start_for }
};

static const unordered_map<Token_End, string_view, EnumHash<Token_End>> ends =
{
    { Token_End::E_ENDMAIN,  end_main },
    { Token_End::E_ENDFUNC,  end_func },
    { Token_End::E_ENDBLOCK, end_block },

    { Token_End::E_ENDWHILE, end_while },
    { Token_End::E_ENDFOR,   end_for }
};

static const unordered_map<Token_Visibility, string_view, EnumHash<Token_Visibility>> visibilities =
{
    { Token_Visibility::V_PUBLIC,   visibility_public },
    { Token_Visibility::V_PRIVATE,  visibility_private }
};

static const unordered_map<Token_Mutability, string_view, EnumHash<Token_Mutability>> mutabilities =
{
    { Token_Mutability::M_READONLY,  mutability_readonly },
    { Token_Mutability::M_LOCKED,    mutability_locked }
};

static const unordered_map<Token_Jump, string_view, EnumHash<Token_Jump>> jumps =
{
    { Token_Jump::J_RETURN,   jump_return },
    { Token_Jump::J_BREAK,    jump_break },
    { Token_Jump::J_CONTINUE, jump_continue }
};

static unordered_map<string_view, pair<TokenType, TokenValue>> lexerLookup{};

template<typename EnumType>
static void InvertMap(const unordered_map<EnumType, string_view, EnumHash<EnumType>>& sourceMap, TokenType type)
{
    for (const auto& [k, v] : sourceMap)
    {
        lexerLookup[v] = { type, k };
    }
}

static void InitializeLexerTables()
{
    InvertMap(operators,    TokenType::T_OPERATOR);
    InvertMap(variables,    TokenType::T_VARIABLE);
    InvertMap(returns,      TokenType::T_RETURN);
    InvertMap(starts,       TokenType::T_START);
    InvertMap(ends,         TokenType::T_END);
    InvertMap(visibilities, TokenType::T_VISIBILITY);
    InvertMap(mutabilities, TokenType::T_MUTABILITY);
    InvertMap(jumps,        TokenType::T_JUMP);
}

namespace Coral
{
    string_view Lexer::TokenValueToString(const TokenValue& value)
    {
        if (holds_alternative<monostate>(value)) return "identifier or value";

        for (const auto& [k, v] : lexerLookup)
        {
            if (v.second == value) return k;
        }

        return "unknown token value";
    }

    TokenData Lexer::Tokenize(const path& script)
    {
        if (lexerLookup.empty()) InitializeLexerTables();

        string relative = path(script.parent_path().filename() / script.filename()).string(); 

        Log::Print(
            "Starting to tokenize '" + relative + "'.",
            "LEXER",
            LogType::LOG_INFO);

        vector<string> lines{};
        string errorMsg = ReadLinesFromFile(script, lines);

        if (!errorMsg.empty())
        {
            CoralCore::ExitOnError("Failed to read lines from coral script! Reason: " + errorMsg, "LEXER");
        }

        auto parse_spaces = [&lines, &script]() -> TokenData
            {
                TokenData tdata{};
                tdata.scriptPath = script;

                u32 linePos = 1;

                bool isInMultilineComment{};
                for (const auto& line : lines)
                {
                    if (line.empty())
                    {
                        Log::Print(
                            "Skipped empty line '" + to_string(linePos) + "'.",
                            "LEXER",
                            LogType::LOG_DEBUG);

                        linePos++;

                        continue;
                    }

#ifdef KDEBUG
                    Log::Print(" ");
#endif
                    Log::Print(
                        "--- First lexer pass for line '" + line + "' ---",
                        "LEXER",
                        LogType::LOG_DEBUG);

                    u32 wordCharStart = 1;
            
                    Token t{};

                    for (size_t i = 0; i < line.size(); i++)
                    {
                        char c = line[i];

                        if (!isInMultilineComment
                            && i != line.size() -1
                            && c == '/')
                        {
                            if (line[i + 1] == '/')
                            {
                                Log::Print(
                                    "Found command operator "
                                    "at line '" + to_string(linePos) + "' "
                                    "and pos '" + to_string(wordCharStart) + "'.",
                                    "LEXER",
                                    LogType::LOG_DEBUG);

                                if (!t.token.empty())
                                {
                                    t.line = linePos;

                                    tdata.tokens.push_back(t);
                                    t.token.clear();
                                }

                                break;
                            }
                            else if (line[i + 1] == '*')
                            {
                                if (isInMultilineComment)
                                {
                                    CoralCore::ExitOnError(
                                        "Found duplicate multiline comment start "
                                        "on line '" + to_string(linePos) + "' "
                                        "and position '" + to_string(wordCharStart) + "'!", "LEXER");
                                }

                                Log::Print(
                                    "Found multiline command operator start "
                                    "at line '" + to_string(linePos) + "' "
                                    "and pos '" + to_string(wordCharStart) + "'.",
                                    "LEXER",
                                    LogType::LOG_DEBUG);

                                isInMultilineComment = true;

                                if (!t.token.empty())
                                {
                                    t.line = linePos;

                                    tdata.tokens.push_back(t);
                                    t.token.clear();
                                }

                                i++;
                                wordCharStart += 2;

                                continue;
                            }
                        }

                        if (i != line.size() -1
                            && c == '*'
                            && line[i + 1] == '/')
                        {
                            if (!isInMultilineComment)
                            {
                                CoralCore::ExitOnError(
                                    "Found multiline comment end without existing multiline comment start "
                                    "on line '" + to_string(linePos) + "' "
                                    "and position '" + to_string(wordCharStart) + "'!", "LEXER");
                            }

                            isInMultilineComment = false;

                            i++;
                            wordCharStart += 2;

                            continue;
                        }

                        if (isInMultilineComment)
                        {
                            wordCharStart++;
                            continue;
                        }

                        if (isspace(c))
                        {
                            if (!t.token.empty())
                            {
                                t.line = linePos;

                                tdata.tokens.push_back(t);
                                t.token.clear();
                            }

                            wordCharStart++;
                            continue;
                        }

                        if (t.token.empty()) t.pos = wordCharStart;
                        t.token += c;

                        wordCharStart++;
                    }

                    if (!t.token.empty())
                    {
                        t.line = linePos;

                        tdata.tokens.push_back(t);
                        t.token.clear();
                    }

                    linePos++;
                }

                if (isInMultilineComment)
                {
                    CoralCore::ExitOnError("Script ended without multiline commend end!", "LEXER");
                }

                return tdata;
            };

        TokenData tdata = parse_spaces();

        auto parse_operators = [&tdata]() -> TokenData
            {
                TokenData opTokenData{};
                opTokenData.scriptPath = tdata.scriptPath;

                for (auto& t : tdata.tokens)
                {
#ifdef KDEBUG
                    Log::Print(" ");
#endif
                    Log::Print(
                        "--- Second lexer pass for token '" + t.token + "' at line '" + to_string(t.line) + "' and pos '" + to_string(t.pos) + "' ---",
                        "LEXER",
                        LogType::LOG_DEBUG);

                    auto has_any_symbols = [&t]() -> bool
                        {
                            for (const char c : t.token)
                            {
                                if (!isalnum(c)) return true;
                            }

                            return false;
                        };

                    auto has_any_op = [&t]() -> bool
                        {
                            for (const auto& [_, v] : operators)
                            {
                                return t.token.find(v) != string::npos;
                            }

                            return false;
                        };

                    if (has_any_symbols())
                    {
                        if (!has_any_op()
                            && t.token.find('!') == string::npos)
                        {
                            CoralCore::ExitOnError(
                                "Found unsupported operator symbol in token '" + t.token + "' "
                                "on line '" + to_string(t.line) + "' "
                                "and position '" + to_string(t.pos) + "'!", "LEXER");
                        }

                        vector<Token> splitTokens{};
                        Token splitToken{};
                        splitToken.line = t.line;

                        u32 tokenCharStart = 1;

                        for (size_t i = 0; i < t.token.size(); i++)
                        {
                            char c = t.token[i];

                            if (!isalnum(c))
                            {
                                if (!splitToken.token.empty())
                                {
                                    splitTokens.push_back(splitToken);
                                    splitToken.token.clear();
                                }

                                tokenCharStart++;

                                if (i + 1 != t.token.size())
                                {
                                    if (c == '!'
                                        && !isalpha(t.token[i + 1])
                                        && t.token[i + 1] != '=')
                                    {
                                        CoralCore::ExitOnError(
                                            "Found unsupported exclamation mark structure "
                                            "on line '" + to_string(t.line) + "' "
                                            "and position '" + to_string(t.pos) + "'!", "LEXER");
                                    }

                                    if (!isalnum(t.token[i + 1]))
                                    {
                                        string doubleOp = { c, t.token[i + 1] };

                                        if (c == '!'
                                            && doubleOp != operator_not_equals)
                                        {
                                            CoralCore::ExitOnError(
                                                "Found unsupported exclamation mark structure "
                                                "on line '" + to_string(t.line) + "' "
                                                "and position '" + to_string(t.pos) + "'!", "LEXER");
                                        }

                                        auto is_double_op = [&doubleOp]() -> bool
                                            {
                                                return doubleOp == operator_equals
                                                    || doubleOp == operator_not_equals
                                                    || doubleOp == operator_plus_equals
                                                    || doubleOp == operator_minus_equals
                                                    || doubleOp == operator_multiply_eq
                                                    || doubleOp == operator_divide_eq
                                                    || doubleOp == operator_more_and_eq
                                                    || doubleOp == operator_less_and_eq;
                                            };

                                        if (is_double_op())
                                        {
                                            splitTokens.push_back({
                                                .line = t.line,
                                                .pos = tokenCharStart,
                                                .token = doubleOp
                                            });
                                        } 

                                        i++;
                                        tokenCharStart += 2;
                                        continue;
                                    }
                                    else
                                    {
                                        splitTokens.push_back({
                                            .line = t.line,
                                            .pos = tokenCharStart,
                                            .token = {c}
                                        });

                                        continue;
                                    }
                                }

                                splitTokens.push_back({
                                    .line = t.line,
                                    .pos = tokenCharStart,
                                    .token = {c}
                                });

                                continue;
                            }

                            if (splitToken.token.empty()) splitToken.pos = tokenCharStart;
                            splitToken.token += c;

                            tokenCharStart++;
                        }

                        if (!splitToken.token.empty())
                        {
                            splitTokens.push_back(splitToken);
                            splitToken.token.clear();
                        }

                        opTokenData.tokens.insert(
                            opTokenData.tokens.end(),
                            make_move_iterator(splitTokens.begin()),
                            make_move_iterator(splitTokens.end()));
                    }
                    else 
                    {
                        opTokenData.tokens.push_back(t);
                        continue;
                    }
                }

                return opTokenData;
            };

        TokenData opTokenData = parse_operators();

        auto assign_token_types = [&opTokenData]() -> void
            {
                auto is_digit = [](string_view value) -> bool
                    {
                        bool allDigits = true;
                        for (const auto c : value)
                        {
                            if (!isdigit(c))
                            {
                                allDigits = false;
                                break;
                            }
                        }

                        if (allDigits) return true;

                        return isdigit(value[0])
                            && value.find('.') != string::npos;
                    };

                for (auto& t : opTokenData.tokens)
                {
#ifdef KDEBUG
                    Log::Print(" ");
#endif
                    Log::Print(
                        "--- Third lexer pass for token '" + t.token + "' at line '" + to_string(t.line) + "' and pos '" + to_string(t.pos) + "' ---",
                        "LEXER",
                        LogType::LOG_DEBUG);

                    auto it = lexerLookup.find(t.token);
                    if (it != lexerLookup.end())
                    {
                        t.tokenType = it->second.first;
                        t.tokenValue = it->second.second;

                        if (CoralCore::IsVerbose())
                        {
                            string_view tt{};
                            EnumToString(t.tokenType, types, tt);

                            string_view tv = TokenValueToString(t.tokenValue);

                            Log::Print(
                                "Token '" + t.token + "' "
                                "at line '" + to_string(t.line) + "' "
                                "and pos '" + to_string(t.pos) + "' "
                                "has type '" + string(tt) + "' and value '" + string(tv) + "'.",
                                "LEXER",
                                LogType::LOG_INFO);
                        }
                    }
                    else
                    {
                        if (is_digit(t.token))
                        {
                            t.tokenType = TokenType::T_VALUE;

                            string_view tt{};
                            EnumToString(t.tokenType, types, tt);

                            Log::Print(
                                "Token '" + t.token + "' "
                                "at line '" + to_string(t.line) + "' "
                                "and pos '" + to_string(t.pos) + "' "
                                "has type '" + string(tt) + "'.",
                                "LEXER",
                                LogType::LOG_INFO);
                        }
                        else
                        {
                            if (CoralCore::IsVerbose())
                            {
                                Log::Print(
                                    "Unable to determine if token '" + t.token + "' "
                                    "at line '" + to_string(t.line) + "' "
                                    "and pos '" + to_string(t.pos) + "' "
                                    "is a value or identifier, type will be assigned at the parser stage.",
                                    "LEXER",
                                    LogType::LOG_INFO);
                            }
                        }
                    }
                }
            };

        assign_token_types();

        Log::Print(
            "Finished tokenizing '" + relative + "'!",
            "LEXER",
            LogType::LOG_SUCCESS);

        return tdata;
    }
}