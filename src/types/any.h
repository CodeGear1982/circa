// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace any_t {

    std::string to_string(TaggedValue*);
    bool matches_type(Type* type, Type* otherType);
    void cast(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly);

} // namespace any_t
} // namespace circa
