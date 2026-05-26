#include "vm/vm_heap.h"
#include "vm/vm_value.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>

using namespace Hulk::VM;

namespace {

bool throws_runtime_error(auto&& fn) {
    try {
        fn();
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

void number_round_trip_preserves_value() {
    const Word value = make_number(123.25);
    assert(is_number(value));
    assert(!is_bool(value));
    assert(!is_nil(value));
    assert(as_number(value) == 123.25);
}

void tagged_nan_pattern_is_canonicalized_as_number() {
    constexpr Word tagged_nan_payload = 0x7ffc000000000000ULL;
    double colliding_nan = 0.0;
    std::memcpy(&colliding_nan, &tagged_nan_payload, sizeof(colliding_nan));

    const Word value = make_number(colliding_nan);
    assert(is_number(value));
    assert(std::isnan(as_number(value)));
}

void bool_and_nil_tags_are_distinct() {
    const Word nil = make_nil();
    const Word truth = make_bool(true);
    const Word falsity = make_bool(false);

    assert(is_nil(nil));
    assert(is_bool(truth));
    assert(is_bool(falsity));
    assert(as_bool(truth));
    assert(!as_bool(falsity));
    assert(throws_runtime_error([&] { (void)as_number(truth); }));
    assert(throws_runtime_error([&] { (void)as_bool(nil); }));
}

void string_and_object_handles_keep_index_and_generation() {
    const Word string_ref = make_string_ref(17, 9);
    const Word object_ref = make_object_ref(23, 11);

    assert(is_string(string_ref));
    assert(is_object(object_ref));
    assert(as_string_index(string_ref) == 17);
    assert(as_string_generation(string_ref) == 9);
    assert(as_object_index(object_ref) == 23);
    assert(as_object_generation(object_ref) == 11);
    assert(throws_runtime_error([&] { (void)as_object_index(string_ref); }));
    assert(throws_runtime_error([&] { (void)as_string_index(object_ref); }));
}

void handle_payload_range_is_checked() {
    assert(throws_runtime_error([&] { (void)make_string_ref(1U << 24, 0); }));
    assert(throws_runtime_error([&] { (void)make_object_ref(0, 1U << 16); }));
}

void to_string_uses_heap_for_dynamic_strings() {
    VMHeap heap;
    const Word value = heap.allocate_string("hola");
    assert(to_string(value, heap) == "hola");
    assert(to_string(make_bool(true), heap) == "true");
    assert(to_string(make_nil(), heap) == "nil");
    assert(to_string(make_number(7.0), heap) == "7");
}

}

int main() {
    number_round_trip_preserves_value();
    tagged_nan_pattern_is_canonicalized_as_number();
    bool_and_nil_tags_are_distinct();
    string_and_object_handles_keep_index_and_generation();
    handle_payload_range_is_checked();
    to_string_uses_heap_for_dynamic_strings();
    return 0;
}
