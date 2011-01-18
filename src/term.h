// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

#include "branch.h"
#include "references.h"
#include "ref_list.h"
#include "tagged_value.h"
#include "term_source_location.h"
#include "types/dict.h"

namespace circa {

struct UniqueName
{
    std::string name;
    std::string base;
    int ordinal;
    UniqueName() : ordinal(0) {}
};

struct Term : TaggedValue
{
    // Inherited from TaggedValue:
    //   TaggedValue::Data value_data
    //   Type* value_type

    // A Type term that describes our data type
    Ref type;

    // Input terms
    RefList inputs;

    // Our function: the thing that takes our inputs and produces a value.
    Ref function;

    // Our name binding.
    std::string name;

    // A name which is unique across this branch.
    UniqueName uniqueName;

    // The branch that owns this term. May be NULL
    Branch* owningBranch;

    // The index that this term currently holds inside owningBranch
    int index;

    // Code which is nested inside this term. Usually this is empty.
    // This is a new construct, code is still being refactored to use
    // this.
    Branch nestedContents;

    // A globally unique ID
    unsigned int globalID;

    // Dynamic properties
    Dict properties;

    // Reference count.
    int refCount;

    // Terms which are using this term as an input.
    RefList users;

    // Location in textual source code.
    TermSourceLocation sourceLoc;

    Term();
    ~Term();

    Term* input(int index) const;
    int numInputs() const;

    std::string toString();

    // Returns the named property
    TaggedValue* property(std::string const& name);

    bool hasProperty(std::string const& name);
    TaggedValue* addProperty(std::string const& name, Term* type);
    void removeProperty(std::string const& name);

    int intProp(std::string const& name);
    float floatProp(std::string const& name);
    bool boolProp(std::string const& name);
    std::string const& stringProp(std::string const& name);

    void setIntProp(std::string const& name, int i);
    void setFloatProp(std::string const& name, float f);
    void setBoolProp(std::string const& name, bool b);
    void setStringProp(std::string const& name, std::string const& s);

    int intPropOptional(std::string const& name, int defaultValue);
    float floatPropOptional(std::string const& name, float defaultValue);
    bool boolPropOptional(std::string const& name, bool defaultValue);
    std::string stringPropOptional(std::string const& name, std::string const& defaultValue);
};

// Allocate a new Term object.
Term* alloc_term();

void assert_term_invariants(Term* t);

} // namespace circa
