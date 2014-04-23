// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "kernel.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "selector.h"
#include "string_type.h"
#include "type.h"

namespace circa {

caValue* selector_advance(caValue* value, caValue* selectorElement, caValue* error)
{
    if (is_int(selectorElement)) {
        int selectorIndex = as_int(selectorElement);

        if (!is_list(value)) {
            set_error_string(error, "Value is not indexable: ");
            string_append_quoted(error, value);
            return NULL;
        }

        if (selectorIndex >= list_length(value)) {
            set_error_string(error, "Index ");
            string_append(error, selectorIndex);
            string_append(error, " is out of range");
            return NULL;
        }

        return get_index(value, selectorIndex);

    } else if (is_string(selectorElement)) {
        caValue* field = get_field(value, selectorElement, NULL);
        if (field == NULL) {
            set_error_string(error, "Field not found: ");
            string_append(error, selectorElement);
            return NULL;
        }
        return field;
    } else {
        set_error_string(error, "Unrecognized selector element: ");
        string_append_quoted(error, selectorElement);
        return NULL;
    }
}

Type* element_type_from_selector(Type* type, caValue* selectorElement)
{
    if (!is_struct_type(type))
        return TYPES.any;

    if (is_int(selectorElement)) {
        return compound_type_get_field_type(type, as_int(selectorElement));
    } else if (is_string(selectorElement)) {
        int index = list_find_field_index_by_name(type, as_cstring(selectorElement));
        return compound_type_get_field_type(type, index);
    }

    return TYPES.any;
}

caValue* get_with_selector(caValue* root, caValue* selector, caValue* error)
{
    caValue* element = root;
    ca_assert(is_null(error));

    for (int i=0; i < list_length(selector); i++) {
        caValue* selectorElement = list_get(selector, i);
        element = selector_advance(element, selectorElement, error);

        if (!is_null(error))
            return NULL;
    }

    return element;
}

void set_with_selector(caValue* value, caValue* selector, caValue* newValue, caValue* error)
{
    ca_assert(is_null(error));

    if (list_empty(selector)) {
        copy(newValue, value);
        return;
    }

    for (int selectorIndex=0;; selectorIndex++) {
        touch(value);
        caValue* selectorElement = list_get(selector, selectorIndex);
        caValue* element = selector_advance(value, selectorElement, error);

        if (!is_null(error))
            return;

        if (selectorIndex+1 == list_length(selector)) {
            copy(newValue, element);
            Type* elementType = element_type_from_selector(value->value_type, selectorElement);
            if (!cast(element, elementType)) {
                set_string(error, "Couldn't cast value ");
                string_append_quoted(error, newValue);
                string_append(error, " to type ");
                string_append(error, &elementType->name);
                string_append(error, " (element ");
                string_append_quoted(error, selectorElement);
                string_append(error, " of type ");
                string_append(error, &value->value_type->name);
                string_append(error, ")");
            }
            
            break;
        }

        value = element;
        // loop
    }
}

void evaluate_selector(caStack* stack)
{
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

bool is_accessor_function(Term* accessor)
{
    if (accessor->function == FUNCS.get_index
            || accessor->function == FUNCS.get_field)
        return true;

    if (accessor->function == FUNCS.dynamic_method)
        return true;

    // Future: We should be able to detect if a method behaves as an accessor, without
    // an explicit property.
    if (accessor->function->boolProp(sym_FieldAccessor, false))
        return true;
    
    return false;
}

bool term_is_accessor_traceable(Term* accessor)
{
    if (!has_empty_name(accessor))
        return false;

    if (accessor->function == FUNCS.get_index
            || accessor->function == FUNCS.get_field
            || is_copying_call(accessor)
            || accessor->function == FUNCS.dynamic_method
            || is_subroutine(accessor->function))
        return true;

    return false;
}

void trace_accessor_chain(Term* accessor, TermList* chainResult)
{
    chainResult->resize(0);

    while (true) {
        chainResult->append(accessor);

        if (!term_is_accessor_traceable(accessor))
            break;

        if (accessor->input(0) == NULL)
            break;

        // Continue the trace upward.
        accessor = accessor->input(0);
    }

    chainResult->reverse();
}

Term* find_accessor_head_term(Term* accessor)
{
    TermList chain;
    trace_accessor_chain(accessor, &chain);

    if (chain.length() == 0)
        return NULL;

    return chain[0];
}

Term* write_selector_for_accessor_chain(Block* block, TermList* chain)
{
    TermList selectorInputs;

    // Skip index 0 - this is the head term.
    
    for (int i=1; i < chain->length(); i++) {
        Term* term = chain->get(i);

        if (term->function == FUNCS.get_index
                || term->function == FUNCS.get_field) {

            selectorInputs.append(term->input(1));

        } else if (is_accessor_function(term)) {
            Term* element = create_string(block, term->stringProp(sym_Syntax_FunctionName, "").c_str());
            selectorInputs.append(element);
        }
    }

    return apply(block, FUNCS.selector, selectorInputs);
}

Term* rebind_possible_accessor(Block* block, Term* accessor, Term* result)
{
    // Check if this isn't a recognized accessor.
    if (!has_empty_name(accessor)) {
        // Just create a named copy of 'result'.
        return apply(block, FUNCS.copy, TermList(result), &accessor->nameValue);
    }

    TermList accessorChain;
    trace_accessor_chain(accessor, &accessorChain);

    Term* head = accessorChain[0];

    // Create the selector
    Term* selector = write_selector_for_accessor_chain(block, &accessorChain);

    Term* set = apply(block, FUNCS.set_with_selector,
            TermList(head, selector, result), &head->nameValue);

    set_declared_type(set, declared_type(head));
    return set;
}

Term* find_or_create_next_unnamed_term_output(Term* term)
{
    for (int i=0;; i++) {
        Term* output = get_output_term(term, i);
        if (output == NULL)
            return find_or_create_output_term(term, i);

        if (has_empty_name(output))
            return output;
    }

    internal_error("unreachable");
    return NULL;
}

Term* resolve_rebind_operators_in_inputs(Block* block, Term* term)
{
    for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
        Term* input = term->input(inputIndex);

        if (input == NULL)
            continue;

        // Walk upwards on 'input', see if one of the terms uses the @ operator.
        Term* head = input;
        Term* termBeforeHead = term;
        while (head->input(0) != NULL && term_is_accessor_traceable(head)) {
            termBeforeHead = head;
            head = head->input(0);
        }

        // Ignore term if there isn't a rebind.
        int inputIndexOfInterest = 0;
        if (termBeforeHead == term)
            inputIndexOfInterest = inputIndex;

        caValue* identifierRebindHint = term_get_input_property(termBeforeHead,
                inputIndexOfInterest, sym_Syntax_IdentifierRebind);
        if (head == NULL || has_empty_name(head)
                || identifierRebindHint == NULL
                || !as_bool(identifierRebindHint))
            continue;

        // Find the output term (may be an extra_output or may be 'term')
        Term* output = find_or_create_next_unnamed_term_output(term);

        if (input == head) {
            // No accessor expression, then just do a name rebind.
            rename(output, &head->nameValue);
            output->setBoolProp(sym_Syntax_ImplicitName, true);
        } else {
            // Create a set_with_selector expression.
            TermList accessorChain;
            trace_accessor_chain(input, &accessorChain);

            Term* selector = write_selector_for_accessor_chain(block, &accessorChain);

            Term* set = apply(block, FUNCS.set_with_selector,
                    TermList(head, selector, output), &head->nameValue);

            set_declared_type(set, declared_type(head));
            return set;
        }
    }

    return NULL;
}

void get_with_selector_evaluate(caStack* stack)
{
    caValue* root = circa_input(stack, 0);
    caValue* selector = circa_input(stack, 1);

    circa::Value error;

    caValue* result = get_with_selector(root, selector, &error);

    if (!is_null(&error)) {
        copy(&error, circa_output(stack, 0));
        raise_error(stack);
        return;
    }

    copy(result, circa_output(stack, 0));
}

void set_with_selector_evaluate(caStack* stack)
{
    caValue* out = circa_output(stack, 0);
    copy(circa_input(stack, 0), out);
    
    caValue* selector = circa_input(stack, 1);
    caValue* newValue = circa_input(stack, 2);

    circa::Value error;
    set_with_selector(out, selector, newValue, &error);

    if (!is_null(&error)) {
        circa_output_error_val(stack, &error);
        return;
    }
}

void selector_setup_funcs(Block* kernel)
{
    FUNCS.selector = 
        import_function(kernel, evaluate_selector, "selector(any elements :multiple) -> Selector");

    FUNCS.get_with_selector = 
        import_function(kernel, get_with_selector_evaluate,
            "get_with_selector(any object, Selector selector) -> any");

    FUNCS.set_with_selector =
        import_function(kernel, set_with_selector_evaluate,
            "set_with_selector(any object, Selector selector, any newValue) -> any");
}

} // namespace circa
