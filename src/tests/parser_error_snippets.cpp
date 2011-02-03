// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// In this test, we have a bunch of snippets, where we try to reproduce every
// possible parser error.
//
// The goal of this test is to make sure that for every error: 1) no exception
// is thrown, and 2) it's detected as an error. 
//
// (1) is a pretty common bug, since the parsing code makes liberal use of ca_assert()
//

#include <circa.h>

namespace circa {
namespace parser_error_snippets {

struct TestInput
{
    std::string text;
    bool exceptionThrown;
    bool failedToCauseError;

    TestInput(std::string const& _text) : text(_text),
        exceptionThrown(false), failedToCauseError(false) {}
};

std::vector<TestInput> TEST_INPUTS;

void register_input(std::string const& in)
{
    TEST_INPUTS.push_back(TestInput(in));
}

void register_every_possible_parse_error()
{
    TEST_INPUTS.clear();

    register_input("def");
    register_input("def myfunc");
    register_input("def myfunc%");
    register_input("def myfunc(%");
    register_input("def myfunc(int %");
    register_input("def myfunc(int) %");
    register_input("def myfunc(int) -> %");
    register_input("def myfunc(foo, bar) : baz");
    register_input("type");
    register_input("type mytype");
    register_input("type mytype %");
    register_input("type mytype { %");
    register_input("type mytype { int %");
    register_input("type mytype { int a %");
    register_input("type mytype { int a");
    register_input("if");
    register_input("if else");
    register_input("if true else else");
    register_input("if %");
    register_input("for");
    register_input("for %");
    register_input("for x");
    register_input("for x %");
    register_input("for x in");
    register_input("for x in %");
    register_input("for x in 1\n");
    register_input("namespace");
    register_input("namespace %");
    register_input("state");
    register_input("state %");
    register_input("state x = ");
    register_input("state int x = ");
    register_input("state int %");
    register_input("state foo x");
    register_input("foo = 1; state foo x");
    register_input("a.b 3 4 = 4");
    register_input("a.0");
    register_input("[].append %");
    register_input("[].append(%");
    register_input("[].append(");
    register_input("1 -> %");
    register_input("1 -> 2");
    register_input("(1 + 2");
    register_input("add %");
    register_input("add(");
    register_input("add( %");
    register_input("add(1");
    register_input("[");
    register_input("a = []; a[");
    register_input("a = [1]; a[0");
    register_input("a()");
    register_input("a = 1; a()");
    register_input("def func() -> NonexistantType end");
    register_input("nonexistant = nonexistant + 1");
    register_input("for i in 0..1 print([0 [0 ]) end");
    register_input("a = 1; a = x");
}

void test_every_parse_error()
{
    register_every_possible_parse_error();

    std::vector<TestInput>::iterator it;

    int problemCount = 0;

    for (it = TEST_INPUTS.begin(); it != TEST_INPUTS.end(); ++it) {
        TestInput &input = *it;

        Branch branch;

        try {
            parser::compile(&branch, parser::statement_list, input.text);
        } catch (std::exception const&) {
            input.exceptionThrown = true;
            problemCount++;
            continue;
        }

        if (!has_static_errors(branch)) {
            dump_branch(branch);
            input.failedToCauseError = true;
            problemCount++;
        }
    }

    if (problemCount > 0) {
        std::cout << "Encountered " << problemCount << " problem";
        std::cout << (problemCount == 1 ? "" : "s");
        std::cout << " in parser_error_snippets:" << std::endl;
        for (it = TEST_INPUTS.begin(); it != TEST_INPUTS.end(); ++it) {
            if (it->exceptionThrown)
                std::cout << "[EXCEPTION]";
            else if (it->failedToCauseError)
                std::cout << "[NO ERROR] ";
            else 
                std::cout << "[Success]  ";

            std::cout << " " << it->text << std::endl;
        }

        declare_current_test_failed();
    }
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_error_snippets::test_every_parse_error);
}

} // namespace parser_error_snippets
} // namespace circa
