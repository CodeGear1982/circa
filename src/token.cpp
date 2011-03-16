// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "token.h"

namespace circa {
namespace token {

const char* get_token_text(int match)
{
    switch (match) {
        case LPAREN: return "(";
        case RPAREN: return ")";
        case LBRACE: return "{";
        case RBRACE: return "}";
        case LBRACKET: return "[";
        case RBRACKET: return "]";
        case COMMA: return ",";
        case AT_SIGN: return "@";
        case IDENTIFIER: return "IDENTIFIER";
        case INTEGER: return "INTEGER";
        case HEX_INTEGER: return "HEX_INTEGER";
        case FLOAT_TOKEN: return "FLOAT";
        case STRING: return "STRING";
        case COLOR: return "COLOR";
        case COMMENT: return "COMMENT";
        case DOT: return ".";
        case STAR: return "*";
        case QUESTION: return "?";
        case SLASH: return "/";
        case DOUBLE_SLASH: return "//";
        case PLUS: return "+";
        case MINUS: return "-";
        case LTHAN: return "<";
        case LTHANEQ: return "<=";
        case GTHAN: return ">";
        case GTHANEQ: return ">=";
        case PERCENT: return "%";
        case COLON: return ":";
        case DOUBLE_COLON: return "::";
        case DOUBLE_EQUALS: return "==";
        case NOT_EQUALS: return "!=";
        case EQUALS: return "=";
        case PLUS_EQUALS: return "+=";
        case MINUS_EQUALS: return "-=";
        case STAR_EQUALS: return "*=";
        case SLASH_EQUALS: return "/=";
        case COLON_EQUALS: return ":=";
        case RIGHT_ARROW: return "->";
        case LEFT_ARROW: return "<-";
        case AMPERSAND: return "&";
        case DOUBLE_AMPERSAND: return "&&";
        case DOUBLE_VERTICAL_BAR: return "||";
        case SEMICOLON: return ";";
        case TWO_DOTS: return "..";
        case ELLIPSIS: return "...";
        case TRIPLE_LTHAN: return "<<<";
        case TRIPLE_GTHAN: return ">>>";
        case WHITESPACE: return "WHITESPACE";
        case NEWLINE: return "NEWLINE";
        case BEGIN: return "begin";
        case DO: return "do";
        case END: return "end";
        case IF: return "if";
        case ELSE: return "else";
        case ELIF: return "elif";
        case FOR: return "for";
        case STATE: return "state";
        case DEF: return "def";
        case TYPE: return "type";
        case RETURN: return "return";
        case IN_TOKEN: return "in";
        case TRUE_TOKEN: return "true";
        case FALSE_TOKEN: return "false";
        case DO_ONCE: return "do once";
        case NAMESPACE: return "namespace";
        case INCLUDE: return "include";
        case AND: return "and";
        case OR: return "or";
        case DISCARD: return "discard";
        case NULL_TOKEN: return "null";
        case BREAK: return "break";
        case CONTINUE: return "continue";
        case UNRECOGNIZED: return "UNRECOGNIZED";
        default: return "NOT FOUND";
    }
}

std::string Token::toString() const
{
    std::stringstream out;
    out << get_token_text(match) << " \"" << text << "\"";
    return out.str();
}

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    int linePosition;
    int charPosition;
    std::vector<Token> &results;

    TokenizeContext(std::string const &_input, std::vector<Token> &_results)
        : input(_input),
          nextIndex(0),
          linePosition(1),
          charPosition(0),
          results(_results)
    { }

    char next(int lookahead=0) const {
        unsigned int index = nextIndex + lookahead;
        if (index >= input.length())
            return 0;
        return input[index];
    }

    char consume() {
        if (finished())
            return 0;

        char c = next();
        nextIndex++;

        if (c == '\n') {
            this->linePosition++;
            this->charPosition = 0;
        } else
            this->charPosition++;

        return c;
    }

    bool finished() const {
        return nextIndex >= (int) input.length();
    }

    void push(int match, std::string text = "") {
        if (text == "" && match != STRING)
            text = get_token_text(match);

        Token instance;
        instance.match = match;
        instance.text = text;

        // Record where this token started
        if (results.size() == 0) {
            instance.lineStart = 1;
            instance.colStart = 0;
            instance.precedingIndent = -1;
        } else {
            Token& prevToken = results[results.size()-1];
            if (prevToken.match == NEWLINE) {
                instance.lineStart = prevToken.lineEnd + 1;
                instance.colStart = 0;
                instance.precedingIndent = -1;
            } else {
                instance.lineStart = prevToken.lineEnd;
                instance.colStart = prevToken.colEnd;
                instance.precedingIndent = prevToken.precedingIndent;
            }
        }

        // Record where this token ended
        if (instance.match == NEWLINE) {
            instance.lineEnd = this->linePosition - 1;
            instance.colEnd = instance.colStart + 1;
        } else {
            instance.lineEnd = this->linePosition;
            instance.colEnd = this->charPosition;
        }

        // Update precedingIndent if this is the first whitespace on a line
        if (instance.precedingIndent == -1) {
            if (instance.match == WHITESPACE)
                instance.precedingIndent = instance.text.length();
            else
                instance.precedingIndent = 0;
        }

        ca_assert(instance.lineStart >= 0);
        ca_assert(instance.lineEnd >= 0);
        ca_assert(instance.colStart >= 0);
        ca_assert(instance.colEnd >= 0);
        ca_assert(instance.lineStart <= instance.lineEnd);
        ca_assert((instance.colEnd > instance.colStart) ||
                instance.lineStart < instance.lineEnd);

        results.push_back(instance);
    }
};

void top_level_consume_token(TokenizeContext &context);
void consume_identifier(TokenizeContext &context);
void consume_whitespace(TokenizeContext &context);
void consume_comment(TokenizeContext& context);
bool match_number(TokenizeContext &context);
void consume_number(TokenizeContext &context);
void consume_hex_number(TokenizeContext &context);
void consume_string_literal(TokenizeContext &context);
void consume_triple_quoted_string_literal(TokenizeContext &context);
void consume_color_literal(TokenizeContext &context);

void tokenize(std::string const &input, TokenList &results)
{
    TokenizeContext context(input, results);

    while (!context.finished()) {
        top_level_consume_token(context);
    }
}

bool is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_number(char c)
{
    return (c >= '0' && c <= '9');
}

bool is_hexadecimal_digit(char c)
{
    return is_number(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_acceptable_inside_identifier(char c)
{
    return is_letter(c) || is_number(c) || c == '_';
}

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

bool is_newline(char c)
{
    return c == '\n';
}

bool try_to_consume_keyword(TokenizeContext& context, int keyword)
{
    const char* str = get_token_text(keyword);
    int str_len = (int) strlen(str);

    // Check if every letter matches
    for (int i=0; i < str_len; i++) {
        if (context.next(i) != str[i])
            return false;
    }

    // Check that this is really the end of the word
    if (is_acceptable_inside_identifier(context.next(str_len)))
        return false;

    // Don't match as a keyword if the next character is (. This might be
    // a bad idea.
    if (context.next(str_len) == '(')
        return false;

    // Keyword matches, now consume it
    for (int i=0; i < str_len; i++)
        context.consume();

    context.push(keyword);

    return true;
}

void top_level_consume_token(TokenizeContext &context)
{
    if (is_letter(context.next()) || context.next() == '_') {

        if (try_to_consume_keyword(context, DEF)) return;
        if (try_to_consume_keyword(context, TYPE)) return;
        if (try_to_consume_keyword(context, BEGIN)) return;
        if (try_to_consume_keyword(context, END)) return;
        if (try_to_consume_keyword(context, IF)) return;
        if (try_to_consume_keyword(context, ELSE)) return;
        if (try_to_consume_keyword(context, ELIF)) return;
        if (try_to_consume_keyword(context, FOR)) return;
        if (try_to_consume_keyword(context, STATE)) return;
        if (try_to_consume_keyword(context, IN_TOKEN)) return;
        if (try_to_consume_keyword(context, TRUE_TOKEN)) return;
        if (try_to_consume_keyword(context, FALSE_TOKEN)) return;
        // check 'do once' before 'do'
        if (try_to_consume_keyword(context, DO_ONCE)) return; 
        if (try_to_consume_keyword(context, DO)) return;
        if (try_to_consume_keyword(context, NAMESPACE)) return;
        if (try_to_consume_keyword(context, INCLUDE)) return;
        if (try_to_consume_keyword(context, AND)) return;
        if (try_to_consume_keyword(context, OR)) return;
        if (try_to_consume_keyword(context, DISCARD)) return;
        if (try_to_consume_keyword(context, NULL_TOKEN)) return;
        if (try_to_consume_keyword(context, RETURN)) return;
        if (try_to_consume_keyword(context, BREAK)) return;
        if (try_to_consume_keyword(context, CONTINUE)) return;

        consume_identifier(context);
        return;
    }

    if (is_whitespace(context.next())) {
        consume_whitespace(context);
        return;
    }

    if (context.next() == '0'
        && context.next(1) == 'x') {
        consume_hex_number(context);
        return;
    }

    if (match_number(context)) {
        consume_number(context);
        return;
    }

    // Check for specific characters
    switch(context.next()) {
        case '(':
            context.consume();
            context.push(LPAREN, "(");
            return;
        case ')':
            context.consume();
            context.push(RPAREN, ")");
            return;
        case '{':
            context.consume();
            context.push(LBRACE);
            return;
        case '}':
            context.consume();
            context.push(RBRACE);
            return;
        case '[':
            context.consume();
            context.push(LBRACKET);
            return;
        case ']':
            context.consume();
            context.push(RBRACKET);
            return;
        case ',':
            context.consume();
            context.push(COMMA);
            return;
        case '@':
            context.consume();
            context.push(AT_SIGN);
            return;
        case '=':
            context.consume();

            if (context.next() == '=') {
                context.consume();
                context.push(DOUBLE_EQUALS);
                return;
            } 

            context.push(EQUALS);
            return;
        case '"':
        case '\'':
            consume_string_literal(context);
            return;
        case '\n':
            context.consume();
            context.push(NEWLINE, "\n");
            return;
        case '.':
            context.consume();

            if (context.next(0) == '.') {
                context.consume();

                if (context.next(0) == '.') {
                    context.consume();
                    context.push(ELLIPSIS); 
                } else {
                    context.push(TWO_DOTS);
                }
            } else {
                context.push(DOT);
            }
            return;
        case '?':
            context.consume();
            context.push(QUESTION);
            return;
        case '*':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(STAR_EQUALS);
                return;
            }

            context.push(STAR);
            return;
        case '/':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(SLASH_EQUALS);
                return;
            }
            if (context.next() == '/') {
                context.consume();
                context.push(DOUBLE_SLASH);
                return;
            }
            context.push(SLASH);
            return;
        case '!':
            if (context.next(1) == '=') {
                context.consume();
                context.consume();
                context.push(NOT_EQUALS);
                return;
            }
            break;

        case ':':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(COLON_EQUALS);
                return;
            }
            else if (context.next() == ':') {
                context.consume();
                context.push(DOUBLE_COLON);
                return;
            }

            context.push(COLON);
            return;
        case '+':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(PLUS_EQUALS);
            } else {
                context.push(PLUS);
            }
            return;
        case '-':
            if (context.next(1) == '>') {
                context.consume();
                context.consume();
                context.push(RIGHT_ARROW);
                return;
            }

            if (context.next(1) == '-') {
                consume_comment(context);
                return;
            }

            if (context.next(1) == '=') {
                context.consume();
                context.consume();
                context.push(MINUS_EQUALS);
                return;
            }

            context.consume();
            context.push(MINUS);
            return;

        case '<':
            if (context.next(1) == '<' && context.next(2) == '<') {
                consume_triple_quoted_string_literal(context);
                return;
            }

            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(LTHANEQ);
                return;
            }
            if (context.next() == '-') {
                context.consume();
                context.push(LEFT_ARROW);
                return;
            }
            context.push(LTHAN);
            return;

        case '>':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(GTHANEQ);
                return;
            }
            context.push(GTHAN);
            return;

        case '%':
            context.consume();
            context.push(PERCENT);
            return;

        case '|':
            if (context.next(1) == '|') {
                context.consume();
                context.consume();
                context.push(DOUBLE_VERTICAL_BAR);
                return;
            }
            break;

        case '&':
            context.consume();

            if (context.next() == '&') {
                context.consume();
                context.push(DOUBLE_AMPERSAND);
                return;
            }

            context.push(AMPERSAND);
            return;

        case ';':
            if (context.next() == ';') {
                context.consume();
                context.push(SEMICOLON);
                return;
            }
            break;

        case '#':
            consume_color_literal(context);
            return;
    }

    // Fall through, consume the next letter as UNRECOGNIZED
    std::stringstream text;
    text << context.consume();
    context.push(UNRECOGNIZED, text.str());
}

void consume_identifier(TokenizeContext &context)
{
    std::stringstream text;

    while (is_acceptable_inside_identifier(context.next()))
        text << context.consume();

    context.push(IDENTIFIER, text.str());
}

void consume_whitespace(TokenizeContext &context)
{
    std::stringstream text;

    while (is_whitespace(context.next())) {
        text << context.consume();
    }

    context.push(WHITESPACE, text.str());
}

void consume_comment(TokenizeContext& context)
{
    std::stringstream text;

    // consume the -- part
    text << context.consume();
    text << context.consume();

    while (!context.finished() && !is_newline(context.next()))
        text << context.consume();

    context.push(COMMENT, text.str());
}

bool match_number(TokenizeContext &context)
{
    int lookahead = 0;

    if (context.next(lookahead) == '.')
        lookahead++;

    if (is_number(context.next(lookahead)))
        return true;

    return false;
}

void consume_number(TokenizeContext &context)
{
    std::stringstream text;

    bool minus_sign = false;
    bool dot_encountered = false;

    if (context.next() == '-') {
        text << context.consume();
        minus_sign = true;
    }

    while (true) {
        if (is_number(context.next())) {
            text << context.consume();
        }
        else if (context.next() == '.') {
            // If we've already encountered a dot, finish and don't consume
            // this one.
            if (dot_encountered)
                break;

            // Special case: if this dot is followed by another dot, then it should
            // be tokenized as TWO_DOTS, so don't consume it here.
            if (context.next(1) == '.')
                break;

            text << context.consume();
            dot_encountered = true;
        }
        else {
            break;
        }
    }

    if (dot_encountered)
        context.push(FLOAT_TOKEN, text.str());
    else
        context.push(INTEGER, text.str());
}

void consume_hex_number(TokenizeContext &context)
{
    std::stringstream text;

    // consume the 0x part
    text << context.consume();
    text << context.consume();

    while (is_hexadecimal_digit(context.next()))
        text << context.consume();

    context.push(HEX_INTEGER, text.str());
}

void consume_string_literal(TokenizeContext &context)
{
    std::stringstream text;

    // Consume starting quote, this can be ' or "
    char quote_type = context.consume();
    text << quote_type;

    bool escapedNext = false;
    while (!context.finished()) {
        char c = context.next();

        if (c == quote_type && !escapedNext)
            break;

        escapedNext = false;
        if (c == '\\')
            escapedNext = true;

        text << context.consume();
    }

    // consume ending quote
    text << context.consume();

    context.push(STRING, text.str());
}

void consume_triple_quoted_string_literal(TokenizeContext &context)
{
    std::stringstream text;

    // Consume initial <<<
    text << context.consume();
    text << context.consume();
    text << context.consume();

    while (!context.finished() &&
            !(context.next() == '>' && context.next(1) == '>' && context.next(2) == '>'))
        text << context.consume();

    // Consume closing >>>
    text << context.consume();
    text << context.consume();
    text << context.consume();

    context.push(STRING, text.str());
}

void consume_color_literal(TokenizeContext &context)
{
    std::stringstream text;

    // consume #
    text << context.consume();

    while (is_hexadecimal_digit(context.next()))
        text << context.consume();

    // acceptable lengths are 3, 4, 6 or 8 characters (not including #)
    std::string result = text.str();
    int length = int(result.length() - 1);

    if (length == 3 || length == 4 || length == 6 || length == 8)
        context.push(COLOR, result);
    else
        context.push(UNRECOGNIZED, result);
}

} // namespace token
} // namespace circa
