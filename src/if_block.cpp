// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "building.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "locals.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"

#include "if_block.h"

namespace circa {

// The format of if_block is as follows:
//
// N = branch length
//
// {
//   [0] if(cond0) : Branch
//   [1] if(cond1) : Branch
//   ...
//   [N-2] branch()  (this corresponds to the 'else' block)
//   [N-1] #joining = branch() 
//

void update_if_block_joining_branch(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);

    // Create the joining contents if necessary
    if (!contents->contains("#joining"))
        create_branch(contents, "#joining");

    Branch* joining = nested_contents(contents->get("#joining"));
    clear_branch(joining);

    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;

    for (int branch_index=0; branch_index < contents->length()-1; branch_index++) {
        Term* term = contents->get(branch_index);
        Branch* branch = nested_contents(term);

        TermNamespace::const_iterator it;
        for (it = branch->names.begin(); it != branch->names.end(); ++it) {
            std::string const& name = it->first;

            // Ignore empty or hidden names
            if (name == "" || name[0] == '#') {
                continue;
            }

            boundNames.insert(it->first);
        }
    }

    Branch* outerScope = ifCall->owningBranch;
    ca_assert(outerScope != NULL);

    // Filter out some names from boundNames.
    for (std::set<std::string>::iterator it = boundNames.begin(); it != boundNames.end();)
    {
        std::string const& name = *it;

        // We only rebind names that are either 1) already bound in the outer scope, or
        // 2) bound in every possible branch.
        
        bool boundInOuterScope = find_name(outerScope, name.c_str()) != NULL;

        bool boundInEveryBranch = true;

        for (int branch_index=0; branch_index < contents->length()-1; branch_index++) {
            Branch* branch = nested_contents(contents->get(branch_index));
            if (!branch->contains(name))
                boundInEveryBranch = false;
        }

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    int numBranches = contents->length() - 1;

    // For each name, create a term that selects the correct version of this name.
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it)
    {
        std::string const& name = *it;

        TermList inputs;
        inputs.resize(numBranches);

        Term* outerVersion = get_named_at(ifCall, name);

        for (int i=0; i < numBranches; i++) {
            Term* local = contents->get(i)->contents(name.c_str());
            inputs.setAt(i, local == NULL ? outerVersion : local);
        }

        apply(joining, JOIN_FUNC, inputs, name);
    }

    finish_update_cascade(joining);
}

int if_block_num_branches(Term* ifCall)
{
    return nested_contents(ifCall)->length() - 1;
}
Branch* if_block_get_branch(Term* ifCall, int index)
{
    return ifCall->contents(index)->contents();
}

CA_FUNCTION(evaluate_if_block)
{
    Term* caller = CALLER;
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(caller);
    bool useState = has_any_inlined_state(contents);

    int numBranches = contents->length() - 1;
    int acceptedBranchIndex = 0;
    Branch* acceptedBranch = NULL;

    TaggedValue output;

    TaggedValue localState;
    TaggedValue prevScopeState;
    List* state = NULL;
    if (useState) {
        swap(&prevScopeState, &context->currentScopeState);
        fetch_state_container(caller, &prevScopeState, &localState);
        state = List::lazyCast(&localState);
        state->resize(numBranches);
    }

    for (int branchIndex=0; branchIndex < numBranches; branchIndex++) {
        Term* branch = contents->get(branchIndex);

        //std::cout << "checking: " << get_term_to_string_extended(branch) << std::endl;
        //std::cout << "with stack: " << STACK->toString() << std::endl;

        // Look at input
        TaggedValue inputIsn;
        write_input_instruction(branch, branch->input(0), &inputIsn);
        
        if (branch->input(0) == NULL || as_bool(get_arg(context, &inputIsn))) {

            acceptedBranch = branch->nestedContents;

            ca_assert(acceptedBranch != NULL);

            push_frame(context, acceptedBranch);

            if (useState)
                swap(state->get(branchIndex), &context->currentScopeState);

            // Evaluate each term
            for (int j=0; j < acceptedBranch->length(); j++) {
                evaluate_single_term(context, acceptedBranch->get(j));
                if (evaluation_interrupted(context))
                    break;
            }

            // Save result
            Term* outputTerm = find_last_non_comment_expression(acceptedBranch);
            if (outputTerm != NULL)
                copy(outputTerm, &output);

            if (useState)
                swap(state->get(branchIndex), &context->currentScopeState);

            acceptedBranchIndex = branchIndex;
            break;
        }
    }

    // Reset state for all non-accepted branches
    if (useState) {
        for (int i=0; i < numBranches; i++) {
            if ((i != acceptedBranchIndex))
                set_null(state->get(i));
        }
        save_and_consume_state(caller, &prevScopeState, &localState);
        swap(&prevScopeState, &context->currentScopeState);
    }

    // Copy joined values to output slots
    Branch* joining = nested_contents(contents->get(contents->length()-1));

    for (int i=0; i < joining->length(); i++) {
        Term* joinTerm = joining->get(i);

        // Fetch the result value using an input instruction
        TaggedValue inputIsn;
        write_input_instruction(joinTerm, joinTerm->input(acceptedBranchIndex), &inputIsn);
        TaggedValue* value = get_arg(context, &inputIsn);

        // Write the result value to the corresponding place in an above frame.
        Term* outputTerm = caller->owningBranch->get(caller->index + 1 + i);
        Frame* upperFrame = get_frame(CONTEXT, 1);

        copy(value, upperFrame->registers[outputTerm->index]);
    }

    pop_frame(context);

    swap(&output, OUTPUT);
}

} // namespace circa
