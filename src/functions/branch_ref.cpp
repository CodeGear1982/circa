// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_ref_function {

    CA_FUNCTION(branch_ref)
    {
        #if 0
        branch_ref_t::set_from_ref(OUTPUT, INPUT_TERM(0));
        #endif
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, branch_ref,
            "def branch_ref(void branch +ignore_error) -> BranchRef");
    }
}
}
