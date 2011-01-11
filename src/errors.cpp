// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <cassert>

#include "branch_iterator.h"
#include "build_options.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "source_repro.h"
#include "type.h"

namespace circa {

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message)
{
    if (DEBUG_TRAP_ERROR_OCCURRED)
        ca_assert(false);

    ca_assert(errorTerm != NULL);

    if (context == NULL)
        throw std::runtime_error(message);

    if (!context->errorOccurred) {
        context->errorOccurred = true;
        context->errorTerm = errorTerm;
        context->errorMessage = message;
    }
}

void internal_error(const char* message)
{
    if (ASSERT_INTERNAL_ERROR) {
        std::cerr << "internal error: " << message << std::endl;
        assert(false);
    } else {
        throw std::runtime_error(message);
    }
}

void internal_error(std::string const& message)
{
    internal_error(message.c_str());
}

void ca_assert_function(bool expr, const char* exprStr, int line, const char* file)
{
    if (!expr) {
        std::stringstream msg;
        msg << "ca_assert(" << exprStr << ") failed in " << file << " line " << line;
        internal_error(msg.str().c_str());
    }
}

bool has_static_error(Term* term)
{
    if (term == NULL)
        return false;

    return get_static_error(term) != SERROR_NO_ERROR;
}

static StaticError get_static_error_for_input_index(Term* term, int index)
{
    int effectiveIndex = index;

    bool varArgs = function_t::get_variable_args(term->function);
    if (varArgs)
        effectiveIndex = 0;

    Term* input = term->inputs[index];
    bool meta = function_t::get_input_meta(term->function, effectiveIndex);
    bool optional = function_t::get_input_optional(term->function, effectiveIndex);
    Term* type = function_t::get_input_type(term->function, effectiveIndex);
    optional = optional || function_t::is_state_input(term->function, effectiveIndex);
     
    if (input == NULL) {
        if (meta || optional)
            return SERROR_NO_ERROR;
        else
            return SERROR_NULL_INPUT_TERM;
    }

    // Check type
    if (term_output_never_satisfies_type(input, unbox_type(type)))
        return SERROR_INPUT_TYPE_ERROR;

    return SERROR_NO_ERROR;
}

StaticError get_static_error(Term* term)
{
    if (term->function == NULL) 
        return SERROR_NULL_FUNCTION;

    bool varArgs = function_t::get_variable_args(term->function);
    int funcNumInputs = function_t::num_inputs(term->function);

    // Check # of inputs
    if (!varArgs && (term->inputs.length() != funcNumInputs))
        return SERROR_WRONG_NUMBER_OF_INPUTS;

    // Check each input. Make sure:
    //  - it is not null
    //  - it has a non-null value
    //  - it has no errors
    //  - it has the correct type
    for (int input=0; input < term->inputs.length(); input++) {
        StaticError error = get_static_error_for_input_index(term, input);
        if (error != SERROR_NO_ERROR)
            return error;
    }

    // This next section includes expected parser failures

    // Unknown function
    if (term->function == UNKNOWN_FUNCTION)
        return SERROR_UNKNOWN_FUNCTION;
    if (!is_function(term->function))
        return SERROR_UNKNOWN_FUNCTION;

    // Unknown type
    if (term->function == UNKNOWN_TYPE_FUNC)
        return SERROR_UNKNOWN_TYPE;

    // Unknown identifier
    if (term->function == UNKNOWN_IDENTIFIER_FUNC)
        return SERROR_UNKNOWN_IDENTIFIER;

    // Unrecognized expression
    if (term->function == UNRECOGNIZED_EXPRESSION_FUNC)
        return SERROR_UNRECGONIZED_EXPRESSION;

    if (term->function == UNKNOWN_FIELD_FUNC)
        return SERROR_UNKNOWN_FIELD;

    return SERROR_NO_ERROR;
}

std::string get_static_error_message(Term* term)
{
    StaticError error = get_static_error(term);

    std::stringstream out;

    switch (error) {
    case SERROR_NO_ERROR:
        return "(no error)";

    case SERROR_NULL_FUNCTION:
        return "Function is NULL: "+format_global_id(term);

    case SERROR_WRONG_NUMBER_OF_INPUTS: {
        int funcNumInputs = function_t::num_inputs(term->function);
        out << "Wrong number of inputs for function " << term->function->name
            << " (found " << term->inputs.length()
            << ", expected " << funcNumInputs << ")";
        return out.str();
    }

    case SERROR_NULL_INPUT_TERM:
        return "(null input term)"; // TODO
    case SERROR_INPUT_TYPE_ERROR:
    {
        int errorIndex = -1;
        for (int input=0; input < term->inputs.length(); input++) {
            StaticError error = get_static_error_for_input_index(term, input);
            if (error != SERROR_NO_ERROR)
                errorIndex = input;
        }

        out << "Input type " << term->input(errorIndex)->type->name
            << " doesn't fit in expected type "
            << function_t::get_input_type(term->function, errorIndex)->name
            << " (for arg " << errorIndex << " of function "
            << term->function->name << ")";
        return out.str();
    } 

    case SERROR_UNKNOWN_FUNCTION: {
        std::string functionName = term->stringProp("syntax:functionName");
        if (functionName == "")
            functionName = get_term_source_text(term->function);
        out << "Unknown function: " << functionName;
        return out.str();
    }
    case SERROR_UNKNOWN_TYPE:
        out << "Unknown type: " << term->name;
        return out.str();
    case SERROR_UNKNOWN_IDENTIFIER:
        out << "Unknown identifier: " << term->name;
        return out.str();
    case SERROR_UNKNOWN_FIELD:
        out << "Unknown field: " << term->stringProp("field-name");
        return out.str();
    case SERROR_UNRECGONIZED_EXPRESSION:
        out << "Unrecognized expression: " << term->stringProp("message");
        return out.str();
    }
    
    ca_assert(false);
    return "";
}

int count_static_errors(Branch& branch)
{
    int result = 0;
    for (BranchIterator it(branch); !it.finished(); ++it)
        if (has_static_error(*it))
            result++;

    return result;
}

bool has_static_errors(Branch& branch)
{
    for (BranchIterator it(branch); !it.finished(); ++it)
        if (has_static_error(*it))
            return true;
    return false;
}

void print_static_error_formatted(Term* term, std::ostream& output)
{
    output << get_short_location(term) << " " << get_static_error_message(term) << std::endl;
}

void print_static_errors_formatted(Branch& branch, std::ostream& output)
{
    int count  = count_static_errors(branch);

    output << count << " static error";
    if (count != 1) output << "s";
    output << ":\n";

    for (BranchIterator it(branch); !it.finished(); ++it) {
        if (has_static_error(*it))
            print_static_error_formatted(*it, output);
    } 
}

void print_runtime_error_formatted(EvalContext& context, std::ostream& output)
{
    output << get_short_location(context.errorTerm)
        << " " << context.errorMessage;
}

std::string get_static_errors_formatted(Branch& branch)
{
    std::stringstream strm;
    print_static_errors_formatted(branch, strm);
    return strm.str();
}

} // namespace circa
