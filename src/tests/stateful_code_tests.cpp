// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace stateful_code_tests {

void test_load_and_save()
{
    Branch branch;
    Term* statefulTerm = branch.eval("state i = 1");
    Term* value = create_value(NULL, BRANCH_TYPE);
    Term* value_i = create_value(&as_branch(value), INT_TYPE, "i");
    as_int(value_i) = 5;

    test_assert(as_int(statefulTerm) == 1);

    load_state_into_branch(value, branch);

    test_assert(as_int(statefulTerm) == 5);

    as_int(statefulTerm) = 11;

    persist_state_from_branch(branch, value);

    test_assert(as_int(value_i) == 11);
}

void test_get_type_from_branches_stateful_terms()
{
    Branch branch;
    branch.eval("a = 0");
    branch.eval("state b = 2.2");
    branch.eval("c = 'hello'");
    branch.eval("state d = false");

    Branch type;
    
    get_type_from_branches_stateful_terms(branch, type);

    test_assert(type.numTerms() == 2);
    test_assert(is_value(type[0]));
    test_assert(type[0]->type == FLOAT_TYPE);
    test_assert(is_value(type[0]));
    test_assert(type[1]->type == BOOL_TYPE);
}

void stateful_value_evaluation()
{
    Branch branch;
    Term *i = branch.eval("state i = 2.0");
    branch.eval("i = i + 1.0");
    wrap_up_branch(branch);

    test_equals(as_float(i), 2.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 3.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 4.0);
}

void initialize_from_expression()
{
    Branch branch;
    branch.eval("a = 1 + 2");
    branch.eval("b = a * 2");
    Term *c = branch.eval("state c = b");

    test_equals(as_float(c), 6);
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_code_tests::test_load_and_save);
    REGISTER_TEST_CASE(stateful_code_tests::test_get_type_from_branches_stateful_terms);
    REGISTER_TEST_CASE(stateful_code_tests::stateful_value_evaluation);
    REGISTER_TEST_CASE(stateful_code_tests::initialize_from_expression);
}

} // namespace stateful_code_tests

} // namespace circa
