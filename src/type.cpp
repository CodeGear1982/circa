// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "debug_valid_objects.h"
#include "importing_macros.h"

namespace circa {

Term* IMPLICIT_TYPES = NULL;

namespace type_t {

    void decref(Type* type)
    {
        if (type->permanent)
            return;
        ca_assert(type->refCount > 0);
        //std::cout << "decref: " << type->name << std::endl;
        type->refCount--;
        if (type->refCount == 0) {
            //std::cout << "deleted: " << type->name << std::endl;
            //ca_assert(type->name != "Dict");
            delete type;
        }
    }

    void initialize(Type*, TaggedValue* value)
    {
        Type* type = Type::create();
        type->refCount++;
        set_pointer(value, type);
    }
    void release(TaggedValue* value)
    {
        ca_assert(is_type(value));
        Type* type = (Type*) get_pointer(value);
        if (type != NULL)
            decref(type);
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        ca_assert(is_type(source));
        copy((Type*) get_pointer(source), dest);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "type ", term, phrase_type::KEYWORD);
        append_phrase(source, term->name, term, phrase_type::TYPE_NAME);
        append_phrase(source, term->stringPropOptional("syntax:preLBracketWhitespace", " "),
                term, token::WHITESPACE);
        append_phrase(source, "{", term, token::LBRACKET);
        append_phrase(source, term->stringPropOptional("syntax:postLBracketWhitespace", " "),
                term, token::WHITESPACE);

        Branch& prototype = type_t::get_prototype(term);

        for (int i=0; i < prototype.length(); i++) {
            Term* field = prototype[i];
            ca_assert(field != NULL);
            append_phrase(source, field->stringPropOptional("syntax:preWhitespace",""),
                    term, token::WHITESPACE);
            append_phrase(source, field->type->name, term, phrase_type::TYPE_NAME);
            append_phrase(source, field->stringPropOptional("syntax:postNameWs"," "),
                    term, token::WHITESPACE);
            append_phrase(source, field->name, term, token::IDENTIFIER);
            append_phrase(source, field->stringPropOptional("syntax:postWhitespace",""),
                    term, token::WHITESPACE);
        }
        append_phrase(source, "}", term, token::RBRACKET);
    }

    void remap_pointers(Term *type, ReferenceMap const& map)
    {
        Branch& prototype = type_t::get_prototype(type);

        for (int field_i=0; field_i < prototype.length(); field_i++) {
            Term* orig = prototype[field_i];
            Term* remapped = map.getRemapped(orig);
            assert_valid_term(orig);
            assert_valid_term(remapped);
            prototype.set(field_i, remapped);
        }
    }

    CA_FUNCTION(name_accessor)
    {
        set_string(OUTPUT, as_type(INPUT(0)).name);
    }

    void setup_type(Term* type)
    {
        import_member_function(type, name_accessor, "name(Type) -> string");
    }

    void copy(Type* value, TaggedValue* dest)
    {
        Type* oldType = (Type*) get_pointer(dest);

        if (value == oldType)
            return;

        set_pointer(dest, value);
        value->refCount++;
        decref(oldType);
    }

    Type::RemapPointers& get_remap_pointers_func(Term* type)
    {
        return as_type(type).remapPointers;
    }
    Branch& get_prototype(Term* type)
    {
        return as_type(type).prototype;
    }
    Branch& get_prototype(Type* type)
    {
        return type->prototype;
    }
    Branch& get_attributes(Term* type)
    {
        return as_type(type).attributes;
    }
    TaggedValue* get_default_value(Type* type)
    {
        return &type->defaultValue;
    }

} // namespace type_t

Type::Type() :
    name(""),
    cppTypeInfo(NULL),
    initialize(NULL),
    release(NULL),
    copy(NULL),
    reset(NULL),
    equals(NULL),
    cast(NULL),
    staticTypeQuery(NULL),
    toString(NULL),
    formatSource(NULL),
    touch(NULL),
    getIndex(NULL),
    setIndex(NULL),
    getField(NULL),
    setField(NULL),
    numElements(NULL),
    checkInvariants(NULL),
    remapPointers(NULL),
    hashFunc(NULL),
    parent(NULL),
    refCount(0),
    permanent(false)
{
    debug_register_valid_object_ignore_dupe(this, TYPE_OBJECT);
}

Type::~Type()
{
    debug_unregister_valid_object(this);
}

Type* declared_type(Term* term)
{
    if (term->type == NULL)
        return NULL;
    return unbox_type(term->type);
}

Term* get_output_type(Term* term, int outputIndex)
{
    if (outputIndex == 0)
        return term->type;

    if (term->function == NULL)
        return ANY_TYPE;

    FunctionAttrs* attrs = get_function_attrs(term->function);

    FunctionAttrs::GetOutputType getOutputType = NULL;
    if (attrs != NULL)
        getOutputType = attrs->getOutputType;

    if (getOutputType != NULL)
        return getOutputType(term, outputIndex);

    return function_get_output_type(term->function, outputIndex);
}

Term* get_output_type(Term* term)
{
    return get_output_type(term, 0);
}

Term* get_type_of_input(Term* term, int inputIndex)
{
    if (inputIndex >= term->numInputs())
        return NULL;
    if (term->input(inputIndex) == NULL)
        return NULL;
    int outputIndex = term->inputInfo(inputIndex)->outputIndex;
    return get_output_type(term->input(inputIndex), outputIndex);
}

Type& as_type(Term *term)
{
    ca_assert(unbox_type(term) != NULL);
    return *unbox_type(term);
}

Type* unbox_type(Term* term)
{
    ca_assert(term->type == TYPE_TYPE);
    return (Type*) term->value_data.ptr;
}

Type* unbox_type(TaggedValue* val)
{
    return (Type*) val->value_data.ptr;
}

static void run_static_type_query(StaticTypeQuery* query)
{
    // Check that the subject term and subjectType match.
    if (query->subject && query->subjectType)
        ca_assert(query->subjectType == declared_type(query->subject));

    ca_assert(query->type);

    // Check that either subject or subjectType are provided.
    ca_assert(query->subjectType || query->subject);

    // Populate subjectType from subject if missing.
    if (query->subjectType == NULL)
        query->subjectType = declared_type(query->subject);

    // Always succeed if types are the same.
    if (query->subjectType == query->type)
        return query->succeed();

    // If output term is ANY type then we cannot statically determine.
    if (query->subjectType == unbox_type(ANY_TYPE))
        return query->unableToDetermine();

    // Try using the type's static query func
    Type::StaticTypeQueryFunc staticTypeQueryFunc = query->type->staticTypeQuery;
    if (staticTypeQueryFunc != NULL) {
        staticTypeQueryFunc(query->type, query);
        return;
    }

    // No static query function, and we know that the types are not equal, so
    // default behavior here is to fail.
    return query->fail();
}

StaticTypeQuery::Result run_static_type_query(Type* type, Term* term)
{
    StaticTypeQuery query;
    query.subject = term;
    query.type = type;
    run_static_type_query(&query);
    return query.result;
}

bool term_output_always_satisfies_type(Term* term, Type* type)
{
    return run_static_type_query(type, term) == StaticTypeQuery::SUCCEED;
}

bool term_output_never_satisfies_type(Term* term, Type* type)
{
    return run_static_type_query(type, term) == StaticTypeQuery::FAIL;
}

bool type_is_static_subset_of_type(Type* superType, Type* subType)
{
    StaticTypeQuery query;
    query.type = superType;
    query.subjectType = subType;
    run_static_type_query(&query);
    return query.result != StaticTypeQuery::FAIL;
}

void reset_type(Type* type)
{
    type->initialize = NULL;
    type->release = NULL;
    type->copy = NULL;
    type->reset = NULL;
    type->equals = NULL;
    type->cast = NULL;
    type->remapPointers = NULL;
    type->toString = NULL;
    type->formatSource = NULL;
    type->checkInvariants = NULL;
    type->staticTypeQuery = NULL;
    type->touch = NULL;
    type->getIndex = NULL;
    type->setIndex = NULL;
    type->getField = NULL;
    type->setField = NULL;
    type->numElements = NULL;

    clear_branch(&type->prototype);
    clear_branch(&type->attributes);
    clear_branch(&type->memberFunctions);
}

void initialize_simple_pointer_type(Type* type)
{
    reset_type(type);
}

void type_initialize_kernel(Branch& kernel)
{
    IMPLICIT_TYPES = create_branch(kernel, "#implicit_types").owningTerm;
}

Term* create_implicit_tuple_type(RefList const& types)
{
    std::stringstream typeName;
    typeName << "Tuple<";
    for (int i=0; i < types.length(); i++) {
        if (i != 0) typeName << ",";
        typeName << unbox_type(types[i])->name;
    }
    typeName << ">";

    Term* result = create_type(IMPLICIT_TYPES->nestedContents, typeName.str());
    list_t::setup_type(unbox_type(result));
    Branch& prototype = unbox_type(result)->prototype;
    unbox_type(result)->parent = unbox_type(LIST_TYPE);

    for (int i=0; i < types.length(); i++) {
        ca_assert(is_type(types[i]));
        create_value(prototype, types[i]);
    }
    
    return result;
}

Term* find_member_function(Type* type, std::string const& name)
{
    if (type->memberFunctions.contains(name))
        return type->memberFunctions[name];

    if (type->parent != NULL)
        return find_member_function(type->parent, name);

    return NULL;
}

Term* parse_type(Branch& branch, std::string const& decl)
{
    return parser::compile(branch, parser::type_decl, decl);
}

} // namespace circa
