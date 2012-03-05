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

// Function object
typedef struct caFunction caFunction;

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

// Add a module search path, used when processing 'import' statements.
void circa_add_module_search_path(const char* path);

// Execute with the given command-line args.
int circa_run_command_line(int argc, const char* args[]);

// Load a module by opening the given filename as a source file.
caBranch* circa_load_module_from_file(caName module_name, const char* filename);

// Convert a string to a Circa name (an interned string that is referenced by integer).
// If the name doesn't already exist, it will be created.
caName circa_name(const char* str);

// Retrieve the string for a name.
const char* circa_name_string(caName name);

// Evaluation functions

// Retrieve the given input value. This value must not be modified.
caValue* circa_input(caStack* stack, int index);

// Retrieve the Term corresponding to the given input index, for doing meta-code operations.
caTerm* circa_input_term(caStack* stack, int index);

// Read the given input index as an integer
int circa_int_input(caStack* stack, int index);

// Read the given input index as a string
const char* circa_string_input(caStack* stack, int index);

// Fetch the caValue slot for the given output index.
caValue* circa_output(caStack* stack, int index);

// Initialize the given output value using the default create() function on the
// Term's declared type. Also returns the output slot.
caValue* circa_create_default_output(caStack* stack, int index);

// Fetch the Term that is currently being evaluated.
caTerm* circa_current_term(caStack* stack);

// Fetch the Branch for the function that is currently being evaluated
caBranch* circa_callee_branch(caStack* stack);

bool circa_is_int(caValue* container);
bool circa_is_float(caValue* container);
bool circa_is_string(caValue* container);

// Read an integer from a caValue
int circa_as_int(caValue* container);
float circa_as_float(caValue* container);

// Read a caValue as a string
const char* circa_as_string(caValue* container);

// Convert this caValue to a new string. If the value is already string, then this
// returns an exact copy (no quote marks are added). Otherwise it uses the type's
// to string handler. Either way, the result is a newly allocated string and the
// caller must free() it when finished.
char* circa_to_string(caValue* value);

void circa_get_point(caValue* point, float* xOut, float* yOut);
void circa_get_vec3(caValue* vec3, float* xOut, float* yOut, float* zOut);
void circa_get_vec4(caValue* vec4, float* xOut, float* yOut, float* zOut, float* wOut);
void circa_get_color(caValue* color, float* rOut, float* gOut, float* bOut, float* aOut);

// Read an opaque pointer from a caValue
void* circa_as_pointer(caValue* container);

// Initialize a newly allocated caValue container. This must be called before any set()
// functions.
void circa_init_value(caValue* container);

// Allocate a new caValue container on the heap. This will call circa_init_value for you.
caValue* circa_alloc_value();

// Assign an integer to a caValue
void circa_set_int(caValue* container, int value);

void circa_set_float(caValue* container, float value);

void circa_set_bool(caValue* container, bool value);

// Assign an opaque pointer to a caValue
void circa_set_pointer(caValue* container, void* ptr);

void circa_set_string(caValue* container, const char* str);
void circa_set_string_size(caValue* container, const char* str, int size);
void circa_string_append(caValue* container, const char* str);

void circa_set_list(caValue* list, int numElements);
caValue* circa_list_append(caValue* list);
void circa_set_point(caValue* point, float x, float y);

void circa_set_null(caValue* container);

caValue* circa_handle_get_value(caValue* handle);
void circa_handle_set(caValue* container, caValue* value, caReleaseFunc releaseFunc);

void circa_handle_set_object(caValue* handle, void* object, caReleaseFunc releaseFunc);

void* circa_handle_get_object(caValue* handle);

// Assign a Value using the Type's default create() handler.
void circa_create_value(caValue* value, caType* type);

// Signal that an error has occurred
void circa_raise_error(caStack* stack, const char* msg);

// Load a Circa value from a string representation. The result will be written to 'out'.
// 
// If there is a parsing error, an error value will be saved to 'out'. (the caller should
// check for this).
void circa_parse_string(const char* str, caValue* out);

// Write a string representation of 'value' to 'out'.
void circa_to_string_repr(caValue* value, caValue* out);

// Install an evaluation function to the given named term. Returns the affected Term.
caTerm* circa_install_function(caBranch* branch, const char* name, caEvaluateFunc evaluate);

// Fetch the number of inputs for the given term.
int circa_term_num_inputs(caTerm* term);

// Fetch the nth input for the given term. May return NULL.
caTerm* circa_term_get_input(caTerm* term, int index);

// Fetch the Term's declared type.
caType* circa_term_declared_type(caTerm* term);

// Create a new Stack object
caStack* circa_new_stack();

void circa_run_module(caStack* stack, caName moduleName);

// Return whether a runtime error occurred.
bool circa_error_occurred(caStack* stack);

// Clear a runtime error from the stack.
void circa_clear_error(caStack* stack);

// Accessing code data

// Get a Term from a Branch by index.
caTerm* circa_get_term(caBranch* branch, int index);

caBranch* circa_nested_branch(caTerm* term);
caBranch* circa_get_nested_branch(caBranch* branch, const char* name);

caBranch* circa_function_contents(caFunction* func);

// Access the fixed value of the given Term.
caValue* circa_term_value(caTerm* term);
int circa_term_get_index(caTerm* term);

// Building code
caFunction* circa_declare_function(caBranch* branch, const char* name);
caValue* circa_declare_value(caBranch* branch, const char* name);

void circa_func_set_evaluate(caFunction* func, caEvaluateFunc evaluate);

// Debugging helpers

// 'dump' commands will print a representation to stdout
void circa_dump_s(caStack* stack);
void circa_dump_b(caBranch* branch);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
