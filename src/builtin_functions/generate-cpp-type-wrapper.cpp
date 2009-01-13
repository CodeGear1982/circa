// Copyright 2008 Paul Hodge

#include "branch.h"
#include "circa.h"
#include "introspection.h"

namespace circa {
namespace _function {

    void evaluate(Term* caller)
    {
        Branch& branch = as<Branch>(caller->input(0));
        std::stringstream result;

        for (int i=0; i < branch.numTerms(); i++) {

            Term* term = branch[i];

            if (!should_print_term_source_line(term))
                continue;

            result << get_term_source(term);

            if (i+1 < branch.numTerms()) {
                result << "\n";
            }
        }

        as_string(caller) = result.str();
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function branch-to-source-text(Branch) -> string");
        as_function(main_func).pureFunction = true;
    }
}
}
