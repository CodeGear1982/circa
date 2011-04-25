// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "testing.h"
#include "circa.h"

namespace circa {
namespace feedback_tests {

void train_addition1()
{
    Branch branch;
    Term* a = branch.compile("a = 1.0");
    branch.compile("b = add(a, 2.0)");
    branch.compile("b <- 4");
    evaluate_branch(branch);

    set_trainable(a, true);

    test_equals(as_float(a), 1.0);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(as_float(a), 2.0);
}

void train_addition2()
{
    // In this test, we split a feedback signal to two trainable values
    Branch branch;
    Term* a = branch.compile("a = 1.0");
    Term* b = branch.compile("b = 2.0");
    branch.compile("add(a, b) <- 4");
    evaluate_branch(branch);

    set_trainable(a, true);
    set_trainable(b, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(to_float(a), 1.5);
    test_equals(to_float(b), 2.5);
}

void train_mult()
{
    Branch branch;
    Term* a = branch.compile("a = 1.0");
    branch.compile("b = 3");
    branch.compile("mult(a, b) <- 6");
    evaluate_branch(branch);

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(to_float(a), 2.0);
}

void train_cond()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    branch.compile("b = 1");
    Term* cond = branch.compile("c = true");
    branch.compile("cond(c, a, b) <- 5");
    evaluate_branch(branch);

    set_trainable(a, true);

    refresh_training_branch(branch);

    EvalContext context;
    evaluate_branch(&context, branch);
    test_assert(context);

    test_equals(a->asInt(), 5);

    // try changing cond
    set_int(a, 1);
    set_bool(cond, false);

    evaluate_branch(branch);

    test_assert(a->asInt() == 1);
}

void train_sin()
{
    Branch branch;
    Term* a = branch.compile("a = 0.0");
    branch.compile("sin(a) <- 1");
    evaluate_branch(branch);

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    // angles are in a range of 0..1. Otherwise this result would be PI/2
    test_equals(as_float(a), 0.25);
}

void train_cos()
{
    Branch branch;
    Term* a = branch.compile("a = 5.0");
    branch.eval("cos(a) <- 1");

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(as_float(a), 0);
}

void feedback_across_function()
{
    Branch branch;
    Term* a = branch.compile("a = 1.0");
    branch.compile("def inv(float f) : float\nreturn 0 - f\nend");
    branch.compile("b = inv(a)");
    branch.compile("b <- -2.0");
    evaluate_branch(branch);

    set_trainable(a, true);
    refresh_training_branch(branch);
}

void feedback_operation()
{
    FeedbackOperation operation;
    Branch branch;
    Term* a = branch.compile("1");
    Term* b = branch.compile("2");
    evaluate_branch(branch);

    TermList list = operation.getFeedback(a, DESIRED_VALUE_FEEDBACK);

    test_assert(list.length() == 0);

    operation.sendFeedback(a, b, DESIRED_VALUE_FEEDBACK);

    list = operation.getFeedback(a, DESIRED_VALUE_FEEDBACK);

    test_assert(list.length() == 1);
    test_assert(list[0] == b);
}

void register_tests()
{
    //TEST_DISABLED REGISTER_TEST_CASE(feedback_tests::train_addition1);
    //TEST_DISABLED REGISTER_TEST_CASE(feedback_tests::train_addition2);
    //TEST_DISABLED REGISTER_TEST_CASE(feedback_tests::train_mult);
    //TEST_DISABLED REGISTER_TEST_CASE(feedback_tests::train_cond);
    //TEST_DISABLED REGISTER_TEST_CASE(feedback_tests::train_sin);
    //TEST_DISABLED REGISTER_TEST_CASE(feedback_tests::train_cos);
    //TEST_DISABLED REGISTER_TEST_CASE(feedback_tests::feedback_operation);
}

} // namespace feedback_tests

} // namespace circa
