// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include <vector>

namespace circa {
namespace token {

struct Token
{
    int match;
    std::string text;
    int lineStart;
    int lineEnd;
    int colStart;
    int colEnd;
    int precedingIndent;

    Token() : match(0), lineStart(0), lineEnd(0), colStart(0), colEnd(0), precedingIndent(0) {}

    std::string toString() const;
};

typedef std::vector<Token> TokenList;

const int IDENTIFIER = 1;
const int INTEGER = 3;
const int HEX_INTEGER = 4;
const int FLOAT_TOKEN = 5;
const int STRING = 6;
const int COLOR = 7;
const int BOOL = 8;

const int LPAREN = 10;
const int RPAREN = 11;
const int LBRACE = 12;
const int RBRACE = 13;
const int LBRACKET = 14;
const int RBRACKET = 15;
const int COMMA = 16;
const int AT_SIGN = 17;
const int DOT = 18;
const int STAR = 19;
const int QUESTION = 20;
const int SLASH = 21;
const int DOUBLE_SLASH = 44;
const int PLUS = 22;
const int MINUS = 23;
const int LTHAN = 24;
const int LTHANEQ = 25;
const int GTHAN = 26;
const int GTHANEQ = 27;
const int PERCENT = 41;
const int COLON = 28;
const int DOUBLE_COLON = 47;
const int DOUBLE_EQUALS = 29;
const int NOT_EQUALS = 30;
const int EQUALS = 31;
const int PLUS_EQUALS = 32;
const int MINUS_EQUALS = 33;
const int STAR_EQUALS = 34;
const int SLASH_EQUALS = 35;
const int COLON_EQUALS = 36;
const int RIGHT_ARROW = 37;
const int LEFT_ARROW = 43;
const int AMPERSAND = 45;
const int DOUBLE_AMPERSAND = 38;
const int DOUBLE_VERTICAL_BAR = 39;
const int SEMICOLON = 40;
const int TWO_DOTS = 46;
const int ELLIPSIS = 42;
const int TRIPLE_LTHAN = 48;
const int TRIPLE_GTHAN = 49;

const int DEF = 51;
const int TYPE = 52;
const int BEGIN = 64;
const int DO = 70;
const int END = 53;
const int IF = 54;
const int ELSE = 55;
const int ELIF = 63;
const int FOR = 56;
const int STATE = 57;
const int RETURN = 58;
const int IN_TOKEN = 59;
const int TRUE_TOKEN = 60;
const int FALSE_TOKEN = 61;
const int DO_ONCE = 62;
const int NAMESPACE = 65;
const int INCLUDE = 66;
const int AND = 67;
const int OR = 68;
const int DISCARD = 69;
const int NULL_TOKEN = 71;
const int BREAK = 72;
const int CONTINUE = 73;

const int WHITESPACE = 80;
const int NEWLINE = 81;
const int COMMENT = 82;
const int EOF_TOKEN = 83;

const int UNRECOGNIZED = 90;

const char* get_token_text(int match);
void tokenize(std::string const &input, TokenList &results);

} // namespace token
} // namespace circa
