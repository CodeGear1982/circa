// Copyright 2008 Andrew Fischer

#include "circa.h"

#include "math.h"

namespace circa {
namespace cos_function {

    void evaluate(Term* caller)
    {
        float input = as_float(caller->inputs[0]);
        as_float(caller) = cos(input);
    }

    void setup(Branch& kernel)
    {
        Term* main_func = import_c_function(kernel, evaluate,
                "function cos(float) -> float");
        as_function(main_func).pureFunction = true;
    }
}
}
