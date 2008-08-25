
#include "common_headers.h"

#include "all_tests.h"
#include "common.h"
#include "errors.h"

namespace circa {

void register_branch_tests();
void register_builtin_function_tests();
void register_list_tests();
void register_parser_tests();
void register_primitive_type_tests();
void register_subroutine_tests();
void register_struct_tests();
void register_tokenizer_tests();

void run_all_tests()
{
    gTestCases.clear();

    register_branch_tests();
    register_builtin_function_tests();
    register_list_tests();
    register_parser_tests();
    register_primitive_type_tests();
    register_subroutine_tests();
    register_struct_tests();
    register_tokenizer_tests();

    std::vector<TestCase>::iterator it;
    int totalTestCount = (int) gTestCases.size();
    int numSuccess = 0;
    int numFailure = 0;
    for (it = gTestCases.begin(); it != gTestCases.end(); ++it) {
        try {
            it->execute();
            numSuccess++;
        }
        catch (errors::CircaError &err) {
            std::cout << "Error white running test case " << it->name << std::endl;
            std::cout << err.message() << std::endl;
            numFailure++;
        }
    }

    std::cout << "Ran " << totalTestCount << " tests, " << numSuccess <<
        " success(es) and " << numFailure << " failure(s)." << std::endl;
}

}
