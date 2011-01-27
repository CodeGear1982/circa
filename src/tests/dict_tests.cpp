// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "testing.h"
#include "types/dict.h"

namespace circa {
namespace dict_tests {

void test_simple()
{
    TaggedValue five;
    set_int(&five, 5);
    TaggedValue ten;
    set_int(&ten, 10);

    dict_t::DictData* data = dict_t::create_dict();
    dict_t::insert_value(&data, "a", &five);
    dict_t::insert_value(&data, "b", &ten);

    test_assert(dict_t::get_value(data, "a")->asInt() == 5);
    test_assert(dict_t::get_value(data, "b")->asInt() == 10);
    test_equals(dict_t::to_string(data), "[a: 5, b: 10]");
    dict_t::free_dict(data);
}

void test_insert()
{
    Dict dict;
    TaggedValue v;
    set_int(&v, 5);

    TaggedValue* a_inserted = dict.insert("a");
    test_equals(dict.toString(), "[a: null]");

    set_int(a_inserted, 7);
    test_equals(dict.toString(), "[a: 7]");
}

void dont_insert_same_key_multiple_times()
{
    dict_t::DictData* data = dict_t::create_dict();

    TaggedValue val;
    set_int(&val, 5);
    TaggedValue val2;
    set_string(&val2, "a");

    dict_t::insert_value(&data, "key", &val);
    dict_t::insert_value(&data, "key", &val2);

    test_assert(dict_t::get_value(data, "key")->asString() == "a");
    test_assert(dict_t::count(data) == 1);
    dict_t::free_dict(data);
}


void handle_missing_keys()
{
    dict_t::DictData* data = dict_t::create_dict();

    test_assert(dict_t::find_key(data, "a") == -1);
    test_assert(dict_t::get_value(data, "xyz") == NULL);

    dict_t::free_dict(data);
}

void hash_collision()
{
    // Start with two strings which we know will cause a hash collision
    const char* a = "headbutt";
    const char* b = "butthead";

    // Make sure they actually collide
    dict_t::DictData* data1 = dict_t::create_dict();
    dict_t::DictData* data2 = dict_t::create_dict();

    int index_a1 = dict_t::insert(&data1, a);
    int index_b2 = dict_t::insert(&data2, b);

    // This collision isn't a requirement for the dict type, but it is required
    // in order for this test to work properly. If this assert fails, then
    // change the strings above to two strings that do collide.
    test_assert(index_a1 == index_b2);

    // Now insert an item that really has a collision
    int index_b1 = dict_t::insert(&data1, b);
    test_assert(index_b1 != index_a1);

    TaggedValue x, y;
    set_string(&x, "x");
    set_string(&y, "y");

    dict_t::insert_value(&data1, a, &x);
    dict_t::insert_value(&data1, b, &y);

    test_assert(dict_t::get_value(data1, a)->asString() == "x");
    test_assert(dict_t::get_value(data1, b)->asString() == "y");

    // Remove the first key, make sure the second key moves up
    dict_t::remove(data1, a);
    test_assert(dict_t::get_value(data1, b)->asString() == "y");
    test_assert(dict_t::find_key(data1, b) == index_a1);

    dict_t::free_dict(data1);
    dict_t::free_dict(data2);
}

void many_items()
{
    const int count = 100;

    dict_t::DictData* data = dict_t::create_dict();

    // Insert lots of items
    for (int i=0; i < count; i++) {
        char key[10]; sprintf(key, "%d", i);
        TaggedValue val; set_int(&val, i);
        dict_t::insert_value(&data, key, &val);
    }

    test_assert(dict_t::count(data) == count);

    // Make sure they are really there
    for (int i=0; i < count; i++) {
        char key[10]; sprintf(key, "%d", i);
        test_assert(dict_t::get_value(data, key)->asInt() == i);
    }

    // Delete every odd item
    for (int i=0; i < count; i++) {
        if ((i % 2) == 0)
            continue;
        char key[10]; sprintf(key, "%d", i);
        dict_t::remove(data, key);
    }

    test_assert(dict_t::count(data) == count / 2);

    // Make sure the even items are accessible
    for (int i=0; i < count; i++) {
        char key[10]; sprintf(key, "%d", i);
        if ((i % 2) == 0) {
            // Even numbers should still be there
            test_assert(dict_t::get_value(data, key)->asInt() == i);
        } else {
            // Odd numbers should be gone
            test_assert(dict_t::get_value(data, key) == NULL);
        }
    }
}

void test_duplicate()
{
    TaggedValue eleven = TaggedValue::fromInt(11);
    TaggedValue one_and_change = TaggedValue::fromFloat(1.2);
    TaggedValue t = TaggedValue::fromBool(true);
    TaggedValue hello = TaggedValue::fromString("hello");

    dict_t::DictData* data = dict_t::create_dict();
    dict_t::insert_value(&data, "a", &eleven);
    dict_t::insert_value(&data, "b", &one_and_change);
    dict_t::insert_value(&data, "c", &t);

    test_equals(dict_t::to_string(data), "[a: 11, b: 1.2, c: true]");

    dict_t::DictData* dupe = dict_t::duplicate(data);

    test_equals(dict_t::to_string(dupe), "[a: 11, b: 1.2, c: true]");

    // Modify original, make sure that dupe is unaffected
    dict_t::remove(data, "b");
    dict_t::insert_value(&data, "d", &hello);
    test_equals(dict_t::to_string(data), "[a: 11, c: true, d: 'hello']");
    test_equals(dict_t::to_string(dupe), "[a: 11, b: 1.2, c: true]");

    dict_t::free_dict(data);
    dict_t::free_dict(dupe);
}

void test_reset()
{
    Dict dict;

    TaggedValue a;
    set_int(&a, 4);
    dict.set("a", &a);

    test_equals(dict.toString(), "[a: 4]");
    reset(&dict);
    test_equals(dict.toString(), "[]");
    dict.set("a", &a);
    test_equals(dict.toString(), "[a: 4]");
}

void test_iterate()
{
    Dict dict;
    TaggedValue iterator;

    TaggedValue one;
    set_int(&one, 1);
    TaggedValue two;
    set_int(&two, 2);

    dict.set("one", &one);
    dict.set("two", &two);

    const char* currentKey;
    TaggedValue* currentValue;

    bool foundOne = false;
    bool foundTwo = false;
    for (dict.iteratorStart(&iterator);
            !dict.iteratorFinished(&iterator);
            dict.iteratorNext(&iterator)) {

        dict.iteratorGet(&iterator, &currentKey, &currentValue);

        test_assert(currentValue != NULL);

        if (std::string(currentKey) == "one") {
            test_assert(!foundOne);
            test_equals(currentValue->asInt(), 1);
            foundOne = true;
        } else if (std::string(currentKey) == "two") {
            test_assert(!foundTwo);
            test_equals(currentValue->asInt(), 2);
            foundTwo = true;
        } else {
            test_assert(false);
        }
    }
    test_assert(foundOne);
    test_assert(foundTwo);
}

void register_tests()
{
    REGISTER_TEST_CASE(dict_tests::test_simple);
    REGISTER_TEST_CASE(dict_tests::test_insert);
    REGISTER_TEST_CASE(dict_tests::dont_insert_same_key_multiple_times);
    REGISTER_TEST_CASE(dict_tests::handle_missing_keys);
    REGISTER_TEST_CASE(dict_tests::hash_collision);
    REGISTER_TEST_CASE(dict_tests::many_items);
    REGISTER_TEST_CASE(dict_tests::test_duplicate);
    REGISTER_TEST_CASE(dict_tests::test_reset);
    REGISTER_TEST_CASE(dict_tests::test_iterate);
}

}
}
