// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace set_field_function {

    CA_FUNCTION(evaluate)
    {
        copy(INPUT(0), OUTPUT);

        Term* head = CALLER;

        for (int nameIndex=2; nameIndex < NUM_INPUTS; nameIndex++) {
            std::string name = INPUT(nameIndex)->asString();
            int index = head->value_type->findFieldIndex(name);

            if (index == -1) {
                error_occurred(CONTEXT, CALLER, "field not found: "+name);
                return;
            }

            head = as_branch(head)[index];
        }

        if (head->type == ANY_TYPE)
            copy(INPUT(1), head);
        else
            cast(head->value_type, INPUT(1), head);
    }

    Term* specializeType(Term* caller)
    {
        return caller->input(0)->type;
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_source_for_input(source, term, 0);
        for (int i=2; i < term->numInputs(); i++) {
            append_phrase(source, ".", term, phrase_type::UNDEFINED);
            append_phrase(source, term->input(i)->asString().c_str(),
                    term, phrase_type::UNDEFINED);
        }
        append_phrase(source, " =", term, phrase_type::UNDEFINED);
        format_source_for_input(source, term, 1);
    }

    void setup(Branch& kernel)
    {
        SET_FIELD_FUNC = import_function(kernel, evaluate,
                "set_field(any, any, string...) -> any");
        function_t::get_attrs(SET_FIELD_FUNC).specializeType = specializeType;
        function_t::get_attrs(SET_FIELD_FUNC).formatSource = formatSource;
    }
}
}