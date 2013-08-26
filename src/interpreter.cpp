// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "blob.h"
#include "block.h"
#include "bytecode.h"
#include "closures.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "dict.h"
#include "function.h"
#include "generic.h"
#include "hashtable.h"
#include "if_block.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "parser.h"
#include "reflection.h"
#include "stack.h"
#include "stateful_code.h"
#include "string_type.h"
#include "symbols.h"
#include "names.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

static Frame* stack_push_blank(Stack* stack);
static void stack_resize_frame_list(Stack* stack, int newCapacity);
static Term* frame_current_term(Frame* frame);
static Frame* frame_by_index(Stack* stack, int id);
static Frame* expand_frame(Frame* parent, Frame* top);
static Frame* expand_frame_indexed(Frame* parent, Frame* top, int index);
static void retain_stack_top(Stack* stack);
static void update_stack_for_possibly_changed_blocks(Stack* stack);
static void start_interpreter_session(Stack* stack);
static void push_inputs_dynamic(Stack* stack);
void run(Stack* stack);
bool run_memoization_lookahead_check(Frame* frame, Frame* top, const char* bc, int* pos);
static int for_loop_find_index_value(Frame* frame);

Stack::Stack()
 : errorOccurred(false),
   world(NULL)
{
    id = global_world()->nextStackID++;

    step = sym_StackReady;
    framesCapacity = 0;
    framesCount = 0;
    frames = NULL;
    nextRootStack = NULL;
    prevRootStack = NULL;
}

Stack::~Stack()
{
    // Clear error, so that stack_pop doesn't complain about losing an errored frame.
    stack_ignore_error(this);

    stack_reset(this);

    free(frames);

    if (world != NULL) {
        if (world->firstRootStack == this)
            world->firstRootStack = world->firstRootStack->nextRootStack;
        if (world->lastRootStack == this)
            world->lastRootStack = world->lastRootStack->prevRootStack;
        if (nextRootStack != NULL)
            nextRootStack->prevRootStack = prevRootStack;
        if (prevRootStack != NULL)
            prevRootStack->nextRootStack = nextRootStack;
    }
}

void
Stack::dump()
{
    circa::dump(this);
}

Stack* create_stack(World* world)
{
    Stack* stack = new Stack();
    stack->world = world;
    
    // Add this Stack as a root stack. TODO for garbage collection, is the ability to
    // create non-root stacks.
    if (world != NULL) {
        if (world->firstRootStack == NULL)
            world->firstRootStack = stack;
        if (world->lastRootStack != NULL)
            world->lastRootStack->nextRootStack = stack;
        stack->prevRootStack = world->lastRootStack;
        stack->nextRootStack = NULL;
        world->lastRootStack = stack;
    }

    return stack;
}

void free_stack(Stack* stack)
{
    delete stack;
}

Frame* stack_top(Stack* stack)
{
    if (stack->framesCount == 0)
        return NULL;
    return &stack->frames[stack->framesCount - 1];
}

Frame* stack_top_parent(Stack* stack)
{
    if (stack->framesCount <= 1)
        return NULL;
    return &stack->frames[stack->framesCount - 2];
}

Block* stack_top_block(Stack* stack)
{
    Frame* frame = stack_top(stack);
    if (frame == NULL)
        return NULL;
    return frame->block;
}

void stack_init(Stack* stack, Block* block)
{
    // Pop existing frames.
    while (stack_top(stack) != NULL)
        stack_pop(stack);

    stack_push(stack, block);
}

Frame* stack_push(Stack* stack, Block* block, int parentPc)
{
    INCREMENT_STAT(PushFrame);

    refresh_bytecode(block);

    Frame* frame = stack_push_blank(stack);

    frame->parentPc = parentPc;
    frame->block = block;
    frame->blockVersion = block->version;
    set_list(&frame->registers, block_locals_count(block));

    return frame;
}

Frame* stack_push(Stack* stack, Block* block)
{
    INCREMENT_STAT(PushFrame);

    int parentPc = 0;
    if (stack_top(stack) != NULL)
        parentPc = stack_top(stack)->pc;

    return stack_push(stack, block, parentPc);
}

static Frame* stack_push3(Stack* stack, Frame* parent, int parentPc, Block* block)
{
    Frame* top = stack_push_blank(stack);
    top->parentPc = parentPc;
    top->block = block;
    top->blockVersion = block->version;
    set_list(&top->registers, block_locals_count(block));
    refresh_bytecode(block);
    return top;
}

static Frame* expand_frame(Frame* parent, Frame* top)
{
    // Look for frame state to carry over from parent.
    if (is_list(&parent->state) && !is_null(list_get(&parent->state, top->parentPc))) {

        // Found non-null state for this parentPc.
        caValue* state = list_get(&parent->state, top->parentPc);

        if (is_frame(state)
                // Don't expand state on calls from within declared_state.
                && (parent->block->owningTerm != FUNCS.declared_state)) {

            Frame* savedFrame = as_frame(state);
            if (savedFrame->block == top->block)
                copy(&savedFrame->state, &top->state);
        }
    }

    return top;
}

static Frame* expand_frame_indexed(Frame* parent, Frame* top, int index)
{
    // Look for frame state to carry over from parent.
    if (is_list(&parent->state) && !is_null(list_get(&parent->state, top->parentPc))) {

        // Found non-null state for this parentPc.
        caValue* state = list_get(&parent->state, top->parentPc);

        if (is_list(state) && index < list_length(state)) {
            caValue* element = list_get(state, index);

            if (is_frame(element)
                    // Don't expand state on calls from within declared_state.
                    && (parent->block->owningTerm != FUNCS.declared_state)) {

                Frame* savedFrame = as_frame(element);
                if (savedFrame->block == top->block)
                    copy(&savedFrame->state, &top->state);
            }
        }
    }

    return top;
}

void stack_pop_no_retain(Stack* stack)
{
    Frame* frame = stack_top(stack);

    set_null(&frame->registers);
    set_null(&frame->customBytecode);
    set_null(&frame->dynamicScope);
    set_null(&frame->state);

    stack->framesCount--;
}

void stack_pop(Stack* stack)
{
    Frame* frame = stack_top(stack);

    if (frame->retain)
        retain_stack_top(stack);

    stack_pop_no_retain(stack);
}

static void retain_stack_top(Stack* stack)
{
    Frame* top = stack_top(stack);
    Frame* parent = frame_parent(top);
    if (parent == NULL)
        return;

    // Expand parent->state if needed.
    if (is_null(&parent->state))
        set_list(&parent->state, parent->block->length());

    touch(&parent->state);

    caValue* slot = list_get(&parent->state, top->parentPc);

    if (top->block->owningTerm->function == FUNCS.case_func) {

        // If-block special case: Store a list where each element corresponds with a
        // condition block.

        int caseIndex = case_block_get_index(top->block);

        if (!is_list(slot))
            set_list(slot);
        else {
            list_touch(slot);

            // Erase pre-existing state in each unused condition block.
            for (int i=0; i < list_length(slot); i++)
                if (caseIndex != i)
                    set_null(list_get(slot, i));
        }

        if (list_length(slot) <= caseIndex)
            list_resize(slot, caseIndex + 1);

        slot = list_get(slot, caseIndex);

    } else if (top->block->owningTerm->function == FUNCS.for_func) {

        // For-loop special case: Store a list where each element corresponds with a
        // loop iteration.
        //
        // Note that when a loop iteration is being saved, retain_stack_top is called by LoopDone
        // instead of by stack_pop.
        
        if (!is_list(slot))
            set_list(slot);

        int loopIndex = for_loop_find_index_value(top);

        if (list_length(slot) <= loopIndex)
            list_resize(slot, loopIndex + 1);

        slot = list_get(slot, loopIndex);
    }

    copy_stack_frame_to_boxed(top, slot);

    parent->retain = true;
}

static Frame* stack_push_blank(Stack* stack)
{
    // Check capacity.
    if ((stack->framesCount + 1) >= stack->framesCapacity)
        stack_resize_frame_list(stack, stack->framesCapacity == 0 ? 8 : stack->framesCapacity * 2);

    stack->framesCount++;

    Frame* frame = stack_top(stack);

    // Initialize frame
    frame->pc = 0;
    frame->pos = 0;
    frame->exitType = sym_None;
    frame->callType = sym_NormalCall;
    frame->retain = false;
    frame->block = NULL;
    frame->blockVersion = 0;

    return frame;
}

void stack_reset(Stack* stack)
{
    stack->errorOccurred = false;

    while (stack_top(stack) != NULL)
        stack_pop(stack);
}

void stack_restart(Stack* stack)
{
    if (stack->step == sym_StackReady)
        return;

    if (stack_top(stack) == NULL)
        return;

    bool errorOccurred = stack_errored(stack);

    while (stack_top_parent(stack) != NULL)
        stack_pop(stack);

    Frame* top = stack_top(stack);
    Block* block = top->block;
    top->pc = 0;
    top->pos = 0;

    // Clear registers
    for (int i=0; i < list_length(&top->registers); i++) {
        // Don't delete output values.
        Term* term = block->getSafe(i);
        if (term != NULL && is_output_placeholder(term))
            continue;

        set_null(list_get(&top->registers, i));
    }

    stack->step = sym_StackReady;
}

Stack* stack_duplicate(Stack* stack)
{
    Stack* dupe = create_stack(stack->world);
    stack_resize_frame_list(dupe, stack->framesCapacity);

    for (int i=0; i < stack->framesCapacity; i++) {
        Frame* sourceFrame = &stack->frames[i];
        Frame* dupeFrame = &dupe->frames[i];

        frame_copy(sourceFrame, dupeFrame);
    }

    dupe->framesCount = stack->framesCount;
    dupe->step = stack->step;
    dupe->errorOccurred = stack->errorOccurred;
    set_value(&dupe->context, &stack->context);
    return dupe;
}

caValue* stack_get_state(Stack* stack)
{
    Frame* top = stack_top(stack);
    Term* stateSlot = NULL;
    
    if (stack->step == sym_StackReady)
        stateSlot = find_state_input(top->block);
    else
        stateSlot = find_state_output(top->block);
    
    if (stateSlot == NULL)
        return NULL;

    return frame_register(top, stateSlot);
}

caValue* stack_find_active_value(Frame* frame, Term* term)
{
    ca_assert(term != NULL);

    if (is_value(term))
        return term_value(term);

    caStack* stack = frame->stack;

    while (true) {
        if (frame->block == term->owningBlock)
            return frame_register(frame, term);

        frame = frame_parent(frame);

        if (frame == NULL)
            break;
    }

    // Special case for function values that aren't on the stack: allow these
    // to be accessed as a term value.
    if (term->function == FUNCS.function_decl) {
        if (is_null(term_value(term)))
            set_closure(term_value(term), term->nestedContents, NULL);
        return term_value(term);
    }

    return NULL;
}

void stack_ignore_error(Stack* cxt)
{
    cxt->errorOccurred = false;
}

void stack_clear_error(Stack* stack)
{
    stack_ignore_error(stack);
    while (stack_top(stack) != NULL)
        stack_pop(stack);

    Frame* top = stack_top(stack);
    top->pc = top->block->length();
}

void stack_to_string(Stack* stack, caValue* out)
{
    std::stringstream strm;

    strm << "[Stack #" << stack->id
        << ", frames = " << stack->framesCount
        << "]" << std::endl;

    for (int frameIndex = 0; frameIndex < stack->framesCount; frameIndex++) {

        Frame* frame = frame_by_index(stack, frameIndex);

        bool lastFrame = frameIndex == stack->framesCount - 1;

        Frame* childFrame = NULL;
        if (!lastFrame)
            childFrame = frame_by_index(stack, frameIndex + 1);

        int activeTermIndex = frame->pc;
        if (childFrame != NULL)
            activeTermIndex = childFrame->parentPc;

        int depth = stack->framesCount - 1 - frameIndex;
        Block* block = frame->block;
        strm << " [Frame index " << frameIndex
             << ", depth = " << depth
             << ", block = #" << block->id
             << ", pc = " << frame->pc
             << ", pos = " << frame->pos
             << ", retain = " << (frame->retain ? "true" : "false")
             << "]" << std::endl;

        if (block == NULL)
            continue;

        // indent
        for (int x = 0; x < frameIndex+2; x++) strm << " ";
        strm << "context: " << to_string(&frame->dynamicScope) << std::endl;
        for (int x = 0; x < frameIndex+2; x++) strm << " ";
        strm << "state: " << to_string(&frame->state) << std::endl;

        for (int i=0; i < frame->block->length(); i++) {
            Term* term = block->get(i);

            // indent
            for (int x = 0; x < frameIndex+1; x++)
                strm << " ";

            if (i == activeTermIndex)
                strm << ">";
            else
                strm << " ";

            print_term(term, strm);

            // current value
            if (term != NULL && !is_value(term)) {
                caValue* value = NULL;

                if (term->index < frame_register_count(frame))
                    value = frame_register(frame, term->index);

                if (value == NULL)
                    strm << " <register OOB>";
                else
                    strm << " = " << to_string(value);
            }
            strm << std::endl;
        }
    }

    set_string(out, strm.str().c_str());
}

void stack_trace_to_string(Stack* stack, caValue* out)
{
    std::stringstream strm;

    for (int frameIndex = 0; frameIndex < stack->framesCount; frameIndex++) {

        Frame* frame = frame_by_index(stack, frameIndex);

        bool lastFrame = frameIndex == stack->framesCount - 1;

        Frame* childFrame = NULL;
        if (!lastFrame)
            childFrame = frame_by_index(stack, frameIndex + 1);

        int activeTermIndex = frame->pc;
        if (childFrame != NULL)
            activeTermIndex = childFrame->parentPc;


        Term* term = frame->block->get(activeTermIndex);

        // Print a short location label
        if (term->function == FUNCS.input) {
            strm << "(input " << term->index << ")";
        } else {
            strm << get_short_location(term) << " ";
            if (term->name != "")
                strm << term->name << " = ";
            strm << term->function->name;
            strm << "()";
        }

        // Print the error value
        caValue* reg = frame_register(frame, activeTermIndex);
        if (lastFrame || is_error(reg)) {
            strm << " | ";
            if (is_string(reg))
                strm << as_cstring(reg);
            else
                strm << to_string(reg);
        }
        strm << std::endl;
    }

    set_string(out, strm.str().c_str());
}

void stack_extract_state(Stack* stack, caValue* output)
{
    frame_extract_state(frame_by_index(stack, 0), output);
}

Frame* frame_parent(Frame* frame)
{
    Stack* stack = frame->stack;
    int index = frame - stack->frames - 1;
    if (index < 0)
        return NULL;
    return &stack->frames[index];
}

Term* frame_caller(Frame* frame)
{
    return frame_term(frame_parent(frame), frame->parentPc);
}

Term* frame_current_term(Frame* frame)
{
    return frame->block->get(frame->pc);
}

Term* frame_term(Frame* frame, int index)
{
    return frame->block->get(index);
}

static Frame* frame_by_index(Stack* stack, int index)
{
    ca_assert(index >= 0);
    ca_assert(index < stack->framesCount);
    return &stack->frames[index];
}

Frame* frame_by_depth(Stack* stack, int depth)
{
    int index = stack->framesCount - 1 - depth;
    return frame_by_index(stack, index);
}

void frame_retain(Frame* frame)
{
    frame->retain = true;
}

static void stack_resize_frame_list(Stack* stack, int newCapacity)
{
    // Currently, the frame list can only be grown.
    ca_assert(newCapacity >= stack->framesCapacity);

    int oldCapacity = stack->framesCapacity;
    stack->framesCapacity = newCapacity;
    stack->frames = (Frame*) realloc(stack->frames, sizeof(Frame) * stack->framesCapacity);

    for (int i = oldCapacity; i < newCapacity; i++) {

        // Initialize new frame
        Frame* frame = &stack->frames[i];
        frame->stack = stack;
        initialize_null(&frame->registers);
        initialize_null(&frame->customBytecode);
        initialize_null(&frame->dynamicScope);
        initialize_null(&frame->state);
        frame->block = 0;
        frame->blockVersion = 0;
    }
}

void fetch_stack_outputs(Stack* stack, caValue* outputs)
{
    Frame* top = stack_top(stack);

    set_list(outputs, 0);

    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(top->block, i);
        if (placeholder == NULL)
            break;

        copy(get_top_register(stack, placeholder), circa_append(outputs));
    }
}

int num_inputs(Stack* stack)
{
    return count_input_placeholders(stack_top(stack)->block);
}

void consume_inputs_to_list(Stack* stack, List* list)
{
    int count = num_inputs(stack);
    list->resize(count);
    for (int i=0; i < count; i++) {
        consume_input(stack, i, list->get(i));
    }
}

caValue* get_input(Stack* stack, int index)
{
    return frame_register(stack_top(stack), index);
}

void consume_input(Stack* stack, int index, caValue* dest)
{
    // Disable input consuming
    copy(get_input(stack, index), dest);
}

caValue* get_output(Stack* stack, int index)
{
    Frame* frame = stack_top(stack);
    Term* placeholder = get_output_placeholder(frame->block, index);
    if (placeholder == NULL)
        return NULL;
    return frame_register(frame, placeholder);
}

caValue* get_caller_output(Stack* stack, int index)
{
    Frame* frame = stack_top_parent(stack);
    Term* currentTerm = frame->block->get(frame->pc);
    return frame_register(frame, get_output_term(currentTerm, index));
}

Term* current_term(Stack* stack)
{
    Frame* top = stack_top(stack);
    return top->block->get(top->pc);
}

Block* current_block(Stack* stack)
{
    Frame* top = stack_top(stack);
    return top->block;
}

caValue* frame_register(Frame* frame, int index)
{
    return list_get(&frame->registers, index);
}

caValue* frame_register(Frame* frame, Term* term)
{
    return frame_register(frame, term->index);
}

int frame_register_count(Frame* frame)
{
    return list_length(&frame->registers);
}

caValue* frame_registers(Frame* frame)
{
    return &frame->registers;
}

caValue* frame_bytecode(Frame* frame)
{
    if (!is_null(&frame->customBytecode))
        return &frame->customBytecode;
    return block_bytecode(frame->block);
}

Block* frame_block(Frame* frame)
{
    return frame->block;
}

bool state_inject(Stack* stack, caValue* name, caValue* value)
{
    caValue* state = stack_get_state(stack);
    Block* block = stack_top(stack)->block;

    // Initialize stateValue if it's currently null.
    if (is_null(state))
        make(block->stateType, state);

    caValue* slot = get_field(state, name, NULL);
    if (slot == NULL)
        return false;

    touch(state);
    copy(value, get_field(state, name, NULL));
    return true;
}

caValue* context_inject(Stack* stack, caValue* name)
{
    Frame* frame = stack_top(stack);

    if (is_null(&frame->dynamicScope))
        set_hashtable(&frame->dynamicScope);

    return hashtable_insert(&frame->dynamicScope, name);
}

caValue* frame_register_from_end(Frame* frame, int index)
{
    return list_get(&frame->registers, frame_register_count(frame) - 1 - index);
}

caValue* get_top_register(Stack* stack, Term* term)
{
    Frame* frame = stack_top(stack);
    ca_assert(term->owningBlock == frame->block);
    return frame_register(frame, term);
}

void create_output(Stack* stack)
{
    Term* caller = current_term(stack);
    caValue* output = get_output(stack, 0);
    make(caller->type, output);
}

void raise_error(Stack* stack)
{
    stack->step = sym_StackFinished;
    stack->errorOccurred = true;
}
void raise_error_msg(Stack* stack, const char* msg)
{
    caValue* slot = get_top_register(stack, current_term(stack));
    set_error_string(slot, msg);
    raise_error(stack);
}

bool stack_errored(Stack* stack)
{
    return stack->errorOccurred;
}

static void update_stack_for_possibly_changed_blocks(Stack* stack)
{
    for (int frameIndex=0; frameIndex < stack->framesCount; frameIndex++) {
        
        Frame* frame = frame_by_index(stack, frameIndex);

        if (frame->block == NULL)
            continue;

        if (frame->blockVersion == frame->block->version) {
            // Same version.

            if (frame_register_count(frame) != block_locals_count(frame->block))
                internal_error("locals count has changed, but version didn't change");

        } else {

            // Resize frame->registers if needed.
            list_resize(&frame->registers, block_locals_count(frame->block));
        }
    }
}

static Block* case_block_choose_block(Stack* stack, Term* term)
{
    // Find the accepted case
    Frame* frame = stack_top(stack);
    Block* contents = nested_contents(term);

    int termIndex = 0;
    while (contents->get(termIndex)->function == FUNCS.input)
        termIndex++;

    for (; termIndex < contents->length(); termIndex++) {
        Term* caseTerm = contents->get(termIndex);

        // Fallback block has NULL input
        if (caseTerm->input(0) == NULL)
            return nested_contents(caseTerm);

        caValue* caseInput = stack_find_active_value(frame, caseTerm->input(0));

        // Check type on caseInput
        if (!is_bool(caseInput)) {
            raise_error_msg(stack, "Expected bool input");
            return NULL;
        }

        if (as_bool(caseInput))
            return nested_contents(caseTerm);
    }
    return NULL;
}

static int for_loop_find_index_value(Frame* frame)
{
    Term* term = for_loop_find_index(frame->block);
    ca_assert(term != NULL);
    return as_int(frame_register(frame, term));
}

static void start_interpreter_session(Stack* stack)
{
    Block* topBlock = stack_top(stack)->block;

    // Refresh bytecode for every block in this stack.
    for (Frame* frame = stack_top(stack); frame != NULL; frame = frame_parent(frame)) {
        refresh_bytecode(frame->block);
    }

    // Make sure there are no pending code updates.
    block_finish_changes(topBlock);

    // Check if our stack needs to be updated following block modification
    update_stack_for_possibly_changed_blocks(stack);

    // Cast all inputs, in case they were passed in uncast.
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(topBlock, i);
        if (placeholder == NULL)
            break;
        caValue* slot = get_top_register(stack, placeholder);
        cast(slot, placeholder->type);
    }
}

void evaluate_block(Stack* stack, Block* block)
{
    block_finish_changes(block);

    // Top-level call
    stack_push(stack, block);

    run_interpreter(stack);

    if (!stack_errored(stack))
        stack_pop(stack);
}

void run_interpreter(Stack* stack)
{
    if (stack->step == sym_StackFinished)
        stack_restart(stack);

    start_interpreter_session(stack);

    stack->errorOccurred = false;
    stack->step = sym_StackRunning;

    run_bytecode(stack, frame_bytecode(stack_top(stack)));
}

void raise_error_input_type_mismatch(Stack* stack)
{
    Frame* frame = stack_top(stack);
    Term* term = frame->block->get(frame->pc);
    caValue* value = frame_register(frame, frame->pc);

    circa::Value msg;
    set_string(&msg, "Couldn't cast input value ");
    string_append_quoted(&msg, value);
    string_append(&msg, " to type ");
    string_append(&msg, &declared_type(term)->name);
    raise_error_msg(stack, as_cstring(&msg));
    return;
}

void raise_error_output_type_mismatch(Stack* stack)
{
    Frame* parent = stack_top_parent(stack);
    Term* outputTerm = frame_current_term(parent);
    caValue* value = frame_register(parent, outputTerm);

    circa::Value msg;
    set_string(&msg, "Couldn't cast output value ");
    string_append_quoted(&msg, value);
    string_append(&msg, " to type ");
    string_append(&msg, &declared_type(outputTerm)->name);
    raise_error_msg(stack, as_cstring(&msg));
    return;
}

int get_count_of_caller_inputs_for_error(Stack* stack)
{
    Frame* parentFrame = stack_top_parent(stack);
    Term* callerTerm = parentFrame->block->get(parentFrame->pc);
    int foundCount = callerTerm->numInputs();

    if (callerTerm->function == FUNCS.func_call)
        foundCount--;
    else if (callerTerm->function == FUNCS.func_apply) {
        caValue* inputs = stack_find_active_value(parentFrame, callerTerm->input(1));
        foundCount = list_length(inputs);
    }

    return foundCount;
}

void raise_error_not_enough_inputs(Stack* stack)
{
    Frame* frame = stack_top(stack);
    Frame* parent = stack_top_parent(stack);
    Term* caller = frame_caller(frame);

    int expectedCount = count_input_placeholders(frame->block);
    int foundCount = get_count_of_caller_inputs_for_error(stack);

    Value msg;
    set_string(&msg, "Too few inputs: expected ");
    string_append(&msg, expectedCount);
    if (has_variable_args(frame->block))
        string_append(&msg, " (or more)");
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    // Discard the top branch.
    stack_pop_no_retain(stack);

    set_error_string(frame_register(stack_top(stack), caller), as_cstring(&msg));
    raise_error(stack);
}

void raise_error_too_many_inputs(Stack* stack)
{
    Frame* frame = stack_top(stack);
    Frame* parent = stack_top_parent(stack);
    Term* caller = frame_caller(frame);

    int expectedCount = count_input_placeholders(frame->block);
    int foundCount = get_count_of_caller_inputs_for_error(stack);

    Value msg;
    set_string(&msg, "Too many inputs: expected ");
    string_append(&msg, expectedCount);
    string_append(&msg, ", received ");
    string_append(&msg, foundCount);

    // Discard the top branch.
    stack_pop_no_retain(stack);

    set_error_string(frame_register(stack_top(stack), caller), as_cstring(&msg));
    raise_error(stack);
}

void pop_outputs_dynamic(Stack* stack, Frame* frame, Frame* top)
{
    Term* caller = frame_caller(top);
    Block* finishedBlock = frame_block(top);

    // Walk through caller's output terms, and pull output values from the frame.
    int placeholderIndex = 0;

    for (int callerOutputIndex=0;; callerOutputIndex++) {
        Term* outputTerm = get_output_term(caller, callerOutputIndex);
        if (outputTerm == NULL)
            break;

        caValue* outputRegister = frame_register(frame, outputTerm);

        Term* placeholder = get_output_placeholder(finishedBlock, placeholderIndex);
        if (placeholder == NULL) {
            set_null(outputRegister);
        } else {
            caValue* placeholderRegister = frame_register(top, placeholder->index);
            copy(placeholderRegister, outputRegister);
        }

        placeholderIndex++;
    }
}


void run_bytecode(Stack* stack, caValue* bytecode)
{
    struct InterpreterTransientState {
        char* bc;
        Frame* frame;
        int pos;

        // Saved when jumping from DynamicMethod to PushApply/PushCall.
        int termIndex;

        // Saved when jumping from Break/Continue/Discard to a LoopDone.
        bool loopEnableOutput;
    };

    InterpreterTransientState s;

    s.bc = as_blob(bytecode);
    s.frame = stack_top(stack);
    s.pos = s.frame->pos;
    s.loopEnableOutput = false;

    while (true) {

        // TODO_WITH_TESTS: delete this check.
        if (stack->step != sym_StackRunning)
            return;

        INCREMENT_STAT(StepInterpreter);

        // bytecode_dump_next_op(frame_bytecode(s.frame), s.frame->block, s.pos);

        // Dispatch op
        char op = blob_read_char(s.bc, &s.pos);
        switch (op) {
        case bc_NoOp:
            continue;
        case bc_Pause:
            s.frame->pos = s.pos;
            return;
        case bc_DoneTransient:
            // Finish interpreter without saving pc.
            return;

        case bc_PushFunction: {
            ca_assert(s.frame == stack_top(stack));

            int termIndex = blob_read_int(s.bc, &s.pos);

            s.frame->pc = termIndex;
            s.frame->pos = s.pos;
            Term* caller = frame_term(s.frame, termIndex);
            Block* block = function_contents(caller->function);

            Frame* top = stack_push3(stack, s.frame, termIndex, block);
            s.frame = stack_top_parent(stack);
            expand_frame(s.frame, top);
            ca_assert(s.frame != NULL);
            continue;
        }
        case bc_PushNested: {
            ca_assert(s.frame == stack_top(stack));

            int index = blob_read_int(s.bc, &s.pos);

            s.frame->pc = index;
            s.frame->pos = s.pos;
            Term* caller = frame_term(s.frame, index);
            Block* block = caller->nestedContents;

            Frame* top = stack_push(stack, block);
            top->parentPc = index;
            s.frame = stack_top_parent(stack);
            continue;
        }
        case bc_PushInputFromStack: {
            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);
            Term* caller = frame_caller(top);

            int inputIndex = blob_read_int(s.bc, &s.pos);
            int destSlot = blob_read_int(s.bc, &s.pos);

            Term* placeholderTerm = top->block->get(inputIndex);
            caValue* placeholderRegister = frame_register(top, inputIndex);
            caValue* value = stack_find_active_value(parent, caller->input(inputIndex));
            ca_assert(value != NULL);
            caValue* slot = frame_register(top, destSlot);
            copy(value, slot);

            if (!cast(placeholderRegister, declared_type(placeholderTerm)))
                if (!placeholderTerm->boolProp("optional", false))
                    return raise_error_input_type_mismatch(stack);

            continue;
        }
        case bc_PushVarargList: {
            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);
            Term* caller = frame_caller(top);

            int startIndex = blob_read_int(s.bc, &s.pos);
            int destSlot = blob_read_int(s.bc, &s.pos);

            caValue* dest = frame_register(top, destSlot);
            int count = caller->numInputs() - startIndex;
            set_list(dest, count);
            for (int i=0; i < count; i++) {
                caValue* value = stack_find_active_value(parent, caller->input(startIndex+i));
                copy(value, list_get(dest, i));
            }

            continue;
        }
        case bc_PushInputNull: {
            int inputIndex = blob_read_int(s.bc, &s.pos);

            Frame* top = stack_top(stack);

            set_null(frame_register(top, inputIndex));
            continue;
        }
        case bc_PushInputsDynamic: {
            push_inputs_dynamic(stack);
            continue;
        }
        case bc_PushExplicitState: {
            int inputIndex = blob_read_int(s.bc, &s.pos);

            Frame* top = stack_top(stack);
            Term* caller = frame_caller(top);
            Frame* parent = stack_top_parent(stack);

            caValue* value = stack_find_active_value(parent, caller->input(inputIndex));

            if (is_frame(value)) {
                Frame* savedFrame = as_frame(value);
                copy(&savedFrame->state, &top->state);

            } else {
                // TODO: Raise error if the value is not null and not a Frame?
            }

            continue;
        }
        
        case bc_EnterFrame: {
            ca_assert(s.frame == stack_top_parent(stack));

            s.frame->pos = s.pos;
            s.frame = stack_top(stack);
            s.bc = as_blob(frame_bytecode(s.frame));
            s.frame->pos = 0;
            s.pos = 0;
            continue;
        }
do_done_insn:
        case bc_Done: {
            ca_assert(s.frame == stack_top(stack));

            // Exit if we have finished the topmost block
            if (frame_parent(s.frame) == NULL) {
                s.frame->pos = s.pos;
                stack->step = sym_StackFinished;
                return;
            }

            s.frame = stack_top_parent(stack);
            s.bc = as_blob(frame_bytecode(s.frame));
            s.pos = s.frame->pos;
            continue;
        }
        case bc_LoopDone: {
            ca_assert(s.frame == stack_top(stack));
            
            s.loopEnableOutput = blob_read_char(s.bc, &s.pos);

do_loop_done_insn:

            Block* contents = s.frame->block;

            // Possibly save state.
            if (s.frame->retain)
                retain_stack_top(stack);

            caValue* index = frame_register(s.frame, for_loop_find_index(contents));
            set_int(index, as_int(index) + 1);

            // Preserve list output.
            if (s.loopEnableOutput && s.frame->exitType != sym_Discard) {
                caValue* outputIndex = frame_register(s.frame, for_loop_find_output_index(contents));

                caValue* resultValue = frame_register_from_end(s.frame, 0);
                caValue* outputList = stack_find_active_value(s.frame, contents->owningTerm);

                copy(resultValue, list_append(outputList));

                INCREMENT_STAT(LoopWriteOutput);

                // Advance output index
                set_int(outputIndex, as_int(outputIndex) + 1);
            }

            // Check if we are finished
            caValue* listInput = frame_register(s.frame, 0);
            if (as_int(index) >= list_length(listInput)
                    || s.frame->exitType == sym_Break
                    || s.frame->exitType == sym_Return) {

                // Silly code- move the output list (in the parent frame) to our frame's output,
                // where it will get copied back to parent when the frame is finished.
                if (s.loopEnableOutput) {
                    caValue* outputList = stack_find_active_value(s.frame, contents->owningTerm);
                    move(outputList, frame_register_from_end(s.frame, 0));
                } else {
                    set_list(frame_register_from_end(s.frame, 0), 0);
                }

                // Erase the retain flag - we've already saved each iteration, and we don't want
                // stack_pop to save anything else.
                s.frame->retain = false;
                
                goto do_done_insn;
            }

            // If we're not finished yet, copy rebound outputs back to inputs.
            for (int i=1;; i++) {
                Term* input = get_input_placeholder(contents, i);
                if (input == NULL)
                    break;
                Term* output = get_output_placeholder(contents, i);
                copy(frame_register(s.frame, output),
                    frame_register(s.frame, input));

                INCREMENT_STAT(Copy_LoopCopyRebound);
            }

            // Return to start of the loop.
            s.frame->pc = 0;
            s.pos = 0;
            s.frame->exitType = sym_None;
            set_null(&s.frame->state);
            expand_frame_indexed(stack_top_parent(stack), s.frame, as_int(index));
            continue;
        }
        
        case bc_PopOutput: {
            ca_assert(s.frame == stack_top_parent(stack));

            int placeholderIndex = blob_read_int(s.bc, &s.pos);
            int outputIndex = blob_read_int(s.bc, &s.pos);

            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);
            Term* caller = frame_term(parent, top->parentPc);

            Term* placeholder = get_output_placeholder(top->block, placeholderIndex);
            caValue* value = frame_register(top, placeholder);

            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);
            copy(value, receiverSlot);

            // Type check
            // Future: should this use receiver's type instead of placeholder?
            bool castSuccess = cast(receiverSlot, declared_type(placeholder));
                
            // For now, allow any output value to be null. Will revisit.
            castSuccess = castSuccess || is_null(receiverSlot);

            if (!castSuccess) {
                return raise_error_output_type_mismatch(stack);
            }

            continue;
        }
        case bc_PopOutputNull: {
            ca_assert(s.frame == stack_top_parent(stack));

            int outputIndex = blob_read_int(s.bc, &s.pos);
            Frame* parent = stack_top_parent(stack);
            Term* caller = frame_current_term(parent);

            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);
            set_null(receiverSlot);
            continue;
        }
        case bc_PopOutputsDynamic: {
            ca_assert(s.frame == stack_top_parent(stack));
            pop_outputs_dynamic(stack, s.frame, stack_top(stack));
            continue;
        }
        case bc_PopExplicitState: {
            ca_assert(s.frame == stack_top_parent(stack));
            int outputIndex = blob_read_int(s.bc, &s.pos);
            Frame* top = stack_top(stack);
            Term* caller = frame_caller(top);
            Frame* parent = stack_top_parent(stack);
            Term* receiver = get_output_term(caller, outputIndex);
            caValue* receiverSlot = frame_register(parent, receiver);

            copy_stack_frame_to_boxed(top, receiverSlot);

            continue;
        }
        case bc_PopFrame: {
            ca_assert(s.frame == stack_top_parent(stack));

            Frame* top = stack_top(stack);

            if (top->retain) {
                // TODO: Capture this frame
            }

            stack_pop(stack);
            s.frame = stack_top(stack);
            s.bc = as_blob(frame_bytecode(s.frame));
            continue;
        }
        case bc_SetNull: {
            ca_assert(s.frame == stack_top(stack));

            int index = blob_read_int(s.bc, &s.pos);
            set_null(frame_register(s.frame, index));
            continue;
        }
        
        case bc_PushDynamicMethod: {
            ca_assert(s.frame == stack_top(stack));

            s.termIndex = blob_read_int(s.bc, &s.pos);
            Term* caller = frame_term(s.frame, s.termIndex);

            s.frame->pc = s.termIndex;

            INCREMENT_STAT(DynamicMethodCall);

            // Lookup method
            caValue* object = stack_find_active_value(s.frame, caller->input(0));
            if (object == NULL) {
                Value msg;
                set_string(&msg, "Input 0 is null");
                set_error_string(frame_register(s.frame, caller), as_cstring(&msg));
                raise_error(stack);
            }

            std::string functionName = caller->stringProp("syntax:functionName", "");

            // Find and dispatch method
            Term* method = find_method(s.frame->block,
                (Type*) circa_type_of(object), functionName.c_str());

            // Method not found. Raise error.
            if (method == NULL) {
                Value msg;
                set_string(&msg, "Method ");
                string_append(&msg, functionName.c_str());
                string_append(&msg, " not found on type ");
                string_append(&msg, &circa_type_of(object)->name);
                set_error_string(frame_register(s.frame, caller), as_cstring(&msg));
                raise_error(stack);
                return;
            }
            
            // Check for methods that are normally handled with different bytecode.

            if (method == FUNCS.func_call)
                goto do_func_call;
            else if (method == FUNCS.func_apply)
                goto do_func_apply;

            Block* block = nested_contents(method);

            Frame* top = stack_push3(stack, s.frame, s.termIndex, block);
            s.frame = stack_top_parent(stack);
            expand_frame(s.frame, top);
            continue;
        }

        case bc_PushFuncCall: {
            ca_assert(s.frame == stack_top(stack));

            s.termIndex = blob_read_int(s.bc, &s.pos);

do_func_call:
            s.frame->pc = s.termIndex;
            s.frame->pos = s.pos;
            Term* caller = frame_term(s.frame, s.termIndex);

            caValue* closure = stack_find_active_value(s.frame, caller->input(0));
            Block* block = as_block(list_get(closure, 0));

            if (block == NULL) {
                Value msg;
                set_string(&msg, "Block is null");
                circa_output_error(stack, as_cstring(&msg));
                return;
            }

            Frame* top = stack_push3(stack, s.frame, s.termIndex, block);
            s.frame = stack_top_parent(stack);
            expand_frame(s.frame, top);
            top->callType = sym_FuncCall;
            continue;
        }

        case bc_PushFuncApply: {
            ca_assert(s.frame == stack_top(stack));

            s.termIndex = blob_read_int(s.bc, &s.pos);

do_func_apply:
            Term* caller = frame_term(s.frame, s.termIndex);
            caValue* closure = stack_find_active_value(s.frame, caller->input(0));
            Block* block = as_block(list_get(closure, 0));

            if (block == NULL) {
                Value msg;
                set_string(&msg, "Block is null");
                circa_output_error(stack, as_cstring(&msg));
                return;
            }

            Frame* top = stack_push3(stack, s.frame, s.termIndex, block);
            s.frame = stack_top_parent(stack);
            expand_frame(s.frame, top);
            top->callType = sym_FuncApply;
            continue;
        }

        case bc_PushCase: {
            ca_assert(s.frame == stack_top(stack));

            int index = blob_read_int(s.bc, &s.pos);

            s.frame->pc = index;
            s.frame->pos = s.pos;
            Term* caller = frame_term(s.frame, index);

            Block* block = case_block_choose_block(stack, caller);
            if (block == NULL)
                // Error occurred inside case_block_choose_block
                return;

            Frame* top = stack_push(stack, block);
            s.frame = stack_top_parent(stack);
            // TODO Optimization: Add case index to bytecode instead of looking it up here.
            expand_frame_indexed(s.frame, top, case_block_get_index(block));
            top->parentPc = index;
            continue;
        }
        case bc_PushLoop: {
            ca_assert(s.frame == stack_top(stack));

            int index = blob_read_int(s.bc, &s.pos);
            bool loopEnableOutput = blob_read_char(s.bc, &s.pos) != 0;

            s.frame->pc = index;
            s.frame->pos = s.pos;
            Term* caller = frame_term(s.frame, index);
            
            caValue* input = stack_find_active_value(s.frame, caller->input(0));

            // If there are zero inputs, use the #zero block.
            Block* block = NULL;
            bool zeroBlock = false;
            if (is_list(input) && list_length(input) == 0) {
                block = for_loop_get_zero_block(caller->nestedContents);
                zeroBlock = true;
            } else {
                block = caller->nestedContents;
            }

            Frame* top = stack_push3(stack, s.frame, index, block);
            s.frame = stack_top_parent(stack);

            if (!zeroBlock) {

                // Initialize the loop index
                // TODO Optimization: Don't do O(n) search for index term.
                set_int(frame_register(top, for_loop_find_index(block)), 0);

                expand_frame_indexed(s.frame, top, 0);

                if (loopEnableOutput) {
                    // Initialize output index.
                    set_int(frame_register(top, for_loop_find_output_index(block)), 0);

                    // Initialize output value.
                    caValue* outputList = stack_find_active_value(top, block->owningTerm);
                    set_list(outputList, 0);
                }
            }

            continue;
        }
        case bc_InlineCopy: {
            ca_assert(s.frame == stack_top(stack));

            int index = blob_read_int(s.bc, &s.pos);
            Term* caller = frame_term(s.frame, index);

            caValue* source = stack_find_active_value(s.frame, caller->input(0));
            caValue* dest = frame_register(s.frame, caller);
            copy(source, dest);

            continue;
        }
        case bc_FireNative: {
            ca_assert(s.frame == stack_top(stack));

            EvaluateFunc override = get_override_for_block(s.frame->block);
            ca_assert(override != NULL);

            // Override functions may not push/pop frames or change PC.
            int snapshotPc = s.frame->pc;
            Frame* snapshotFrame = s.frame;

            // Call override
            override(stack);

            // Assert that top frame has not changed.
            ca_assert(snapshotPc == s.frame->pc);
            ca_assert(snapshotFrame == s.frame);

            if (stack_errored(stack)) {
                s.frame->pc = s.frame->block->length() - 1;
                return;
            }

            continue;
        }
        case bc_Return: {
            ca_assert(s.frame == stack_top(stack));

            int index = blob_read_int(s.bc, &s.pos);
            Term* caller = frame_term(s.frame, index);

            Frame* toFrame = s.frame;

            // Find destination frame, the first parent major block.
            while (!is_major_block(toFrame->block) && frame_parent(toFrame) != NULL)
                toFrame = frame_parent(toFrame);

            // Copy outputs to destination frame.
            for (int i=0; i < caller->numInputs(); i++) {
                caValue* dest = frame_register_from_end(toFrame, i);
                if (caller->input(i) == NULL)
                    set_null(dest);
                else
                    copy(stack_find_active_value(s.frame, caller->input(i)), dest);
            }

            // Throw away intermediate frames.
            while (stack_top(stack) != toFrame)
                stack_pop(stack);

            s.frame = stack_top(stack);
            s.bc = as_blob(frame_bytecode(s.frame));
            s.loopEnableOutput = false;
            goto do_done_insn;
        }
        case bc_Continue:
        case bc_Break:
        case bc_Discard: {
            ca_assert(s.frame == stack_top(stack));

            int index = blob_read_int(s.bc, &s.pos);
            Term* caller = frame_term(s.frame, index);

            Frame* toFrame = s.frame;

            // Find destination frame, the parent for-loop block.
            while (!is_for_loop(toFrame->block) && frame_parent(toFrame) != NULL)
                toFrame = frame_parent(toFrame);

            // Copy outputs to destination frame.
            for (int i=0; i < caller->numInputs(); i++) {
                caValue* dest = frame_register_from_end(toFrame, i);
                if (caller->input(i) == NULL)
                    set_null(dest);
                else
                    copy(stack_find_active_value(s.frame, caller->input(i)), dest);
            }

            // Throw away intermediate frames.
            while (stack_top(stack) != toFrame)
                stack_pop(stack);

            // Save exit type
            if (op == bc_Continue)
                toFrame->exitType = sym_Continue;
            else if (op == bc_Break)
                toFrame->exitType = sym_Break;
            else if (op == bc_Discard) {
                toFrame->exitType = sym_Discard;
                s.frame->retain = false;
            }

            s.frame = stack_top(stack);
            s.pos = s.frame->pos;
            s.bc = as_blob(frame_bytecode(s.frame));
            s.loopEnableOutput = false;
            goto do_loop_done_insn;
        }

        case bc_NotEnoughInputs:
            return raise_error_not_enough_inputs(stack);
        case bc_TooManyInputs:
            return raise_error_too_many_inputs(stack);
        case bc_ErrorNotEnoughInputs: {
            Term* currentTerm = frame_current_term(s.frame);
            circa::Value msg;
            Block* func = function_contents(currentTerm->function);
            int expectedCount = count_input_placeholders(func);
            if (has_variable_args(func))
                expectedCount--;
            int foundCount = currentTerm->numInputs();
            set_string(&msg, "Too few inputs, expected ");
            string_append(&msg, expectedCount);
            if (has_variable_args(func))
                string_append(&msg, " (or more)");
            string_append(&msg, ", received ");
            string_append(&msg, foundCount);
            raise_error_msg(stack, as_cstring(&msg));
            return;
        }
        case bc_ErrorTooManyInputs: {
            Term* currentTerm = frame_current_term(s.frame);
            circa::Value msg;
            Block* func = function_contents(currentTerm->function);
            int expectedCount = count_input_placeholders(func);
            int foundCount = currentTerm->numInputs();
            set_string(&msg, "Too many inputs, expected ");
            string_append(&msg, expectedCount);
            string_append(&msg, ", received ");
            string_append(&msg, foundCount);

            raise_error_msg(stack, as_cstring(&msg));
            return;
        }

        case bc_MemoizeFrame: {
            frame_retain(s.frame);
            continue;
        }

        case bc_UseMemoizedOnEqualInputs: {
            run_memoization_lookahead_check(s.frame, stack_top(stack), s.bc, &s.pos);
            continue;
        }

        case bc_PackState: {
            int declaredIndex = blob_read_int(s.bc, &s.pos);
            int resultIndex = blob_read_int(s.bc, &s.pos);

            caValue* result = stack_find_active_value(s.frame, frame_term(s.frame, resultIndex));
            caValue* frameState = &s.frame->state;

            if (is_null(frameState))
                set_list(frameState, s.frame->block->length());
            touch(frameState);

            copy(result, list_get(frameState, declaredIndex));
            frame_retain(s.frame);
            continue;
        }

        case bc_MaybeNullifyState: {
            Frame* top = stack_top(stack);
            Frame* parent = stack_top_parent(stack);
            if (!top->retain && !is_null(&parent->state)) {
                caValue* slot = list_get(&parent->state, top->parentPc);
                set_null(slot);
            }
            continue;
        }

        default:
            std::cout << "Op not recognized: " << int(s.bc[s.pos - 1]) << std::endl;
            ca_assert(false);
        }
    }
}

static void push_inputs_dynamic(Stack* stack)
{
    Frame* top = stack_top(stack);
    Frame* callerFrame = stack_top_parent(stack);
    Term* caller = frame_caller(top);

    // If it's a closure call, handle it here.
    if (top->callType == sym_FuncCall
            || top->callType == sym_FuncApply) {

        Value copiedInputs;
        caValue* inputs;

        caValue* func = stack_find_active_value(callerFrame, caller->input(0));
        caValue* bindings = list_get(func, 1);

        if (top->callType == sym_FuncApply) {
            inputs = stack_find_active_value(callerFrame, caller->input(1));
        } else {
            set_list(&copiedInputs, caller->numInputs() - 1);
            inputs = &copiedInputs;
            for (int i=0; i < list_length(&copiedInputs); i++) {
                caValue* active = stack_find_active_value(callerFrame, caller->input(i + 1));
                copy(active, list_get(&copiedInputs, i));
            }
        }

        int placeholderIndex = 0;

        // Handle input() placeholders.
        for (;; placeholderIndex++) {
            Term* placeholder = top->block->getSafe(placeholderIndex);

            if (placeholder == NULL || placeholder->function != FUNCS.input) {
                // Finished with placeholders. Check if there are too many inputs.
                if (list_length(inputs) > placeholderIndex)
                    return raise_error_too_many_inputs(stack);

                break;
            }

            caValue* placeholderSlot = frame_register(top, placeholderIndex);

            if (placeholder->boolProp("multiple", false)) {
                list_slice(inputs, placeholderIndex, -1, placeholderSlot);
                break;
            }

            // Copy normal input.
            if (placeholderIndex >= list_length(inputs))
                return raise_error_not_enough_inputs(stack);

            copy(list_get(inputs, placeholderIndex), placeholderSlot);
        }

        // Handle unbound_input() placeholders.
        int nextBindingIndex = 0;
        for (;; placeholderIndex++) {
            Term* placeholder = top->block->getSafe(placeholderIndex);

            if (placeholder == NULL || placeholder->function != FUNCS.unbound_input) {
                break;
            }

            caValue* placeholderSlot = frame_register(top, placeholderIndex);

            // Silently stop if there aren't enough bindings here.
            if (nextBindingIndex >= list_length(bindings))
                break;

            copy(list_get(bindings, nextBindingIndex++), placeholderSlot);
        }

        return;
    }

    Value bytecode;
    bytecode_write_input_instructions(&bytecode, caller, top->block);
    blob_append_char(&bytecode, bc_DoneTransient);

    run_bytecode(stack, &bytecode);
}

bool run_memoization_lookahead_check(Frame* frame, Frame* top, const char* bc, int* interpreterPos)
{
    int pos = *interpreterPos;

    // Lookahead at PushInput instructions, and check to see if the existing values strictly
    // match the incoming values.
    
    while (true) {
        switch (blob_read_char(bc, &pos)) {
        case bc_PushInputFromStack: {
            int inputIndex = blob_read_int(bc, &pos);
            int destSlot = blob_read_int(bc, &pos);

            Term* caller = frame_caller(top);
            caValue* value = stack_find_active_value(frame, caller->input(inputIndex));
            caValue* slot = frame_register(top, destSlot);

            if (!strict_equals(value, slot))
                return false;

            continue;
        }
        case bc_PushVarargList: {
            int startIndex = blob_read_int(bc, &pos);
            int destSlot = blob_read_int(bc, &pos);

            Term* caller = frame_caller(top);
            caValue* dest = frame_register(top, destSlot);
            int count = caller->numInputs() - startIndex;

            if (!is_list(dest) || list_length(dest) != count)
                return false;

            for (int i=0; i < count; i++) {
                caValue* value = stack_find_active_value(frame, caller->input(startIndex+i));
                caValue* slot = list_get(dest, i);
                if (!strict_equals(value, slot))
                    return false;
            }
            continue;
        }
        case bc_PushInputNull: {
            int inputIndex = blob_read_int(bc, &pos);

            if (!is_null(frame_register(top, inputIndex)))
                return false;

            continue;
        }
        case bc_PushInputsDynamic: {
            // TODO
            continue;
        }
        case bc_EnterFrame: {
            // Memoization check succeeded.

            // Reuse saved frame if possible.
            // TODO
            
            // Advance the interpreter past the EnterFrame insn.
            *interpreterPos = pos;
            return true;
        }

        default:
            std::cout << "Unexpected op in run_memoization_lookahead_check:";
            std::cout << int(bc[pos - 1]) << std::endl;
            ca_assert(false);
        }
    }

    return false;
}

void evaluate_range(Stack* stack, Block* block, int start, int end)
{
#if 0
    start_interpreter_session(stack);

    block_finish_changes(block);
    stack_push(stack, block);

    Value bytecode;
    set_list(&bytecode, block->length() + 1);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        caValue* op = list_get(&bytecode, i);
        if (i < start || i >= end)
            bytecode_write_noop(op);
        else
            write_term_bytecode(term, op);
    }

    move(&bytecode, &stack_top(stack)->customBytecode);

    run_interpreter(stack);

    if (stack_errored(stack))
        return;

    stack_pop(stack);
#endif
}

void evaluate_minimum(Stack* stack, Term* term, caValue* result)
{
#if 0
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current block.
    
    Block* block = term->owningBlock;
    block_finish_changes(block);

    bool *marked = new bool[block->length()];
    memset(marked, false, sizeof(bool)*block->length());

    marked[term->index] = true;

    // Walk up the block, marking terms.
    for (int i=term->index; i >= 0; i--) {
        Term* checkTerm = block->get(i);
        if (checkTerm == NULL)
            continue;

        if (marked[i]) {
            for (int inputIndex=0; inputIndex < checkTerm->numInputs(); inputIndex++) {
                Term* input = checkTerm->input(inputIndex);
                if (input == NULL)
                    continue;

                // don't compile stuff outside this block.
                if (input->owningBlock != block)
                    continue;

                // don't follow :meta inputs.
                if (is_input_meta(nested_contents(checkTerm->function), inputIndex))
                    continue;

                marked[input->index] = true;
            }
        }
    }

    // Construct a bytecode fragment that only includes marked terms.
    Value bytecode;
    set_list(&bytecode, block->length() + 1);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        caValue* op = list_get(&bytecode, i);
        if (!marked[i])
            bytecode_write_noop(op);
        else
            write_term_bytecode(term, op);
    }
    
    bytecode_write_finish_op(list_get(&bytecode, block->length()));
    delete[] marked;

    // Push frame, use our custom bytecode.
    stack_push(stack, block);
    move(&bytecode, &stack_top(stack)->customBytecode);

    run_interpreter(stack);

    // Possibly save output
    if (result != NULL)
        copy(get_top_register(stack, term), result);

    stack_pop(stack);
#endif
}

void evaluate_minimum2(Term* term, caValue* output)
{
    // Check if 'term' is just a value; don't need to create a Stack if so.
    if (is_value(term))
        copy(term_value(term), output);

    Stack stack;
    evaluate_minimum(&stack, term, output);
}

Frame* as_frame_ref(caValue* value)
{
    ca_assert(value != NULL);
    if (!is_list(value) || list_length(value) != 2)
        return NULL;
    Stack* stack = (Stack*) as_opaque_pointer(list_get(value, 0));
    int frameId = as_int(list_get(value, 1));
    return frame_by_index(stack, frameId);
}

void Frame__registers(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);

    caValue* out = circa_output(callerStack, 0);
    copy(&frame->registers, out);

    // Touch 'output', as the interpreter may violate immutability.
    touch(out);
}

void Frame__active_value(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    Term* term = as_term_ref(circa_input(callerStack, 1));
    caValue* value = stack_find_active_value(frame, term);
    if (value == NULL)
        set_null(circa_output(callerStack, 0));
    else
        set_value(circa_output(callerStack, 0), value);
}

void Frame__set_active_value(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    Term* term = as_term_ref(circa_input(callerStack, 1));
    caValue* value = stack_find_active_value(frame, term);
    if (value == NULL)
        return raise_error_msg(callerStack, "Value not found");

    set_value(value, circa_input(callerStack, 2));
}

void Frame__block(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    ca_assert(frame != NULL);
    set_block(circa_output(callerStack, 0), frame->block);
}

void Frame__parent(caStack* callerStack)
{
    Frame* frame = as_frame_ref(circa_input(callerStack, 0));
    Frame* parent = frame_parent(frame);
    if (parent == NULL)
        set_null(circa_output(callerStack, 0));
    else
        copy_stack_frame_to_boxed(parent, circa_output(callerStack, 0));
}

void Frame__has_parent(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    Frame* parent = frame_parent(frame);
    set_bool(circa_output(stack, 0), parent != NULL);
}

void Frame__register(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    int index = circa_int_input(stack, 1);
    copy(frame_register(frame, index), circa_output(stack, 0));
}

void Frame__pc(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(stack, 0), frame->pc);
}

void Frame__parentPc(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_int(circa_output(stack, 0), frame->parentPc);
}

void Frame__current_term(caStack* stack)
{
    Frame* frame = as_frame_ref(circa_input(stack, 0));
    ca_assert(frame != NULL);
    set_term_ref(circa_output(stack, 0), frame_current_term(frame));
}

void frame_extract_state(Frame* frame, caValue* output)
{
    Block* block = frame->block;

    set_hashtable(output);
    if (!is_list(&frame->state))
        return;

    for (int i=0; i < list_length(&frame->state); i++) {
        Term* term = block->get(i);
        caValue* element = list_get(&frame->state, i);
        caValue* name = unique_name(term);

        if (term->function == FUNCS.declared_state) {
            copy(element, hashtable_insert(output, name));
        } else if (is_frame(element)) {
            frame_extract_state(as_frame(element), hashtable_insert(output, name));
        } else if (is_list(element)) {
            caValue* listOutput = hashtable_insert(output, name);
            set_list(listOutput, list_length(element));
            for (int i=0; i < list_length(element); i++) {
                caValue* indexedState = list_get(element, i);
                // indexedState is either null or a Frame
                if (is_frame(indexedState)) {
                    Frame* indexedFrame = as_frame(indexedState);
                    frame_extract_state(indexedFrame, list_get(listOutput, i));
                }
            }
        }
    }
}

void Frame__extract_state(caStack* stack)
{
    Frame* frame = as_frame(circa_input(stack, 0));
    caValue* output = circa_output(stack, 0);
    frame_extract_state(frame, output);
}

void make_stack(caStack* callerStack)
{
    Stack* newStack = create_stack(callerStack->world);
    set_pointer(circa_create_default_output(callerStack, 0), newStack);
}

void capture_stack(caStack* callerStack)
{
    Stack* newStack = stack_duplicate(callerStack);
    stack_pop(newStack);
    stack_top(newStack)->pc++;
    // FIXME: This needs to also adjust top frame's 'pos', to be past the Output calls.
    set_pointer(circa_create_default_output(callerStack, 0), newStack);
}

void Stack__block(caStack* stack)
{
    Stack* actor = as_stack(circa_input(stack, 0));
    set_block(circa_output(stack, 0), stack_top(actor)->block);
}

void Stack__dump(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    dump(self);
}

void Stack__find_active_frame_for_term(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    Term* term = as_term_ref(circa_input(stack, 1));

    if (term == NULL)
        return raise_error_msg(stack, "Term is null");

    Frame* frame = stack_top(self);

    while (true) {
        if (frame->block == term->owningBlock) {
            copy_stack_frame_to_boxed(frame, circa_output(stack, 0));
            return;
        }

        frame = frame_parent(frame);

        if (frame == NULL)
            break;
    }

    set_null(circa_output(stack, 0));
}

void Stack__inject_state(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);

    if (stack_top(self) == NULL)
        return raise_error_msg(self, "Can't inject onto stack with no frames");

    bool success = state_inject(self, name, val);
    set_bool(circa_output(stack, 0), success);
}

void Stack__inject_context(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    caValue* name = circa_input(stack, 1);
    caValue* val = circa_input(stack, 2);

    if (stack_top(self) == NULL)
        return raise_error_msg(stack, "Can't inject onto stack with no frames");

    copy(val, context_inject(self, name));
}

void Stack__call(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));

    if (self == NULL)
        return raise_error_msg(self, "Stack is null");

    stack_restart(self);

    // Populate inputs.
    caValue* ins = circa_input(self, 1);

    for (int i=0; i < list_length(ins); i++)
        copy(list_get(ins, i), circa_input(self, i));

    run_interpreter(self);

    copy(circa_output(self, 0), circa_output(stack, 0));
}

void Stack__stack_push(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    Block* block = as_block(circa_input(stack, 1));

    if (block == NULL)
        return circa_output_error(stack, "Null block for input 1");

    ca_assert(block != NULL);

    Frame* top = stack_push(self, block);

    caValue* inputs = circa_input(stack, 2);

    for (int i=0; i < list_length(inputs); i++) {
        if (i >= frame_register_count(top))
            break;
        copy(list_get(inputs, i), frame_register(top, i));
    }
}

void Stack__stack_pop(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_pop(self);
}

void Stack__set_state_input(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    if (stack_top(self) == NULL)
        return circa_output_error(stack, "No stack frame");

    // find state input
    Block* block = stack_top(self)->block;
    caValue* stateSlot = NULL;
    for (int i=0;; i++) {
        Term* input = get_input_placeholder(block, i);
        if (input == NULL)
            break;
        if (is_state_input(input)) {
            stateSlot = get_top_register(self, input);
            break;
        }
    }

    if (stateSlot == NULL)
        // No-op if block doesn't expect state
        return;

    copy(circa_input(stack, 1), stateSlot);
}

void Stack__get_state_output(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);

    if (stack_top(self) == NULL)
        return circa_output_error(stack, "No stack frame");

    // find state output
    Block* block = stack_top(self)->block;
    caValue* stateSlot = NULL;
    for (int i=0;; i++) {
        Term* output = get_output_placeholder(block, i);
        if (output == NULL)
            break;
        if (is_state_output(output)) {
            stateSlot = get_top_register(self, output);
            break;
        }
    }

    if (stateSlot == NULL) {
        // Couldn't find outgoing state
        set_null(circa_output(stack, 0));
        return;
    }

    copy(stateSlot, circa_output(stack, 0));
}

void Stack__reset(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_reset(self);
}

void Stack__restart(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    stack_restart(self);
}

void Stack__run(caStack* stack)
{
    Stack* self = as_stack(circa_input(stack, 0));
    ca_assert(self != NULL);
    run_interpreter(self);
}

void Stack__frame(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);
    Frame* frame = frame_by_depth(self, index);

    copy_stack_frame_to_boxed(frame, circa_output(stack, 0));
}

void Stack__output(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);
    int index = circa_int_input(stack, 1);

    Frame* frame = stack_top(self);
    Term* output = get_output_placeholder(frame->block, index);
    if (output == NULL)
        set_null(circa_output(stack, 0));
    else
        copy(frame_register(frame, output), circa_output(stack, 0));
}

void Stack__errored(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    set_bool(circa_output(stack, 0), stack_errored(self));
}

void Stack__error_message(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));

    Frame* frame = stack_top(self);

    if (frame->pc >= frame_register_count(frame)) {
        set_string(circa_output(stack, 0), "");
        return;
    }

    caValue* errorReg = frame_register(frame, frame->pc);

    if (is_string(errorReg))
        set_string(circa_output(stack, 0), as_cstring(errorReg));
    else
        set_string(circa_output(stack, 0), to_string(errorReg).c_str());
}

void Stack__toString(caStack* stack)
{
    Stack* self = (Stack*) get_pointer(circa_input(stack, 0));
    ca_assert(self != NULL);

    stack_to_string(self, circa_output(stack, 0));
}

void reflect__caller(caStack* stack)
{
    Frame* frame = stack_top_parent(stack);
    Frame* callerFrame = frame_parent(frame);
    Term* theirCaller = frame_current_term(callerFrame);
    set_term_ref(circa_output(stack, 0), theirCaller);
}

void reflect_get_frame_state(caStack* stack)
{
    Frame* frame = stack_top_parent(stack);
    Frame* callerFrame = frame_parent(frame);

    if (is_null(&callerFrame->state))
        set_null(circa_output(stack, 0));
    else
        copy(list_get(&callerFrame->state, frame->parentPc), circa_output(stack, 0));
}

void interpreter_install_functions(Block* kernel)
{
    static const ImportRecord records[] = {
        {"Frame.active_value", Frame__active_value},
        {"Frame.set_active_value", Frame__set_active_value},
        {"Frame.block", Frame__block},
        {"Frame.parent", Frame__parent},
        {"Frame.has_parent", Frame__has_parent},
        {"Frame.register", Frame__register},
        {"Frame.registers", Frame__registers},
        {"Frame.pc", Frame__pc},
        {"Frame.parentPc", Frame__parentPc},
        {"Frame.current_term", Frame__current_term},
        {"Frame.extract_state", Frame__extract_state},

        {"make_stack", make_stack},
        {"capture_stack", capture_stack},
        {"Stack.block", Stack__block},
        {"Stack.dump", Stack__dump},
        {"Stack.find_active_frame_for_term", Stack__find_active_frame_for_term},
        {"Stack.inject", Stack__inject_state},
        {"Stack.inject_context", Stack__inject_context},
        {"Stack.apply", Stack__call},
        {"Stack.call", Stack__call},
        {"Stack.stack_push", Stack__stack_push},
        {"Stack.stack_pop", Stack__stack_pop},
        {"Stack.set_state_input", Stack__set_state_input},
        {"Stack.get_state_output", Stack__get_state_output},
        {"Stack.reset", Stack__reset},
        {"Stack.restart", Stack__restart},
        {"Stack.run", Stack__run},
        {"Stack.frame", Stack__frame},
        {"Stack.output", Stack__output},
        {"Stack.errored", Stack__errored},
        {"Stack.error_message", Stack__error_message},
        {"Stack.toString", Stack__toString},
        {"reflect:caller", reflect__caller},
        {"reflect_get_frame_state", reflect_get_frame_state},

        {NULL, NULL}
    };

    install_function_list(kernel, records);

    TYPES.frame = circa_find_type_local(kernel, "Frame");
}

CIRCA_EXPORT caStack* circa_create_stack(caWorld* world)
{
    return create_stack(world);
}

CIRCA_EXPORT void circa_free_stack(caStack* stack)
{
    free_stack(stack);
}

CIRCA_EXPORT bool circa_has_error(caStack* stack)
{
    return stack_errored(stack);
}

CIRCA_EXPORT void circa_clear_error(caStack* stack)
{
    stack_ignore_error(stack);
}

CIRCA_EXPORT void circa_clear_stack(caStack* stack)
{
    stack_reset(stack);
}

CIRCA_EXPORT void circa_restart(caStack* stack)
{
    stack_restart(stack);
}

CIRCA_EXPORT bool circa_push_function_by_name(caStack* stack, const char* name)
{
    caBlock* func = circa_find_function(NULL, name);

    if (func == NULL) {
        // TODO: Save this error on the stack instead of stdout
        std::cout << "Function not found: " << name << std::endl;
        return false;
    }

    circa_push_function(stack, func);
    return true;
}

CIRCA_EXPORT void circa_push_function(caStack* stack, caBlock* func)
{
    block_finish_changes(func);
    stack_push(stack, func);
}

CIRCA_EXPORT void circa_push_module(caStack* stack, const char* name)
{
    Value nameStr;
    set_string(&nameStr, name);
    Block* block = find_module(stack->world->root, &nameStr);
    if (block == NULL) {
        // TODO: Save this error on the stack instead of stdout
        std::cout << "in circa_push_module, module not found: " << name << std::endl;
        return;
    }
    stack_push(stack, block);
}

CIRCA_EXPORT caValue* circa_frame_input(caStack* stack, int index)
{
    Frame* top = stack_top(stack);
    
    if (top == NULL)
        return NULL;

    Term* term = top->block->get(index);

    if (term->function != FUNCS.input)
        return NULL;
    
    return get_top_register(stack, term);
}

CIRCA_EXPORT caValue* circa_frame_output(caStack* stack, int index)
{
    Frame* top = stack_top(stack);

    int realIndex = top->block->length() - index - 1;

    Term* term = top->block->get(realIndex);
    if (term->function != FUNCS.output)
        return NULL;

    return get_top_register(stack, term);
}

CIRCA_EXPORT void circa_run(caStack* stack)
{
    run_interpreter(stack);
}

CIRCA_EXPORT void circa_pop(caStack* stack)
{
    stack_pop(stack);
}

CIRCA_EXPORT caBlock* circa_stack_top_block(caStack* stack)
{
    return (caBlock*) stack_top(stack)->block;
}

CIRCA_EXPORT caValue* circa_input(caStack* stack, int index)
{
    return get_input(stack, index);
}

CIRCA_EXPORT int circa_num_inputs(caStack* stack)
{
    return num_inputs(stack);
}

CIRCA_EXPORT int circa_int_input(caStack* stack, int index)
{
    return circa_int(circa_input(stack, index));
}

CIRCA_EXPORT float circa_float_input(caStack* stack, int index)
{
    return circa_to_float(circa_input(stack, index));
}

CIRCA_EXPORT float circa_bool_input(caStack* stack, int index)
{
    return circa_bool(circa_input(stack, index));
}

CIRCA_EXPORT const char* circa_string_input(caStack* stack, int index)
{
    return circa_string(circa_input(stack, index));
}

CIRCA_EXPORT caValue* circa_output(caStack* stack, int index)
{
    return get_output(stack, index);
}

CIRCA_EXPORT void circa_output_error(caStack* stack, const char* msg)
{
    set_error_string(circa_output(stack, 0), msg);
    raise_error(stack);
}

CIRCA_EXPORT caTerm* circa_caller_input_term(caStack* stack, int index)
{
    return circa_term_get_input(circa_caller_term(stack), index);
}

CIRCA_EXPORT caBlock* circa_caller_block(caStack* stack)
{
    Frame* frame = stack_top_parent(stack);
    if (frame == NULL)
        return NULL;
    return frame->block;
}

CIRCA_EXPORT caTerm* circa_caller_term(caStack* stack)
{
    Frame* frame = stack_top_parent(stack);
    return frame->block->get(frame->pc);
}

CIRCA_EXPORT void circa_dump_stack_trace(caStack* stack)
{
    Value str;
    stack_trace_to_string(stack, &str);
    write_log(as_cstring(&str));
}

CIRCA_EXPORT caValue* circa_inject_context(caStack* stack, const char* name)
{
    Value nameVal;
    set_symbol_from_string(&nameVal, name);
    return context_inject(stack, &nameVal);
}

} // namespace circa
