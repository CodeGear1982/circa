// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace stateful_code_tests {

void test_is_get_state()
{
    Branch branch;
    Term* i = branch.compile("state int i");
    test_assert(is_get_state(i));
    test_assert(i->type == INT_TYPE);

    Term* j = branch.compile("state i = 0");
    test_assert(is_get_state(j));
}

void test_is_function_stateful()
{
    Branch branch;
    Term* f = branch.compile("def f() { state s }");

    test_assert(is_function_stateful(f));

    Term* g = branch.compile("def g() { 1 2 3 }");

    test_assert(!is_function_stateful(g));

    test_assert(has_implicit_state(branch.compile("f()")));
    test_assert(!has_implicit_state(branch.compile("g()")));
}

CA_FUNCTION(_empty_evaluate) {}

void test_get_type_from_branches_stateful_terms()
{
    Branch branch;
    branch.eval("a = 0");
    branch.eval("state number b");
    branch.eval("c = 'hello'");
    branch.eval("state bool d");

    Branch type;
    
    get_type_from_branches_stateful_terms(branch, type);

    test_assert(type.length() == 2);
    test_assert(is_value(type[0]));
    test_assert(type[0]->type == FLOAT_TYPE);
    test_assert(is_value(type[0]));
    test_assert(type[1]->type == BOOL_TYPE);
}

void initial_value()
{
    Branch branch;
    EvalContext context;

    Term* i = branch.compile("state i = 3");
    Term* j = branch.compile("state int j = 4");
    evaluate_branch(&context, branch);

    test_assert(is_int(get_local(i)));
    test_equals(get_local(i)->asInt(), 3);

    test_assert(is_int(get_local(j)));
    test_equals(get_local(j)->asInt(), 4);
}

void initialize_from_expression()
{
    Branch branch;
    branch.compile("a = 1 + 2");
    branch.compile("b = a * 2");
    Term *c = branch.compile("state c = b");

    test_assert(branch);

    evaluate_branch(branch);

    test_equals(to_float(c), 6);

    branch.clear();
    Term* d = branch.compile("d = 5");
    Term* e = branch.compile("state e = d");
    EvalContext context;
    evaluate_branch(&context, branch);
    test_equals(e->asInt(), 5);

    set_int(d, 10);
    evaluate_branch(&context, branch);
    test_equals(e->asInt(), 5);
}


int NEXT_UNIQUE_OUTPUT = 0;

CA_FUNCTION(_unique_output)
{
    set_int(OUTPUT, NEXT_UNIQUE_OUTPUT++);
}

List SPY_RESULTS;

CA_FUNCTION(_spy)
{
    copy(INPUT(0), SPY_RESULTS.append());
}

void one_time_assignment_inside_for_loop()
{
    Branch branch;

    import_function(branch, _unique_output, "unique_output() -> int");
    import_function(branch, _spy, "spy(int)");
    branch.compile("for i in [1 1 1] { state s = unique_output(); spy(s) }");
    test_assert(branch);

    NEXT_UNIQUE_OUTPUT = 0;
    SPY_RESULTS.clear();

    EvalContext context;
    evaluate_branch(&context, branch);

    test_equals(&SPY_RESULTS, "[0, 1, 2]");

    SPY_RESULTS.clear();
    evaluate_branch(&context, branch);

    test_equals(&SPY_RESULTS, "[0, 1, 2]");
}

void explicit_state()
{
    Branch branch;
    branch.compile("state s");
    branch.compile("s = 1");

    EvalContext context;
    evaluate_branch(&context, branch);

    test_equals(context.state.toString(), "[s: 1]");
}

void implicit_state()
{
    Branch branch;
    branch.compile("def f() { state s; s = 1 }");
    branch.compile("f()");

    EvalContext context;
    evaluate_branch(&context, branch);
    
    test_equals(context.state.toString(), "[_f: [s: 1]]");
}

namespace test_interpreted_state_access
{
    CA_FUNCTION(evaluate)
    {
        TaggedValue* state = get_state_input(CONTEXT, CALLER);
        change_type(state, &INT_T);
        set_int(state, as_int(state) + 1);
    }

    void test()
    {
        Branch branch;
        import_function(branch, evaluate, "func() -> void");
        Term* a = branch.compile("a = func()");

        test_equals(a->uniqueName.name, "a");

        EvalContext context;
        test_equals(context.state.toString(), "[]");

        evaluate_branch(&context, branch);
        test_equals(context.state.toString(), "[a: 1]");

        evaluate_branch(&context, branch);
        test_equals(context.state.toString(), "[a: 2]");

        evaluate_branch(&context, branch);
        test_equals(context.state.toString(), "[a: 3]");
    }
}

void bug_with_top_level_state()
{
    // This code once caused an assertion failure
    Branch branch;
    branch.compile("state s = 1");
    branch.compile("def f() { state t }");
    branch.compile("f()");
    evaluate_branch(branch);
}

void bug_with_state_and_plus_equals()
{
    Branch branch;
    branch.compile("state int count = 0; count += 1");

    EvalContext context;
    evaluate_branch(&context, branch);
}

void subroutine_unique_name_usage()
{
    Branch branch;
    branch.compile("def f() { state s = 0; s += 1; s += 2; s += 5 } f()");
    EvalContext context;
    evaluate_branch(&context, branch);

    test_equals(&context.state, "[_f: [s: 8]]");
}

void subroutine_early_return()
{
    Branch branch;
    branch.compile("def f()->int { state s = 2; return 0; s = 4; } f()");
    EvalContext context;
    evaluate_branch(&context, branch);
    test_equals(&context.state, "[_f: [s: 2]]");
}

void test_branch_has_inlined_state()
{
    Branch branch;

    test_assert(is_null(&branch.hasInlinedState));
    test_assert(!has_any_inlined_state(branch));
    test_assert(as_bool(&branch.hasInlinedState) == false);
    set_null(&branch.hasInlinedState);

    branch.compile("state int i");
    test_assert(has_any_inlined_state(branch));

    branch.clear();
    test_assert(!has_any_inlined_state(branch));

    Branch& nested = create_branch(branch);
    test_assert(!has_any_inlined_state(branch));
    test_assert(!has_any_inlined_state(nested));

    nested.compile("state i");
    test_assert(has_any_inlined_state(branch));
    test_assert(has_any_inlined_state(nested));

    nested.clear();
    test_assert(!has_any_inlined_state(nested));
    test_assert(!has_any_inlined_state(branch));
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_tests::test_is_get_state);
    REGISTER_TEST_CASE(stateful_code_tests::test_is_function_stateful);
    REGISTER_TEST_CASE(stateful_code_tests::test_get_type_from_branches_stateful_terms);
    REGISTER_TEST_CASE(stateful_code_tests::initial_value);
    REGISTER_TEST_CASE(stateful_code_tests::initialize_from_expression);
    REGISTER_TEST_CASE(stateful_code_tests::one_time_assignment_inside_for_loop);
    REGISTER_TEST_CASE(stateful_code_tests::explicit_state);
    REGISTER_TEST_CASE(stateful_code_tests::implicit_state);
    REGISTER_TEST_CASE(stateful_code_tests::test_interpreted_state_access::test);
    REGISTER_TEST_CASE(stateful_code_tests::bug_with_top_level_state);
    REGISTER_TEST_CASE(stateful_code_tests::bug_with_state_and_plus_equals);
    REGISTER_TEST_CASE(stateful_code_tests::subroutine_unique_name_usage);
    REGISTER_TEST_CASE(stateful_code_tests::subroutine_early_return);
    REGISTER_TEST_CASE(stateful_code_tests::test_branch_has_inlined_state);
}

} // namespace stateful_code_tests

} // namespace circa
