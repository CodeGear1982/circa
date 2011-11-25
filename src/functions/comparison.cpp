// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace comparison_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(less_than_i, "less_than_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) < INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(less_than_f, "less_than_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) < FLOAT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(less_than_eq_i, "less_than_eq_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) <= INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(less_than_eq_f, "less_than_eq_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) <= FLOAT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_i, "greater_than_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) > INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_f, "greater_than_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) > FLOAT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_eq_i, "greater_than_eq_i(int,int) -> bool")
    {
        set_bool(OUTPUT, INT_INPUT(0) >= INT_INPUT(1));
    }

    CA_DEFINE_FUNCTION(greater_than_eq_f, "greater_than_eq_f(number,number) -> bool")
    {
        set_bool(OUTPUT, FLOAT_INPUT(0) >= FLOAT_INPUT(1));
    }

    void setup(Branch* kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);

        TermList lt_overloads(kernel->get("less_than_i"), kernel->get("less_than_f"));
        create_overloaded_function(kernel, "less_than", &lt_overloads);

        TermList lte_overloads(kernel->get("less_than_eq_i"), kernel->get("less_than_eq_f"));
        create_overloaded_function(kernel, "less_than_eq", &lte_overloads);

        TermList gt_overloads(kernel->get("greater_than_i"), kernel->get("greater_than_f"));
        create_overloaded_function(kernel, "greater_than", &gt_overloads);

        TermList gte_overloads(kernel->get("greater_than_eq_i"), kernel->get("greater_than_eq_f"));
        create_overloaded_function(kernel, "greater_than_eq", &gte_overloads);
    }
}
}
