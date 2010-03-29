// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_BUILTIN_TYPES_INCLUDED
#define CIRCA_BUILTIN_TYPES_INCLUDED

namespace circa {

namespace int_t {
    void setup_type(Type* type);
}

namespace float_t {
    bool equals(TaggedValue* a, TaggedValue* b);
    void setup_type(Type* type);
}

namespace bool_t {
    void setup_type(Type* type);
}

namespace set_t {
    void add(Branch& branch, Term* value);
    void setup_type(Type* type);
}

namespace old_list_t {
    void setup(Type*);
}

namespace dict_t {
    std::string to_string(Branch& branch);
}

namespace string_t {
    void setup_type(Type* type);
    void postponed_setup_type(Type* type);
}

namespace ref_t {
    void get_name(EvalContext*, Term* caller);
    void hosted_to_string(EvalContext*, Term* caller);
    void get_function(EvalContext*, Term* caller);
    void hosted_typeof(EvalContext*, Term* caller);
    void assign(EvalContext*, Term* caller);
    void tweak(EvalContext*, Term* caller);
    void asint(EvalContext*, Term* caller);
    void asfloat(EvalContext*, Term* caller);
    void get_input(EvalContext*, Term* caller);
    void num_inputs(EvalContext*, Term* caller);
    void get_source_location(EvalContext*, Term* caller);
    void setup_type(Type* type);
}

namespace type_t {
    void initialize(Type* type, TaggedValue* value);
    void assign(TaggedValue* source, TaggedValue* dest);
}

namespace point_t {

    // Helper functions:
    void read(Term* term, float* x, float* y);
    void write(Term* term, float x, float y);
}

void initialize_primitive_types(Branch& kernel);

// Do some more setup, after all the standard builtin types have been created.
void post_setup_primitive_types();

void setup_builtin_types(Branch& kernel);
void parse_builtin_types(Branch& kernel);

}

#endif
