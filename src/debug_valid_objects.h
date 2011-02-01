// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "build_options.h"

namespace circa {

const int TERM_OBJECT = 1;
const int LIST_OBJECT = 2;
const int BRANCH_OBJECT = 3;
const int FUNCTION_ATTRS_OBJECT = 4;
const int TYPE_OBJECT = 5;

#if CIRCA_VALID_OBJECT_CHECKING

void debug_register_valid_object(void* obj, int type);
void debug_register_valid_object_ignore_dupe(void* obj, int type);
void debug_unregister_valid_object(void* obj);
void debug_assert_valid_object(void* obj, int type);

#else

// No-op these calls
#define debug_register_valid_object(...)
#define debug_register_valid_object_ignore_dupe(...)
#define debug_unregister_valid_object(...)
#define debug_assert_valid_object(...)

#endif

// Returns whether this object is valid and is registered with the given
// type. If possible, you are encouraged to use debug_assert_valid_object
// instead of this call.
bool debug_is_object_valid(void* obj, int type);

// Type-specific calls below:


// Checks if term is a valid pointer according to our map, and triggers
// an ca_assert if not.
void assert_valid_term(Term* term);

// Returns whether this pointer is valid according to our map. 
bool debug_is_term_pointer_valid(Term* term);

}
