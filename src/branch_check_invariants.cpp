// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "branch.h"
#include "building.h"
#include "introspection.h"
#include "term.h"

#include "branch_check_invariants.h"

namespace circa {

const int INTERNAL_ERROR_TYPE = 1;

void append_internal_error(BranchInvariantCheck* result, int index, std::string const& message)
{
    List& error = *make_list(result->errors.append(), 3);
    make_int(error[0], INTERNAL_ERROR_TYPE);
    make_int(error[1], index);
    make_string(error[2], message);
}

void branch_check_invariants(BranchInvariantCheck* result, Branch& branch)
{
    int nextExpectedRegisterIndex = 0;

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (term == NULL) {
            append_internal_error(result, i, "NULL pointer");
            continue;
        }

        int regCount = get_register_count(term);
        int expectedRegister = -1;

        if (regCount != 0) {
            expectedRegister = nextExpectedRegisterIndex;
            nextExpectedRegisterIndex += regCount;
        }

        if (term->registerIndex != expectedRegister) {
            std::stringstream msg;
            msg << "registerIndex unexpected value, registerIndex = " << term->registerIndex
                << ", expected = " << expectedRegister;
            append_internal_error(result, i, msg.str());
        }

        if (term->registerIndex >= branch.registerCount) {
            std::stringstream msg;
            msg << "registerIndex above limit, registerIndex = " << term->registerIndex
                << ", branch.registerCount = " << branch.registerCount;
            append_internal_error(result, i, msg.str());
        }
    }
} 

bool branch_check_invariants_print_result(Branch& branch, std::ostream& out)
{
    BranchInvariantCheck result;
    branch_check_invariants(&result, branch);

    if (result.errors.length() == 0)
        return true;

    out << result.errors.length() << " errors found in branch " << &branch
        << std::endl;

    for (int i=0; i < result.errors.length(); i++) {
        List* error = List::checkCast(result.errors[i]);
        out << "[" << error->get(1)->asInt() << "] ";
        out << error->get(2)->asString();
        out << std::endl;
    }

    std::cout << "contents:" << std::endl;
    print_branch_raw(out, branch);

    return false;
}

} // namespace circa