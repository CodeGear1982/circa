// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "cpp_codegen.h"
#include "function.h"
#include "introspection.h"
#include "source_repro.h"
#include "term.h"
#include "type.h"

namespace circa {
namespace cpp_codegen {

void write_type_name(CppWriter& writer, Type* type)
{
    if (type->name == "number")
        writer.write("float");
    else if (type->name == "string")
        writer.write("std::string");
    else
        writer.write(type->name);
}

void write_value(CppWriter& writer, Term* term)
{
    writer.write(term->toString());
}

void write_function(CppWriter& writer, Term* term)
{
    FunctionAttrs* func = get_function_attrs(term);

    write_type_name(writer, function_get_output_type(func, 0));
    writer.write(" ");
    writer.write(term->name);
    writer.write("(");

    for (int i=0; i < function_num_inputs(func); i++) {
        if (i != 0) writer.write(", ");
        write_type_name(writer, function_get_input_type(func, i));
        writer.write(" ");
        writer.write(function_get_input_name(func, i));
    }
    writer.write(")");
    writer.newline();
    writer.write("{");
    writer.indent();
    writer.newline();
    write_branch_contents(writer, nested_contents(term));
    writer.unindent();
    writer.write("}");
}

void write_expression(CppWriter& writer, Term* term)
{
    if (is_value(term)) {
        write_value(writer, term);
    } else if (term->stringProp("syntax:declarationStyle") == "infix") {
        write_expression(writer, term->input(0));
        writer.write(" ");
        writer.write(term->stringProp("syntax:functionName"));
        writer.write(" ");
        write_expression(writer, term->input(1));
    } else {

        writer.write(term->function->name);
        writer.write("(");

        for (int i=0; i < term->numInputs(); i++) {
            if (i != 0) writer.write(", ");
            write_expression(writer, term->input(0));
        }
        writer.write(")");
    }
}

void write_statement(CppWriter& writer, Term* term)
{
    if (is_comment(term)) {
        if (term->stringProp("comment") != "") {
            writer.write("//");
            writer.write(term->stringProp("comment"));
        }
    } else if (is_function(term)) {
        write_function(writer, term);
    } else if (is_statement(term)) {
        if (term->name != "") {
            write_type_name(writer, term->type);
            writer.write(" ");
            writer.write(term->name);
            writer.write(" = ");
        }
        write_expression(writer, term);
        writer.write(";");
    }
}

void write_branch_contents(CppWriter& writer, Branch* branch)
{
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (!should_print_term_source_line(term))
            continue;
        if (is_comment(term) && term->stringProp("comment") == "")
            continue;
        write_statement(writer, term);

        if (i+1 < branch->length())
            writer.newline();
    }
}

} // namespace circa 
} // namespace cpp_codegen
