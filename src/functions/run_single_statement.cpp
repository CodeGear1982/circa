// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace run_single_statement_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(run_single_statement, "run_single_statement(BranchRef, int)")
    {
        Branch* branchPtr = branch_ref_function::deref(INPUT(0));
        int index = INT_INPUT(1);

        if (branchPtr == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        Branch& branch = *branchPtr;

        // Find the nth statement in this branch
        for (int i=0; i < branch.length(); i++) {
            if (!is_statement(branch[i]) || is_comment(branch[i]))
                continue;

            if (index == 0) {
                evaluate_minimum(CONTEXT, branch[i]);
                break;
            }

            index -= 1;
        }

        set_null(OUTPUT);
    }

    
    CA_DEFINE_FUNCTION(get_statement_count, "get_statement_count(BranchRef br) -> int")
    {
        Branch* branchPtr = branch_ref_function::deref(INPUT(0));

        if (branchPtr == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        Branch& branch = *branchPtr;

        int count = 0;
        for (int i=0; i < branch.length(); i++) {
            if (!is_statement(branch[i]) || is_comment(branch[i]))
                continue;
            count++;
        }
        set_int(OUTPUT, count);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}