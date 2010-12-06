// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

#include "names.h"
#include "ref_list.h"
#include "term_namespace.h"
#include "types/list.h"

namespace circa {

struct Branch
{
    RefList _terms;

    TermNamespace names;

    // Points to the Term which owns this branch as a value.
    Term* owningTerm;

    int _refCount;

    int registerCount;
    int outputRegister;

    // Local values for each term. These are the results of the most recent
    // interpreted execution of this branch.
    // (This list makes it impossible for the same branch to be simultaneously
    // executed by different threads, but that problem will eventually be
    // revisited).
    List locals;

    // While the interpreter is using a branch, it can be marked 'inuse' and
    // it can have a list of old local lists. This is to support recursion.
    bool inuse;
    List localsStack;

    // For a branch loaded from a file, this keeps track of the file signature
    // of the last time this was loaded.
    List fileSignature;

    Branch();
    ~Branch();

    int length() const;

    Term* get(int index) const;
    Term* operator[](int index) const { return get(index); }

    // Get a term from a name binding.
    inline Term* get(std::string const& name) const { return get_named(*this, name); }
    inline Term* operator[](std::string const& name) const { return get_named(*this, name); }

    // Returns true if there is a term with the given name
    bool contains(std::string const& name) const;

    int getIndex(Term* term) const;
    int debugFindIndex(Term* term) const;

    Term* last() const;

    // Find the first term with the given name binding.
    Term* findFirstBinding(std::string const& name) const;

    // Find the last term with the given name binding.
    Term* findLastBinding(std::string const& name) const;

    // Find a term with the given name, returns -1 if not found.
    int findIndex(std::string const& name) const;
    int findIndex(const char* name) const;

    void set(int index, Term* term);
    void setNull(int index);

    void insert(int index, Term* term);

    void append(Term* term);
    Term* appendNew();

    void move(Term* term, int index);
    void moveToEnd(Term* term);

    void remove(Term* term);
    void remove(std::string const& name);
    void remove(int index);
    void removeNulls();
    void shorten(int newLength);

    // Bind a name to a term
    void bindName(Term* term, std::string name);

    // Remap pointers
    void remapPointers(ReferenceMap const& map);

    void clear();

    // Compile the given statement, return the result term.
    Term* compile(std::string const& statement);

    // Evaluate the given statement, return the result value.
    Term* eval(std::string const& statement);

    std::string toString();

private:
    // Disabled calls
    Branch(Branch const& copy) {}
    Branch& operator=(Branch const& b) { return *this; }
};

bool is_namespace(Term* term);
bool is_namespace(Branch& branch);

std::string get_branch_source_filename(Branch& branch);
Branch* get_outer_scope(Branch const& branch);

void duplicate_branch(Branch& source, Branch& dest);

void parse_script(Branch& branch, std::string const& filename);
void evaluate_script(Branch& branch, std::string const& filename);

Term* find_term_by_id(Branch& branch, unsigned int id);

void persist_branch_to_file(Branch& branch);
std::string get_source_file_location(Branch& branch);

} // namespace circa
