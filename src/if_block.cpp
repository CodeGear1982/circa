// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <set>

#include "common_headers.h"

#include "branch.h"
#include "kernel.h"
#include "building.h"
#include "evaluation.h"
#include "importing_macros.h"
#include "locals.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "if_block.h"

namespace circa {

int if_block_count_cases(Term* term)
{
    Branch* contents = nested_contents(term);
    int result = 0;
    for (int i=0; i < contents->length(); i++)
        if (contents->get(i) != NULL && contents->get(i)->function == CASE_FUNC)
            result++;
    return result;
}

Term* if_block_get_case(Term* term, int index)
{
    Branch* contents = nested_contents(term);
    for (int i=0; i < contents->length(); i++) {
        if (contents->get(i) == NULL || contents->get(i)->function != CASE_FUNC)
            continue;

        if (index == 0)
            return contents->get(i);

        index--;
    }
    return NULL;
}

bool if_block_is_name_bound_in_every_case(Branch* contents, const char* name)
{
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function == CASE_FUNC)
            if (!nested_contents(term)->contains(name))
                return false;
    }
    return true;
}

void if_block_update_master_placeholders(Term* ifCall)
{
    Branch* contents = nested_contents(ifCall);

    // Find the set of all names bound in every branch.
    std::set<std::string> boundNames;
    bool hasState = false;

    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);

        if (term->function == INPUT_PLACEHOLDER_FUNC)
            continue;
        if (term->function != CASE_FUNC)
            break;

        Branch* branch = nested_contents(term);

        hasState = hasState || find_state_input(branch) != NULL;

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

        bool boundInEveryBranch = if_block_is_name_bound_in_every_case(contents, name.c_str());;

        if (!boundInOuterScope && !boundInEveryBranch)
            boundNames.erase(it++);
        else
            ++it;
    }

    // Add each name as an input and output to the if_block
    int inputIndex = 0;
    for (std::set<std::string>::const_iterator it = boundNames.begin();
            it != boundNames.end();
            ++it) {

        std::string const& name = *it;
        Term* outer = get_named_at(ifCall, name);
        set_input(ifCall, inputIndex, outer);

        Term* placeholder = apply(contents, INPUT_PLACEHOLDER_FUNC, TermList(), name);
        contents->move(placeholder, inputIndex);
        inputIndex++;

        apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(NULL), name);
    }

    // If any branches have a state input, then add a master state input too.
    if (hasState) {
        insert_state_input(contents);
        insert_state_output(contents);
    }
}

void if_block_update_case_placeholders_from_master(Term* ifCall, Term* caseTerm)
{
    Branch* masterContents = nested_contents(ifCall);
    Branch* caseContents = nested_contents(caseTerm);

    // Create input placeholders
    for (int i=0;; i++) {
        Term* masterPlaceholder = get_input_placeholder(masterContents, i);
        if (masterPlaceholder == NULL)
            break;

        if (is_state_input(masterPlaceholder) && has_state_input(caseContents))
            continue;

        Term* placeholder = apply(caseContents, INPUT_PLACEHOLDER_FUNC, TermList(),
            masterPlaceholder->name);
        copy(&masterPlaceholder->properties, &placeholder->properties);
        caseContents->move(placeholder, i);
    }

    // Create output placeholders
    for (int i=0;; i++) {
        Term* masterPlaceholder = get_output_placeholder(masterContents, i);
        if (masterPlaceholder == NULL)
            break;

        Term* nameResult = get_named_at(caseContents, caseContents->length(),
            masterPlaceholder->name.c_str());

        Term* placeholder = apply(caseContents, OUTPUT_PLACEHOLDER_FUNC,
            TermList(nameResult), masterPlaceholder->name);
        change_declared_type(placeholder, nameResult->type);
        copy(&masterPlaceholder->properties, &placeholder->properties);
    }

#if 0
    // For each case branch, create a list of input_placeholder() and output_placeholder()
    // terms. This list should be equivalent across all cases.
    for (int caseIndex=firstCaseIndex;; caseIndex++) {
        Term* caseTerm = contents->getSafe(caseIndex);
        if (caseTerm == NULL || caseTerm->function != CASE_FUNC)
            break;

        Branch* caseContents = nested_contents(caseTerm);

        int inputPos = 0;

        for (std::set<std::string>::const_iterator it = boundNames.begin();
                it != boundNames.end();
                ++it) {
            std::string const& name = *it;

            Term* outerVersion = get_named_at(ifCall, name.c_str());
            Term* nameResult = find_name(caseContents, name.c_str());

            Term* inputPlaceholder = apply(caseContents, INPUT_PLACEHOLDER_FUNC,
                TermList(), name);
            caseContents->move(inputPlaceholder, inputPos++);

            if (nameResult->owningBranch != caseContents)
                nameResult = inputPlaceholder;

            Term* placeholder = apply(caseContents, OUTPUT_PLACEHOLDER_FUNC,
                TermList(nameResult), name);
            change_declared_type(placeholder, nameResult->type);

            // Also, now that we have an input_placeholder(), go through our terms
            // and rebind anyone that is using the outer version.
            for (int i=0; i < caseContents->length(); i++)
                remap_pointers_quick(caseContents->get(i), outerVersion, inputPlaceholder);
        }

        // Also add an output_placeholder for the primary output.
        apply(caseContents, OUTPUT_PLACEHOLDER_FUNC,
            TermList(find_last_non_comment_expression(caseContents)));
    }

    // Now that each case branch has an output_placeholder list, create a master list
    // in the above if_block branch.
    for (int outputIndex=0; outputIndex < numOutputs; outputIndex++) {

        List typeList;
        std::string name;

        for (int caseIndex=firstCaseIndex;; caseIndex++) {
            Term* caseTerm = contents->getSafe(caseIndex);
            if (caseTerm == NULL || caseTerm->function != CASE_FUNC)
                break;

            Term* placeholder = caseTerm->contents()->getFromEnd(numOutputs - outputIndex - 1);
            set_type(typeList.append(), placeholder->type);
            name = placeholder->name;
        }

        Term* masterPlaceholder = apply(contents, OUTPUT_PLACEHOLDER_FUNC, TermList(NULL), name);
        change_declared_type(masterPlaceholder, find_common_type(&typeList));
    }
#endif
}

void if_block_update_output_placeholder_types_from_cases(Term* ifBlock)
{
    // TODO
}

void finish_if_block(Term* ifBlock)
{
    if_block_update_master_placeholders(ifBlock);

    Branch* contents = nested_contents(ifBlock);
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function == CASE_FUNC)
            if_block_update_case_placeholders_from_master(ifBlock, term);
    }

    if_block_update_output_placeholder_types_from_cases(ifBlock);
}

CA_FUNCTION(evaluate_if_block)
{
    Term* caller = CALLER;
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(caller);

    Branch* acceptedBranch = NULL;

    TaggedValue output;

    TaggedValue localState;
    TaggedValue prevScopeState;

    int termIndex = 0;
    while (contents->get(termIndex)->function == INPUT_PLACEHOLDER_FUNC)
        termIndex++;

    for (; termIndex < contents->length(); termIndex++) {
        Term* caseTerm = contents->get(termIndex);

        //std::cout << "checking: " << get_term_to_string_extended(caseTerm) << std::endl;
        //std::cout << "with stack: " << STACK->toString() << std::endl;

        // Look at input
        TaggedValue inputIsn;
        write_input_instruction(caseTerm, caseTerm->input(0), &inputIsn);
        
        if (caseTerm->input(0) == NULL || as_bool(get_arg(context, &inputIsn))) {

            acceptedBranch = caseTerm->nestedContents;

            ca_assert(acceptedBranch != NULL);

            // Copy inputs
            List registers;
            registers.resize(acceptedBranch->length());
            int numInputs = NUM_INPUTS;
            for (int inputIndex=0; inputIndex < numInputs; inputIndex++)
                copy(INPUT(inputIndex), registers[inputIndex]);

            push_frame(context, acceptedBranch, &registers);

            // Evaluate each term
            for (int i=0; i < acceptedBranch->length(); i++) {
                evaluate_single_term(context, acceptedBranch->get(i));
                if (evaluation_interrupted(context))
                    break;
            }

            swap(&registers, &top_frame(context)->registers);
            pop_frame(context);

            // Save outputs
            for (int i=0;; i++) {
                Term* placeholder = get_output_placeholder(acceptedBranch, i);
                if (placeholder == NULL)
                    break;

                copy(registers[placeholder->index], OUTPUT_NTH(i));
            }

            return;
        }
    }
}

} // namespace circa
