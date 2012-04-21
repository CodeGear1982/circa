
Circa's object model uses a pluggable type system and tagged pointers. A type is defined
by creating a Type object, and adding functions to its slots to control its behavior. The
type system is meant to be pluggable and flat; inheritance and complex type hierarchies
are discouraged. (Though there is some support for metaclasses)

Values are held by TaggedValue objects, which look like this:

    struct TaggedValue {
        Type* value_type;
        void* value_data;
    };


Each TaggedValue has a pointer to its original type, and a 32-bit slot to hold the actual
data. The type can use `value_data` to hold the actual value, or it can store a pointer
to an allocated object. Builtin types such as int, float, and bool will store their entire
payload in `value_data`.

The object system has enough hooks to support a variety of memory-management strategies,
including reference counting or memory pools. Each type can have its own strategy. Circa
doesn't have a global garbage collector.

# Type slots #

Each Type object has the following slots for controlling behavior. Here's a summary on
how each is used:

## initialize ##

This is called immediately after a TaggedValue is changed to the given type. `value_data`
is always initialized to 0 before initialize(). This function is optional.

## release ##

This is called immediately before a TaggedValue is destroyed or changed to hold a different
type.

## copy ##

Called when a value of this type should be copied to another TaggedValue. This copy should
behave like a separate value, not a reference to the same value. If one value is later
changed, the copy should not be changed. But, the type may reuse the same piece of data if
it can guarantee that there are no visible side effects.

If this function isn't implemented, the default behavior is to do a shallow copy of
`value_data`

## reset ##

Reset an existing value to its 'initial' value. This should have the same effect as
'initialize'.

If this function isn't implemented, the default behavior is to call release() and then
initialize().

## equals ##

Returns whether the given values are equal. The right-hand-side value might be of a
different type, so this function should check the types.

If this function isn't implemented, the default behavior is: when the values have the same
type, do a shallow comparison of `value_data`. If they have different types, return false.

## cast ##

Attempt to convert a value of this type to a value of a different type. The type can
assume that this will only be called if valueFitsType or staticTypeQuery have indicated
that this cast is possible.

If this function isn't implemented, the default behavior is to do a copy()

## isSubtype ##

Returns whether every value of `otherType` can be cast to the current type. This is used
for static and dynamic type checking.

## staticTypeQuery ##

This checks if the given term can be cast to a value of this type. This function may
return a result of "always", "never", or "unable to statically determine". If the answer
can't be statically determined, we'll call valueFitsType at runtime.  This is only used
for static checking, so this function has access to the Term object (isSubtype doesn't
get access to that). This is optional.

## valueFitsType ##

Returns whether the given value fits this type. This is used for dynamic type checking.
Unlike isSubtype, this function gets access to the actual value that is about to be cast.
So unlike `isSubtype`, this function can check against the value. This is optional.

## toString ##

Return a human-readable string describing this value. This is mostly used for debug
output.

## formatSource ##

Returns the value as a snippet of Circa source code. This is only relevant for values
which can be written as a literal value.

## touch ##

This is called when the contents of this value are about to be modified. Types which use
copy-on-write data sharing should create a real copy of their data here.

## getIndex ##

Access a value by index. This is used for lists or list-like objects.

## setIndex ##

Set a value by index.

## getField ##

Access a value by name. This is used for dictionary-like objects.

## setField ##

Set a value by name.

## numElements ##

Return the number of elements in a list-like object.

## remapPointers ##

Deprecated.
This function is called when a Branch is being duplicated, and each local Term reference
needs to be remapped from the old branch to the new copy. If the type holds any Term
reference, then they should remmap them (according to the provided map).

