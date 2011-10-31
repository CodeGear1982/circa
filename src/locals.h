// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
    
int get_output_count(Term* term);
int get_locals_count(Branch* branch);
void update_output_count(Term* term);

} // namespace circa
