// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "debug.h"
#include "function.h"
#include "kernel.h"
#include "inspection.h"
#include "list.h"
#include "modules.h"
#include "names_builtin.h"
#include "string_type.h"
#include "switch.h"
#include "symbols.h"
#include "term.h"
#include "world.h"

#include "names.h"

namespace circa {

// run_name_search: takes a NameSearch object and actually performs the search.
// There are many variations of find_name and find_local_name which all just wrap
// around this function.
Term* run_name_search(NameSearch* params);

bool exposes_nested_names(Term* term);

bool fits_lookup_type(Term* term, Symbol type)
{
    switch (type) {
        case s_LookupAny:
            return true;
        case s_LookupType:
            return is_type(term);
        case s_LookupFunction:
            return is_function(term);
    }
    internal_error("unknown type in fits_lookup_type");
    return false;
}

#if 0
Block* find_module_for_require_statement(Term* term)
{
    if (term->input(0) == NULL)
        return NULL;

    Value* moduleName = term_value(term->input(0));

    return find_module(term->owningBlock, moduleName);
}
#endif

Block* find_builtins_block(Block* block)
{
    Block* builtins = global_world()->builtins;
    return builtins;
}

Term* run_name_search(NameSearch* params)
{
    stat_increment(NameSearch);

    // Temp: Input name can be a symbol, but convert it to a string for now.
    if (is_symbol(&params->name)) {
        Value temp;
        move(&params->name, &temp);
        symbol_to_string(&temp, &params->name);
    }

    if (is_null(&params->name) || string_equals(&params->name, ""))
        return NULL;

    Block* block = params->block;

    if (block == NULL)
        return NULL;

    int position = 0;
    
    if (is_symbol(&params->position) && as_symbol(&params->position) == s_last)
        position = block->length();
    else
        position = as_int(&params->position);

    if (position > block->length())
        position = block->length();

    // Look for an exact match.
    for (int i = position - 1; i >= 0; i--) {

        stat_increment(NameSearchStep);

        Term* term = block->get(i);
        if (term == NULL)
            continue;

        if (equals(&term->nameValue, &params->name)
                && fits_lookup_type(term, params->lookupType)
                && (params->ordinal == -1 || term->uniqueOrdinal == params->ordinal))
            return term;

        // If this term exposes its names, then search inside the nested block.
        // (Deprecated, I think).
        if (term->nestedContents != NULL && exposes_nested_names(term)) {
            NameSearch nestedSearch;
            nestedSearch.block = term->nestedContents;
            set_value(&nestedSearch.name, &params->name);
            set_symbol(&nestedSearch.position, s_last);
            nestedSearch.ordinal = -1;
            nestedSearch.lookupType = params->lookupType;
            nestedSearch.searchParent = false;
            Term* found = run_name_search(&nestedSearch);
            if (found != NULL)
                return found;
        }

        #if 0
        // Check for an 'import' statement. If found, continue this search in the designated module.
        if (term->function == FUNCS.require && term->boolProp(s_Syntax_Import, false)) {
            Block* module = find_module_for_require_statement(term);
            if (module != NULL) {
                NameSearch moduleSearch;
                moduleSearch.block = module;
                set_value(&moduleSearch.name, &params->name);
                set_symbol(&moduleSearch.position, s_last);
                moduleSearch.ordinal = -1;
                moduleSearch.lookupType = params->lookupType;
                moduleSearch.searchParent = false;
                Term* found = run_name_search(&moduleSearch);
                if (found != NULL)
                    return found;
            }
        }
        #endif
    }

    // Did not find in the local block. Possibly continue this search upwards.
    
    if (!params->searchParent)
        return NULL;

    // Possibly take this search to the builtins block.
    if ((get_parent_block(block) == NULL) || is_module(block)) {
        NameSearch builtinsSearch;
        builtinsSearch.block = find_builtins_block(block);
        set_value(&builtinsSearch.name, &params->name);
        set_symbol(&builtinsSearch.position, s_last);
        builtinsSearch.lookupType = params->lookupType;
        builtinsSearch.ordinal = -1;
        builtinsSearch.searchParent = false;
        return run_name_search(&builtinsSearch);
    }

    // Search parent

    // The choice of position is a little weird. For regular name searches,
    // we start at the parent term's position (ie, search all the terms that
    // came before the parent).
    //
    // For a LookupFunction search, start at the bottom of the branch. It's okay
    // for a term to use a function that occurs after the term.
    
    NameSearch parentSearch;

    Term* parentTerm = block->owningTerm;
    if (parentTerm == NULL)
        return NULL;

    parentSearch.block = parentTerm->owningBlock;
    if (params->lookupType == s_LookupFunction)
        set_symbol(&parentSearch.position, s_last);
    else
        set_int(&parentSearch.position, parentTerm->index + 1);

    set_value(&parentSearch.name, &params->name);
    parentSearch.lookupType = params->lookupType;
    parentSearch.ordinal = -1;
    parentSearch.searchParent = true;
    return run_name_search(&parentSearch);
}

Term* find_name(Block* block, Value* name, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    copy(name, &nameSearch.name);
    set_symbol(&nameSearch.position, s_last);
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_local_name(Block* block, Value* name, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    copy(name, &nameSearch.name);
    set_symbol(&nameSearch.position, s_last);
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = false;
    return run_name_search(&nameSearch);
}

Term* find_name(Block* block, const char* nameStr, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    set_string(&nameSearch.name, nameStr);
    set_symbol(&nameSearch.position, s_last);
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_local_name(Block* block, const char* nameStr, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    set_string(&nameSearch.name, nameStr);
    set_symbol(&nameSearch.position, s_last);
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = false;
    return run_name_search(&nameSearch);
}

Term* find_local_name_at_position(Block* block, Value* name, Value* position)
{
    NameSearch nameSearch;
    nameSearch.block = block;
    set_value(&nameSearch.name, name);
    set_value(&nameSearch.position, position);
    nameSearch.ordinal = -1;
    nameSearch.lookupType = s_LookupAny;
    nameSearch.searchParent = false;
    return run_name_search(&nameSearch);
}

Term* find_name_at(Term* term, const char* name, Symbol lookupType)
{
    NameSearch nameSearch;
    nameSearch.block = term->owningBlock;
    set_string(&nameSearch.name, name);
    set_int(&nameSearch.position, term->index);
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

Term* find_name_at(Value* location, Value* name, Symbol lookupType)
{
    NameSearch nameSearch;

    copy(name, &nameSearch.name);

    if (is_block(location)) {
        Block* block = as_block(location);
        nameSearch.block = block;
        set_symbol(&nameSearch.position, s_last);
    } else if (is_term_ref(location)) {
        Term* term = as_term_ref(location);
        nameSearch.block = term->owningBlock;
        set_int(&nameSearch.position, term->index);
    } else {
        internal_error("find_name_at");
    }
    nameSearch.ordinal = -1;
    nameSearch.lookupType = lookupType;
    nameSearch.searchParent = true;
    return run_name_search(&nameSearch);
}

int name_find_qualified_separator(const char* name)
{
    for (int i=0; name[i] != 0; i++) {
        if (name[i] == ':' && name[i+1] != 0)
            return i;
    }
    return -1;
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int name_find_ordinal_suffix(const char* name, int* endPos)
{
    // Walk backwards, see if this name even has an ordinal suffix.
    int search = *endPos - 1;
    if (*endPos == -1)
        search = (int) strlen(name) - 1;

    bool foundADigit = false;
    bool foundOrdinalSuffix = false;

    while (true) {
        if (search < 0)
            break;

        if (is_digit(name[search])) {
            foundADigit = true;
            search--;
            continue;
        }

        if (foundADigit && name[search] == '#') {
            // Found a suffix of the form #123.
            foundOrdinalSuffix = true;
            *endPos = search;
            break;
        }

        break;
    }

    if (!foundOrdinalSuffix)
        return -1;

    // Parse and return the ordinal number.
    return atoi(name + *endPos + 1);
}

bool exposes_nested_names(Term* term)
{
    if (term->nestedContents == NULL)
        return false;
    if (term->function == FUNCS.include_func)
        return true;

    return false;
}

bool name_is_reachable_from(Term* term, Block* block)
{
    if (term->owningBlock == block)
        return true;

    Block* parent = get_parent_block(block);

    if (parent == NULL)
        return false;

    return name_is_reachable_from(term, parent);
}

Block* find_first_common_block(Term* left, Term* right)
{
    Block* leftParent = left->owningBlock;
    Block* rightParent = right->owningBlock;

    if (leftParent == NULL) return NULL;
    if (rightParent == NULL) return NULL;

    // Walk upwards from left term.
    while (leftParent != NULL) {

        // Walk upwards from right term.
        while (rightParent != NULL) {
            if (leftParent == rightParent)
                return leftParent;

            rightParent = get_parent_block(rightParent);
        }

        leftParent = get_parent_block(leftParent);
        rightParent = right->owningBlock;
    }

    return NULL;
}

bool term_is_child_of_block(Term* term, Block* block)
{
    while (term != NULL) {
        if (term->owningBlock == block)
            return true;

        term = parent_term(term);
    }

    return false;
}

// Returns whether or not we succeeded
bool get_relative_name_recursive(Block* block, Term* term, std::stringstream& output)
{
    if (name_is_reachable_from(term, block)) {
        output << term->name();
        return true;
    }

    Term* parentTerm = parent_term(term);

    if (parentTerm == NULL)
        return false;

    // Don't include the names of hidden or builtin blocks.
    if (is_hidden(parentTerm)) {
        output << term->name();
        return true;
    }
    
    if (parentTerm->nestedContents != NULL &&
        block_get_bool_prop(parentTerm->nestedContents, s_Builtins, false))
    {
        output << term->name();
        return true;
    }

    bool success = get_relative_name_recursive(block, parentTerm, output);

    if (!success)
        return false;

    output << ":" << term->name();
    return true;
}

std::string get_relative_name(Block* block, Term* term)
{
    ca_assert(term != NULL);

    if (name_is_reachable_from(term, block))
        return term->name();

    // Build a dot-separated name
    std::stringstream result;

    get_relative_name_recursive(block, term, result);

    return result.str();
}

std::string get_relative_name_at(Term* location, Term* term)
{
    if (location->owningBlock == NULL)
        return term->name();
    else
        return get_relative_name(location->owningBlock, term);
}

void get_relative_name_as_list(Term* term, Block* relativeTo, Value* nameOutput)
{
    set_list(nameOutput, 0);

    // Walk upwards and build the name, stop when we reach relativeTo.
    // The output list will be reversed but we'll fix that.

    while (true) {
        set_value(list_append(nameOutput), unique_name(term));

        if (term->owningBlock == relativeTo) {
            break;
        }

        term = parent_term(term);

        // If term is null, then it wasn't really a child of relativeTo
        if (term == NULL) {
            set_null(nameOutput);
            return;
        }
    }

    // Fix output list
    list_reverse(nameOutput);
}

Term* find_from_relative_name_list(Value* name, Block* relativeTo)
{
    if (is_null(name))
        return NULL;

    Term* term = NULL;
    for (int index=0; index < list_length(name); index++) {
        if (relativeTo == NULL)
            return NULL;

        term = find_from_unique_name(relativeTo, list_get(name, index));

        if (term == NULL)
            return NULL;

        relativeTo = term->nestedContents;

        // relativeTo may now be NULL. But if we reached the end of this match, that's ok.
    }
    return term;
}

void update_unique_name(Term* term)
{
    Term::UniqueName& name = term->uniqueName;

    if (term->owningBlock == NULL) {
        copy(&term->nameValue, &name.name);
        return;
    }

    copy(&term->nameValue, &name.base);

    if (string_empty(&name.base)) {
        if (term->function == NULL)
            set_string(&name.base, "_anon");
        else {
            set_string(&name.base, "_");
            string_append(&name.base, &term->function->nameValue);
        }
    }

    copy(&name.base, &name.name);
    name.ordinal = 0;

    // Look for a name collision. We might need to keep looping, if our generated name
    // collides with an existing name.

    Block* block = term->owningBlock;

    bool updatedName = true;
    while (updatedName) {
        updatedName = false;

        for (int i = term->index-1; i >= 0; i--) {
            Term* other = block->get(i);
            if (other == NULL) continue;

            // If another term shares the same base, then make sure our ordinal is
            // higher. This turns some O(n) cases into O(1)
            if (string_equals(&other->uniqueName.base, &name.base)
                    && (other->uniqueName.ordinal >= name.ordinal)) {
                name.ordinal = other->uniqueName.ordinal + 1;
                updatedName = true;

            // If this name is already used, then just try the next ordinal. This
            // case results in more blind searching, but it's necessary to handle
            // the situation where a generated name is already taken.
            } else if (string_equals(&other->uniqueName.name, &name.name)) {
                name.ordinal++;
                updatedName = true;
            }

            if (updatedName) {
                char ordinalBuf[30];
                sprintf(ordinalBuf, "%d", name.ordinal);
                copy(&name.base, &name.name);
                string_append(&name.name, "_");
                string_append(&name.name, ordinalBuf);
                break;
            }
        }
    }
}

Value* unique_name(Term* term)
{
    return &term->uniqueName.name;
}

Term* find_from_unique_name(Block* block, Value* name)
{
    // O(n) search; this should be made more efficient.

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;
        if (string_equals(unique_name(term), name)) {
            return term;
        }
    }
    return NULL;
}

Type* find_type(Block* block, const char* name)
{
    caTerm* term = find_name(block, name, s_LookupType);
    if (term == NULL)
        return NULL;
    return circa_type(circa_term_value(term));
}
Block* find_function_local(Block* block, const char* name)
{
    caTerm* term = find_name(block, name, s_LookupFunction);
    if (term == NULL)
        return NULL;
    return nested_contents(term);
}

} // namespace circa
