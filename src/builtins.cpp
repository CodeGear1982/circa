// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "importing_macros.h"

#include "circa.h"
#include "heap_debugging.h"
#include "types/any.h"
#include "types/bool.h"
#include "types/branch.h"
#include "types/callable.h"
#include "types/color.h"
#include "types/common.h"
#include "types/dict.h"
#include "types/eval_context.h"
#include "types/float.h"
#include "types/handle.h"
#include "types/indexable.h"
#include "types/int.h"
#include "types/list.h"
#include "types/hashtable.h"
#include "types/ref.h"
#include "types/set.h"
#include "types/string.h"
#include "types/symbol.h"
#include "types/void.h"

namespace circa {

// setup_functions is defined in generated/setup_builtin_functions.cpp
void setup_builtin_functions(Branch&);

extern "C" {

// BUILTIN_SCRIPT_TEXT is defined in generated/builtin_script_text.cpp
extern const char* BUILTIN_SCRIPT_TEXT;

Branch* KERNEL = NULL;

Term* ADD_FUNC = NULL;
Term* ADDITIONAL_OUTPUT_FUNC = NULL;
Term* ALIAS_FUNC = NULL;
Term* ASSIGN_FUNC = NULL;
Term* APPLY_FEEDBACK = NULL;
Term* AVERAGE_FUNC = NULL;
Term* BRANCH_FUNC = NULL;
Term* BREAK_FUNC = NULL;
Term* CAST_FUNC = NULL;
Term* COMMENT_FUNC = NULL;
Term* CONTINUE_FUNC = NULL;
Term* COPY_FUNC = NULL;
Term* DESIRED_VALUE_FEEDBACK = NULL;
Term* DISCARD_FUNC = NULL;
Term* DIV_FUNC = NULL;
Term* DO_ONCE_FUNC = NULL;
Term* ERRORED_FUNC = NULL;
Term* FEEDBACK_FUNC = NULL;
Term* FINISH_MINOR_BRANCH_FUNC = NULL;
Term* FREEZE_FUNC = NULL;
Term* FOR_FUNC = NULL;
Term* GET_FIELD_FUNC = NULL;
Term* GET_INDEX_FUNC = NULL;
Term* GET_INDEX_FROM_BRANCH_FUNC = NULL;
Term* GET_STATE_FIELD_FUNC = NULL;
Term* IF_FUNC = NULL;
Term* IF_BLOCK_FUNC = NULL;
Term* COND_FUNC = NULL;
Term* INCLUDE_FUNC = NULL;
Term* INPUT_PLACEHOLDER_FUNC = NULL;
Term* JOIN_FUNC = NULL;
Term* LAMBDA_FUNC = NULL;
Term* LIST_TYPE = NULL;
Term* LIST_FUNC = NULL;
Term* MULT_FUNC = NULL;
Term* NAMESPACE_FUNC = NULL;
Term* NEG_FUNC = NULL;
Term* NOT_FUNC = NULL;
Term* ONE_TIME_ASSIGN_FUNC = NULL;
Term* OVERLOADED_FUNCTION_FUNC = NULL;
Term* PRESERVE_STATE_RESULT_FUNC = NULL;
Term* SET_FIELD_FUNC = NULL;
Term* SET_INDEX_FUNC = NULL;
Term* SUBROUTINE_OUTPUT_FUNC = NULL;
Term* STATEFUL_VALUE_FUNC = NULL;
Term* SUB_FUNC = NULL;
Term* RANGE_FUNC = NULL;
Term* REF_FUNC = NULL;
Term* RETURN_FUNC = NULL;
Term* UNKNOWN_FUNCTION = NULL;
Term* UNKNOWN_FIELD_FUNC = NULL;
Term* UNKNOWN_IDENTIFIER_FUNC = NULL;
Term* UNKNOWN_TYPE_FUNC = NULL;
Term* UNRECOGNIZED_EXPRESSION_FUNC = NULL;
Term* UNSAFE_ASSIGN_FUNC = NULL;
Term* VALUE_FUNC = NULL;

Term* ANY_TYPE = NULL;
Term* BOOL_TYPE = NULL;
Term* DICT_TYPE = NULL;
Term* FLOAT_TYPE = NULL;
Term* INT_TYPE = NULL;
Term* NULL_T_TERM = NULL;
Term* RECT_I_TYPE_TERM = NULL;
Term* REF_TYPE = NULL;
Term* STRING_TYPE = NULL;
Term* COLOR_TYPE = NULL;
Term* FEEDBACK_TYPE = NULL;
Term* FUNCTION_TYPE = NULL;
Term* FUNCTION_ATTRS_TYPE = NULL;
Term* MAP_TYPE = NULL;
Term* TYPE_TYPE = NULL;
Term* VOID_TYPE = NULL;
Term* OPAQUE_POINTER_TYPE;

} // extern "C"

// Builtin type objects:
Type BOOL_T;
Type BRANCH_T;
Type DICT_T;
Type ERROR_T;
Type EVAL_CONTEXT_T;
Type FLOAT_T;
Type FUNCTION_T;
Type HANDLE_T;
Type INT_T;
Type LIST_T;
Type NULL_T;
Type OPAQUE_POINTER_T;
Type REF_T;
Type STRING_T;
Type SYMBOL_T;
Type TYPE_T;
Type VOID_T;

// Builtin symbols:
TaggedValue OUT_SYMBOL;
TaggedValue REPEAT_SYMBOL;

Type* FILE_SIGNATURE_T;

bool STATIC_INITIALIZATION_FINISHED = false;
bool FINISHED_BOOTSTRAP = false;
bool SHUTTING_DOWN = false;

Branch& kernel()
{
    return *KERNEL;
}

CA_FUNCTION(empty_evaluate_function)
{
    set_null(OUTPUT);
}

void create_primitive_types()
{
    null_t::setup_type(&NULL_T);
    bool_t::setup_type(&BOOL_T);
    branch_t::setup_type(&BRANCH_T);
    dict_t::setup_type(&DICT_T);
    eval_context_t::setup_type(&EVAL_CONTEXT_T);
    float_t::setup_type(&FLOAT_T);
    handle_t::setup_type(&HANDLE_T);
    int_t::setup_type(&INT_T);
    list_t::setup_type(&LIST_T);
    string_t::setup_type(&STRING_T);
    symbol_t::setup_type(&SYMBOL_T);
    ref_t::setup_type(&REF_T);
    void_t::setup_type(&VOID_T);
    opaque_pointer_t::setup_type(&OPAQUE_POINTER_T);

    // errors are just stored as strings for now
    string_t::setup_type(&ERROR_T);

    symbol_t::assign_new_symbol("repeat", &REPEAT_SYMBOL);
    symbol_t::assign_new_symbol("out", &OUT_SYMBOL);
}

void update_bootstrapped_term(Term* term)
{
    term->evaluateFunc = derive_evaluate_func(term);
    update_input_instructions(term);
}

void bootstrap_kernel()
{
    // Create the very first building blocks. Most of the building functions in Circa
    // require a few kernel terms to already be defined. So in this function, we
    // create these required terms manually.

    KERNEL = new Branch();

    // Create value function
    VALUE_FUNC = KERNEL->appendNew();
    KERNEL->bindName(VALUE_FUNC, "value");

    // Create Type type
    TYPE_TYPE = KERNEL->appendNew();
    TYPE_TYPE->function = VALUE_FUNC;
    TYPE_TYPE->type = TYPE_TYPE;
    TYPE_TYPE->value_type = &TYPE_T;
    TYPE_TYPE->value_data.ptr = &TYPE_T;
    TYPE_T.name = "Type";
    TYPE_T.storageType = STORAGE_TYPE_TYPE;
    TYPE_T.initialize = type_t::initialize;
    TYPE_T.release = type_t::release;
    TYPE_T.copy = type_t::copy;
    TYPE_T.remapPointers = type_t::remap_pointers;
    TYPE_T.formatSource = type_t::formatSource;
    KERNEL->bindName(TYPE_TYPE, "Type");

    // Create Any type
    ANY_TYPE = KERNEL->appendNew();
    ANY_TYPE->function = VALUE_FUNC;
    ANY_TYPE->type = TYPE_TYPE;
    change_type(ANY_TYPE, &TYPE_T);
    any_t::setup_type(unbox_type(ANY_TYPE));
    KERNEL->bindName(ANY_TYPE, "any");

    // Create FunctionAttrs type
    FUNCTION_ATTRS_TYPE = KERNEL->appendNew();
    FUNCTION_ATTRS_TYPE->function = VALUE_FUNC;
    FUNCTION_ATTRS_TYPE->type = TYPE_TYPE;
    change_type(FUNCTION_ATTRS_TYPE, &TYPE_T);
    as_type(FUNCTION_ATTRS_TYPE)->name = "FunctionAttrs";
    as_type(FUNCTION_ATTRS_TYPE)->initialize = function_attrs_t::initialize;
    as_type(FUNCTION_ATTRS_TYPE)->copy = function_attrs_t::copy;
    as_type(FUNCTION_ATTRS_TYPE)->release = function_attrs_t::release;
    KERNEL->bindName(FUNCTION_ATTRS_TYPE, "FunctionAttrs");
    ca_assert(is_type(FUNCTION_ATTRS_TYPE));

    // Create Function type
    function_t::setup_type(&FUNCTION_T);
    FUNCTION_TYPE = create_type(*KERNEL, "Function");
    set_type(FUNCTION_TYPE, &FUNCTION_T);

    // Initialize TaggedValue func
    VALUE_FUNC->type = FUNCTION_TYPE;
    VALUE_FUNC->function = VALUE_FUNC;
    change_type((TaggedValue*)VALUE_FUNC, unbox_type(FUNCTION_TYPE));

    // Update locals so that debugging checks don't complain.
    refresh_locals_indices(*KERNEL);

    update_bootstrapped_term(VALUE_FUNC);
    update_bootstrapped_term(TYPE_TYPE);
    update_bootstrapped_term(ANY_TYPE);
    update_bootstrapped_term(FUNCTION_ATTRS_TYPE);
    update_bootstrapped_term(FUNCTION_TYPE);
}

void initialize_primitive_types(Branch& kernel)
{
    STRING_TYPE = create_type(kernel, "string");
    set_type(STRING_TYPE, &STRING_T);

    INT_TYPE = create_type(kernel, "int");
    set_type(INT_TYPE, &INT_T);

    FLOAT_TYPE = create_type(kernel, "number");
    set_type(FLOAT_TYPE, &FLOAT_T);

    NULL_T_TERM = create_type(kernel, "Null");
    set_type(NULL_T_TERM, &NULL_T);

    DICT_TYPE = create_type(kernel, "Dict");
    set_type(DICT_TYPE, &DICT_T);

    BOOL_TYPE = create_type(kernel, "bool");
    set_type(BOOL_TYPE, &BOOL_T);

    REF_TYPE = create_type(kernel, "Ref");
    set_type(REF_TYPE, &REF_T);

    VOID_TYPE = create_type(kernel, "void");
    set_type(VOID_TYPE, &VOID_T);

    LIST_TYPE = create_type(kernel, "List");
    set_type(LIST_TYPE, &LIST_T);

    OPAQUE_POINTER_TYPE = create_type(kernel, "opaque_pointer");
    set_type(OPAQUE_POINTER_TYPE, &OPAQUE_POINTER_T);

    Term* branchType = create_type(kernel, "Branch");
    set_type(branchType, &BRANCH_T);

    // ANY_TYPE was created in bootstrap_kernel
}

void post_initialize_primitive_types(Branch& kernel)
{
    // Properly setup value() func
    initialize_function(VALUE_FUNC);

    FunctionAttrs* attrs = get_function_attrs(VALUE_FUNC);
    attrs->outputTypes = TermList(ANY_TYPE);
    attrs->evaluate = value_function::evaluate;

    ca_assert(function_get_output_type(VALUE_FUNC, 0) == ANY_TYPE);
}

void pre_setup_types(Branch& kernel)
{
    // Declare input_placeholder first because it's used while compiling functions
    INPUT_PLACEHOLDER_FUNC = import_function(kernel, NULL, "input_placeholder() -> any");
    ADDITIONAL_OUTPUT_FUNC = import_function(kernel, empty_evaluate_no_touch_output,
            "additional_output() -> any");

    // FileSignature is used in some builtin functions
    FILE_SIGNATURE_T = unbox_type(parse_type(kernel,
            "type FileSignature { string filename, int time_modified }"));

    namespace_function::early_setup(kernel);
}

void initialize_compound_types(Branch& kernel)
{
    Term* set_type = create_compound_type(kernel, "Set");
    set_t::setup_type(unbox_type(set_type));

    Term* map_type = parse_type(kernel, "type Map;");
    hashtable_t::setup_type(unbox_type(map_type));

    Term* styledSourceType = parse_type(kernel, "type StyledSource;");
    styled_source_t::setup_type(unbox_type(styledSourceType));

    Term* indexableType = parse_type(kernel, "type Indexable;");
    indexable_t::setup_type(unbox_type(indexableType));

    callable_t::setup_type(unbox_type(parse_type(kernel, "type Callable;")));

    RECT_I_TYPE_TERM = parse_type(kernel, "type Rect_i { int x1, int y1, int x2, int y2 }");
}

void pre_setup_builtin_functions(Branch& kernel)
{
    return_function::setup(kernel);
}

void post_setup_functions(Branch& kernel)
{
    // Create vectorized add() functions
    Term* add_v = create_duplicate(kernel, kernel["vectorize_vv"], "add_v");
    set_ref(function_get_parameters(add_v), ADD_FUNC);
    overloaded_function::append_overload(ADD_FUNC, add_v);

    Term* add_s = create_duplicate(kernel, kernel["vectorize_vs"], "add_s");
    set_ref(function_get_parameters(add_s), ADD_FUNC);
    overloaded_function::append_overload(ADD_FUNC, add_s);

    // Create vectorized sub() functions
    Term* sub_v = create_duplicate(kernel, kernel["vectorize_vv"], "sub_v");
    set_ref(function_get_parameters(sub_v), SUB_FUNC);
    overloaded_function::append_overload(SUB_FUNC, sub_v);

    Term* sub_s = create_duplicate(kernel, kernel["vectorize_vs"], "sub_s");
    set_ref(function_get_parameters(sub_s), SUB_FUNC);
    overloaded_function::append_overload(SUB_FUNC, sub_s);

    // Create vectorized mult() functions
    Term* mult_v = create_duplicate(kernel, kernel["vectorize_vv"], "mult_v");
    set_ref(function_get_parameters(mult_v), kernel["mult"]);
    overloaded_function::append_overload(MULT_FUNC, mult_v);

    Term* mult_s = create_duplicate(kernel, kernel["vectorize_vs"], "mult_s");
    set_ref(function_get_parameters(mult_s), kernel["mult"]);
    overloaded_function::append_overload(MULT_FUNC, mult_s);

    // Create vectorized div() function
    Term* div_s = create_duplicate(kernel, kernel["vectorize_vs"], "div_s");
    set_ref(function_get_parameters(div_s), DIV_FUNC);
    overloaded_function::append_overload(DIV_FUNC, div_s);
}

void parse_hosted_types(Branch& kernel)
{
    parse_type(kernel, "type Point { number x, number y }");
    parse_type(kernel, "type Point_i { int x, int y }");
    parse_type(kernel, "type Rect { number x1, number y1, number x2, number y2 }");

    COLOR_TYPE = parse_type(kernel, "type Color { number r, number g, number b, number a }");

    color_t::setup_type(unbox_type(COLOR_TYPE));
}

void parse_builtin_script(Branch& kernel)
{
    parser::compile(kernel, parser::statement_list, BUILTIN_SCRIPT_TEXT);
}

} // namespace circa

using namespace circa;

export_func void circa_initialize()
{
    FINISHED_BOOTSTRAP = false;
    STATIC_INITIALIZATION_FINISHED = true;

    create_primitive_types();
    bootstrap_kernel();
    initialize_primitive_types(*KERNEL);
    post_initialize_primitive_types(*KERNEL);
    pre_setup_types(*KERNEL);
    initialize_compound_types(*KERNEL);

    FINISHED_BOOTSTRAP = true;

    pre_setup_builtin_functions(*KERNEL);
    setup_builtin_functions(*KERNEL);
    post_setup_functions(*KERNEL);
    parse_hosted_types(*KERNEL);

    type_initialize_kernel(*KERNEL);

    parse_builtin_script(*KERNEL);

#if CIRCA_TEST_BUILD
    // Create a space for unit tests.
    create_branch(*KERNEL, "_test_root");
#endif

    // Finally, make sure there are no static errors.
    if (has_static_errors(*KERNEL)) {
        std::cout << "Static errors found in kernel:" << std::endl;
        print_static_errors_formatted(*KERNEL, std::cout);
        return;
    }
}

export_func void circa_shutdown()
{
    SHUTTING_DOWN = true;

    clear_type_contents(&BOOL_T);
    clear_type_contents(&DICT_T);
    clear_type_contents(&ERROR_T);
    clear_type_contents(&FLOAT_T);
    clear_type_contents(&INT_T);
    clear_type_contents(&LIST_T);
    clear_type_contents(&NULL_T);
    clear_type_contents(&OPAQUE_POINTER_T);
    clear_type_contents(&REF_T);
    clear_type_contents(&STRING_T);
    clear_type_contents(&TYPE_T);
    clear_type_contents(&VOID_T);
    clear_contents_of_every_permanent_type();

    delete KERNEL;
    KERNEL = NULL;

    delete_every_permanent_type();
}
