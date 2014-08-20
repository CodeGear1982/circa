// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "block.h"
#include "hashtable.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "function.h"
#include "importing.h"
#include "migration.h"
#include "modules.h"
#include "reflection.h"
#include "string_type.h"
#include "type.h"
#include "world.h"

namespace interpreter_test {

void test_cast_first_inputs()
{
    // Pass an input of [1] to a block that expects a struct.
    // The function will need to cast the [1] to T in order for it to work.

    Block block;
    block.compile("struct T { int i }");
    Term* f = block.compile("def f(T t) -> int { return t.i }");

    Stack* stack = create_stack(test_world());
    stack_init(stack, nested_contents(f));

    Value* in = circa_input(stack, 0);
    circa_set_list(in, 1);
    circa_set_int(circa_index(in, 0), 5);

    circa_run(stack);

    test_assert(circa_int(circa_output(stack, 0)) == 5);
}

void run_block_after_additions()
{
    Block block;

    // Create a block and run it.
    block.compile("a = 1");
    block.compile("test_spy(a)");
    block.compile("b = a + 2");
    block.compile("test_spy(b)");

    test_spy_clear();

    Stack* stack = create_stack(test_world());
    stack_init(stack, &block);

    circa_run(stack);

    test_equals(test_spy_get_results(), "[1, 3]");

    // Add some more stuff to the block, and run it. The Stack was not modified,
    // so it should continue where we stopped.
    block.compile("c = 4");
    block.compile("test_spy(c)");
    block.compile("d = a + b + c");
    block.compile("test_spy(d)");

    test_spy_clear();
    circa_run(stack);

    test_equals(test_spy_get_results(), "[4, 8]");
}

void bug_stale_bytecode_after_migrate()
{
    // There was a bug where Stack was holding on to stale bytecode, which caused
    // problems when the Block was migrated.

    Block version1;
    version1.compile("test_spy(1)");

    Block version2;
    version2.compile("test_spy(2)");

    Stack* stack = create_stack(test_world());
    stack_init(stack, &version1);

    test_spy_clear();
    circa_run(stack);
    test_equals(test_spy_get_results(), "[1]");

    stack_restart(stack);
    migrate_stack(stack, &version1, &version2);
    test_spy_clear();
    circa_run(stack);
    test_equals(test_spy_get_results(), "[2]");
}

void bug_restart_dies_after_code_delete()
{
    Block version1;
    version1.compile("1 + 2");
    version1.compile("3 + 4");
    version1.compile("5 + 6");
    version1.compile("assert(false)");

    Block version2;
    version1.compile("1");

    Stack* stack = create_stack(test_world());
    stack_init(stack, &version1);
    circa_run(stack);

    migrate_stack(stack, &version1, &version2);

    // This was causing a crash, internal NULL deref.
    stack_restart(stack);
}

void test_set_env()
{
    Block block;
    block.compile("test_spy(env(:a) + 5)");

    Stack* stack = create_stack(test_world());
    stack_init(stack, &block);

    set_int(circa_env_insert(stack, "a"), 5);
    test_spy_clear();
    circa_run(stack);

    test_equals(test_spy_get_results(), "[10]");
}

void test_that_stack_is_implicitly_restarted_in_run_interpreter()
{
    Block block;
    compile(&block, "test_spy(1)");

    test_spy_clear();

    Stack* stack = create_stack(test_world());
    stack_init(stack, &block);

    circa_run(stack);

    test_equals(test_spy_get_results(), "[1]");

    circa_run(stack);
    circa_run(stack);
    circa_run(stack);

    test_equals(test_spy_get_results(), "[1, 1, 1, 1]");
}

void register_tests()
{
    REGISTER_TEST_CASE(interpreter_test::test_cast_first_inputs);
    REGISTER_TEST_CASE(interpreter_test::bug_stale_bytecode_after_migrate);
    REGISTER_TEST_CASE(interpreter_test::bug_restart_dies_after_code_delete);
    REGISTER_TEST_CASE(interpreter_test::test_set_env);
    REGISTER_TEST_CASE(interpreter_test::test_that_stack_is_implicitly_restarted_in_run_interpreter);
}

} // namespace interpreter_test
