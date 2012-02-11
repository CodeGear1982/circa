// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct TermMap
{
    std::map<Term*,Term*> _map;

    typedef std::map<Term*,Term*>::iterator iterator;

    explicit TermMap() { }
    Term* operator[](Term* key) const {
        std::map<Term*,Term*>::const_iterator it = _map.find(key);
        if (it == _map.end())
            return NULL;
        else
            return it->second;
    }

    Term*& operator[](Term* key) {
        return _map[key];
    }

    bool contains(Term* key) const {
        return _map.find(key) != _map.end();
    }

    // If 'key' exists in this map, return the associated term.
    // Otherwise, return 'key' back to them.
    Term* getRemapped(Term* key) const {
        if (contains(key))
            return operator[](key);
        else
            return key;
    }

    iterator begin() {
        return _map.begin();
    }

    iterator end() {
        return _map.end();
    }
};

} // namespace circa
