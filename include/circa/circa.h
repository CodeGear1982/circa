// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_H_INCLUDED
#define CIRCA_H_INCLUDED

// Public API

#ifdef __cplusplus
extern "C" {
#endif

typedef int caName;

// a Stack holds the current state of execution, including the list of activation records,
// and some other data.
typedef struct caStack caStack;

// a Value is a variant value. It holds two pointers, one pointer to the Type object and one
// to the value's data. For some types (such as integers, floats, booleans), the data section
// holds the actual value and not a pointer.
typedef struct caValue caValue;

// a Branch is a section of compiled code, it contains a list of Terms and some other metadata.
// Each Term may contain a 'nested' Branch.
typedef struct caBranch caBranch;

// a Term is one unit of compiled code. It consists of a function Term and a list of input Terms.
typedef struct caTerm caTerm;

// a Type holds data for a single Circa type.
typedef struct caType caType;

// EvaluateFunc is the signature for a C evaluation function. The function will access the stack
// to read inputs (if any), perform some action (sometimes), and write output values back to the
// stack (if it has any outputs).
typedef void (*caEvaluateFunc)(caStack* stack);

// ReleaseFunc is the signature for a C function that runs when a user object is about to be
// destroyed.
typedef void (*caReleaseFunc)(caValue* value);

// Initialize Circa, should be called at startup before any other Circa functions.
void circa_initialize();

// Shutdown Circa, should be called when shutting down. This isn't required but calling it will
// make memory-leak checking tools happier.
void circa_shutdown();

// Tell runtime to use the standard filesystem interface when loading files.
// (without this call, the default behavior is to never touch the filesystem).
void circa_use_standard_filesystem();

// Add a module search path, used when processing 'import' statements.
void circa_add_module_search_path(const char* path);

// Execute with the given command-line args.
int circa_run_command_line(int argc, const char* args[]);

// Load a module by opening the given filename as a source file.
caTerm* circa_load_module_from_file(caName module_name, const char* filename);

// Convert a string to a symbol value, creating it if necessary. Symbols are used
// internally as names.
caName circa_string_to_symbol(const char* str);

// Evaluation functions

caValue* circa_input(caStack* stack, int index);

// Read the given input index as an integer
int circa_int_input(caStack* stack, int index);

// Read the given input index as a string
const char* circa_string_input(caStack* stack, int index);

// Fetch the caValue slot for the given output index.
caValue* circa_output(caStack* stack, int index);

// Initialize the given output value using the default create() function on the
// Term's declared type. Also returns the output slot.
caValue* circa_create_default_output(caStack* stack, int index);

// Fetch the caTerm that is currently being evaluated.
caTerm* circa_current_term(caStack* stack);

// Read an integer from a caValue
int circa_as_int(caValue* container);

const char* circa_as_string(caValue* container);

// Read an opaque pointer from a caValue
void* circa_as_pointer(caValue* container);

caValue* circa_handle_get_value(caValue* handle);

// Assign an integer to a caValue
void circa_set_int(caValue* container, int value);

// Assign an opaque pointer to a caValue
void circa_set_pointer(caValue* container, void* ptr);

void circa_set_string_size(caValue* container, const char* str, int size);

void circa_set_null(caValue* container);

// Assign a handle to a caValue
void circa_handle_set(caValue* container, caValue* value, caReleaseFunc releaseFunc);

void circa_handle_set_object(caValue* handle, void* object, caReleaseFunc releaseFunc);

// Assign a Value using the Type's default create() handler.
void circa_create_value(caValue* value, caType* type);

// Signal that an error has occurred
void circa_raise_error(caStack* stack, const char* msg);

// Install an evaluation function to the given named term. Returns the affected Term.
caTerm* circa_install_function(caBranch* branch, const char* name, caEvaluateFunc evaluate);

// Fetch the Term's declared type.
caType* circa_term_declared_type(caTerm* term);

#ifdef __cplusplus
} // extern "C"
#endif

#endif