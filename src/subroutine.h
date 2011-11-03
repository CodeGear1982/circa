// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

namespace subroutine_f {
    void format_source(StyledSource* source, Term* term);
}

CA_FUNCTION(evaluate_subroutine);

bool is_subroutine(Term* term);
Term* find_enclosing_subroutine(Term* term);
int get_input_index_of_placeholder(Term* inputPlaceholder);

// Perform various steps to finish creating a subroutine
void initialize_subroutine(Term* sub);
void finish_building_subroutine(Term* sub, Term* outputType);

void subroutine_check_to_append_implicit_return(Term* sub);

void store_locals(Branch* branch, TaggedValue* storage);
void restore_locals(TaggedValue* storageTv, Branch* branch);

void call_subroutine(Branch* sub, TaggedValue* inputs, TaggedValue* output, TaggedValue* error);
void call_subroutine(Term* sub, TaggedValue* inputs, TaggedValue* output, TaggedValue* error);


} // namespace circa
