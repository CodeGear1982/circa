// Copyright 2008 Andrew Fischer

#ifndef CIRCA_INTROSPECTION_INCLUDED
#define CIRCA_INTROSPECTION_INCLUDED

namespace circa {

bool is_value(Term* term);
bool has_inner_branch(Term* term);
Branch* get_inner_branch(Term* term);
std::string get_short_local_name(Term* term);
std::string term_to_raw_string(Term* term);
void print_raw_term(Term* term, std::ostream &output);
void print_raw_branch(Branch& branch, std::ostream &output);
void print_terms(RefList const& list, std::ostream &output);
bool is_equivalent(Term* target, Term* function, RefList const& inputs);
Term* find_equivalent(Term* function, RefList const& inputs);
void print_runtime_errors(Branch& branch, std::ostream& output);
bool has_compile_errors(Branch& branch);
std::vector<std::string> get_compile_errors(Branch& branch);
void print_compile_errors(Branch& branch, std::ostream& output);

}

#endif
