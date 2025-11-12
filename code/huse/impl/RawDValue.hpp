// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../Type.hpp"
#include <splat/unreachable.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <algorithm>

// huse config
#define SAJSON_NO_STD_STRING
//

namespace huse::json::sajson {

/**
 * Indicates a JSON value's type.
 *
 * In early versions of sajson, this was the tag value directly from the parsed
 * AST storage, but, to preserve API compabitility, it is now synthesized.
 */
enum type : uint8_t {
    TYPE_INTEGER,
    TYPE_DOUBLE,
    TYPE_NULL,
    TYPE_FALSE,
    TYPE_TRUE,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT,
};

/// A simple type encoding a pointer to some memory and a length (in bytes).
/// Does not maintain any memory.
class string {
public:
    string(const char* text_, size_t length)
        : text(text_)
        , _length(length) {
    }

    const char* data() const { return text; }

    size_t length() const { return _length; }

#ifndef SAJSON_NO_STD_STRING
    std::string as_string() const { return std::string(text, text + _length); }
#endif

private:
    const char* const text;
    const size_t _length;

    string(); /*=delete*/
};

namespace internal {

/**
 * get_value_of_key for objects is O(lg N), but most objects have
 * small, bounded key sets, and the sort adds parsing overhead when a
 * linear scan would be fast anyway and the code consuming objects may
 * never lookup values by name! Therefore, only binary search for
 * large numbers of keys.
 */
constexpr inline bool should_binary_search(size_t length) {
#ifdef SAJSON_UNSORTED_OBJECT_KEYS
    return false;
#else
    return length > 100;
#endif
}

/**
 * The low bits of every AST word indicate the value's type. This representation
 * is internal and subject to change.
 */
enum class tag : uint8_t {
    integer,
    double_,
    null,
    false_,
    true_,
    string,
    array,
    object,
};

static const size_t TAG_BITS = 3;
static const size_t TAG_MASK = (1 << TAG_BITS) - 1;
static const size_t VALUE_MASK = ~size_t{} >> TAG_BITS;

static const size_t ROOT_MARKER = VALUE_MASK;

constexpr inline tag get_element_tag(size_t s) {
    return static_cast<tag>(s & TAG_MASK);
}

constexpr inline size_t get_element_value(size_t s) { return s >> TAG_BITS; }

struct object_key_record {
    size_t key_start;
    size_t key_end;
    size_t value;

    bool match(const char* object_data, const string& str) const {
        size_t length = key_end - key_start;
        return length == str.length()
            && 0 == memcmp(str.data(), object_data + key_start, length);
    }
};

struct object_key_comparator {
    object_key_comparator(const char* object_data)
        : data(object_data) {
    }

    bool operator()(const object_key_record& lhs, const string& rhs) const {
        const size_t lhs_length = lhs.key_end - lhs.key_start;
        const size_t rhs_length = rhs.length();
        if (lhs_length < rhs_length) {
            return true;
        }
        else if (lhs_length > rhs_length) {
            return false;
        }
        return memcmp(data + lhs.key_start, rhs.data(), lhs_length) < 0;
    }

    bool operator()(const string& lhs, const object_key_record& rhs) const {
        return !(*this)(rhs, lhs);
    }

    bool
        operator()(const object_key_record& lhs, const object_key_record& rhs) {
        const size_t lhs_length = lhs.key_end - lhs.key_start;
        const size_t rhs_length = rhs.key_end - rhs.key_start;
        if (lhs_length < rhs_length) {
            return true;
        }
        else if (lhs_length > rhs_length) {
            return false;
        }
        return memcmp(data + lhs.key_start, data + rhs.key_start, lhs_length)
            < 0;
    }

    const char* data;
};

} // namespace internal


namespace integer_storage {
enum { word_length = 1 };

inline int load(const size_t* location) {
    int value;
    memcpy(&value, location, sizeof(value));
    return value;
}

inline void store(size_t* location, int value) {
    // NOTE: Most modern compilers optimize away this constant-size
    // memcpy into a single instruction. If any don't, and treat
    // punning through a union as legal, they can be special-cased.
    static_assert(
        sizeof(value) <= sizeof(*location),
        "size_t must not be smaller than int");
    memcpy(location, &value, sizeof(value));
}
} // namespace integer_storage

namespace double_storage {
enum { word_length = sizeof(double) / sizeof(size_t) };

inline double load(const size_t* location) {
    double value;
    memcpy(&value, location, sizeof(double));
    return value;
}

inline void store(size_t* location, double value) {
    // NOTE: Most modern compilers optimize away this constant-size
    // memcpy into a single instruction. If any don't, and treat
    // punning through a union as legal, they can be special-cased.
    memcpy(location, &value, sizeof(double));
}
} // namespace double_storage

/// Represents a JSON value.  First, call get_type() to check its type,
/// which determines which methods are available.
///
/// Note that \ref value does not maintain any backing memory, only the
/// corresponding \ref document does.  It is illegal to access a \ref value
/// after its \ref document has been destroyed.
class value {
public:
    value()
        : value_tag{ tag::null }
        , payload{ nullptr }
        , text{ nullptr } {
    }

/// Returns the JSON value's \ref type.
    type get_type() const {
        // As of 2020, current versions of MSVC generate a jump table for this
        // conversion. If it matters, a more clever mapping with knowledge of
        // the specific values is possible. gcc and clang generate good code --
        // at worst a table lookup.
        switch (value_tag) {
        case tag::integer:
            return TYPE_INTEGER;
        case tag::double_:
            return TYPE_DOUBLE;
        case tag::null:
            return TYPE_NULL;
        case tag::false_:
            return TYPE_FALSE;
        case tag::true_:
            return TYPE_TRUE;
        case tag::string:
            return TYPE_STRING;
        case tag::array:
            return TYPE_ARRAY;
        case tag::object:
            return TYPE_OBJECT;
        }
        SPLAT_UNREACHABLE();
    }

    bool is_boolean() const {
        return value_tag == tag::false_ || value_tag == tag::true_;
    }

    bool get_boolean_value() const {
        switch (value_tag) {
        case tag::true_:
            return true;
        case tag::false_:
            return false;
        default:
            assert(false);
            return false;
        }
    }

    /// Returns the length of the object or array.
    /// Only legal if get_type() is TYPE_ARRAY or TYPE_OBJECT.
    size_t get_length() const {
        assert_tag_2(tag::array, tag::object);
        return payload[0];
    }

    /// Returns the nth element of an array.  Calling with an out-of-bound
    /// index is undefined behavior.
    /// Only legal if get_type() is TYPE_ARRAY.
    value get_array_element(size_t index) const {
        using namespace internal;
        assert_tag(tag::array);
        size_t element = payload[1 + index];
        return value(
            get_element_tag(element),
            payload + get_element_value(element),
            text);
    }

    /// Returns the nth key of an object.  Calling with an out-of-bound
    /// index is undefined behavior.
    /// Only legal if get_type() is TYPE_OBJECT.
    string get_object_key(size_t index) const {
        assert_tag(tag::object);
        const size_t* s = payload + 1 + index * 3;
        return string(text + s[0], s[1] - s[0]);
    }

    /// Returns the nth value of an object.  Calling with an out-of-bound
    /// index is undefined behavior.  Only legal if get_type() is TYPE_OBJECT.
    value get_object_value(size_t index) const {
        using namespace internal;
        assert_tag(tag::object);
        size_t element = payload[3 + index * 3];
        return value(
            get_element_tag(element),
            payload + get_element_value(element),
            text);
    }

    /// Given a string key, returns the value with that key or a null value
    /// if the key is not found.  Running time is O(lg N).
    /// Only legal if get_type() is TYPE_OBJECT.
    value get_value_of_key(const string& key) const {
        assert_tag(tag::object);
        size_t i = find_object_key(key);
        if (i < get_length()) {
            return get_object_value(i);
        }
        else {
            return value(tag::null, 0, 0);
        }
    }

    /// Given a string key, returns the index of the associated value if
    /// one exists.  Returns get_length() if there is no such key.
    /// Note: sajson sorts object keys, so the running time is O(lg N).
    /// Only legal if get_type() is TYPE_OBJECT
    size_t find_object_key(const string& key) const {
        using namespace internal;
        assert_tag(tag::object);
        size_t length = get_length();
        const object_key_record* start
            = reinterpret_cast<const object_key_record*>(payload + 1);
        const object_key_record* end = start + length;
        if (should_binary_search(length)) {
            const object_key_record* i = std::lower_bound(
                start, end, key, object_key_comparator(text));
            if (i != end && i->match(text, key)) {
                return i - start;
            }
        }
        else {
            for (size_t i = 0; i < length; ++i) {
                if (start[i].match(text, key)) {
                    return i;
                }
            }
        }
        return length;
    }

    /// If a numeric value was parsed as a 32-bit integer, returns it.
    /// Only legal if get_type() is TYPE_INTEGER.
    int get_integer_value() const {
        assert_tag(tag::integer);
        return integer_storage::load(payload);
    }

    /// If a numeric value was parsed as a double, returns it.
    /// Only legal if get_type() is TYPE_DOUBLE.
    double get_double_value() const {
        assert_tag(tag::double_);
        return double_storage::load(payload);
    }

    /// Returns a numeric value as a double-precision float.
    /// Only legal if get_type() is TYPE_INTEGER or TYPE_DOUBLE.
    double get_number_value() const {
        assert_tag_2(tag::integer, tag::double_);
        if (value_tag == tag::integer) {
            return get_integer_value();
        }
        else {
            return get_double_value();
        }
    }

    /// Returns true and writes to the output argument if the numeric value
    /// fits in a 53-bit integer.  This is useful for timestamps and other
    /// situations where integral values with greater than 32-bit precision
    /// are used, as 64-bit values are not understood by all JSON
    /// implementations or languages.
    /// Returns false if the value is not an integer or not in range.
    /// Only legal if get_type() is TYPE_INTEGER or TYPE_DOUBLE.
    bool get_int53_value(int64_t* out) const {
        // Make sure the output variable is always defined to avoid any
        // possible situation like
        // https://gist.github.com/chadaustin/2c249cb850619ddec05b23ca42cf7a18
        *out = 0;

        assert_tag_2(tag::integer, tag::double_);
        switch (value_tag) {
        case tag::integer:
            *out = get_integer_value();
            return true;
        case tag::double_: {
            double v = get_double_value();
            if (v < -(1LL << 53) || v >(1LL << 53)) {
                return false;
            }
            int64_t as_int = static_cast<int64_t>(v);
            if (as_int != v) {
                return false;
            }
            *out = as_int;
            return true;
        }
        default:
            return false;
        }
    }

    /// Returns the length of the string.
    /// Only legal if get_type() is TYPE_STRING.
    size_t get_string_length() const {
        assert_tag(tag::string);
        return payload[1] - payload[0];
    }

    /// Returns a pointer to the beginning of a string value's data.
    /// WARNING: Calling this function and using the return value as a
    /// C-style string (that is, without also using get_string_length())
    /// will cause the string to appear truncated if the string has
    /// embedded NULs.
    /// Only legal if get_type() is TYPE_STRING.
    const char* as_cstring() const {
        assert_tag(tag::string);
        return text + payload[0];
    }

#ifndef SAJSON_NO_STD_STRING
    /// Returns a string's value as a std::string.
    /// Only legal if get_type() is TYPE_STRING.
    std::string as_string() const {
        assert_tag(tag::string);
        return std::string(text + payload[0], text + payload[1]);
    }
#endif

    /// \cond INTERNAL
    const size_t* _internal_get_payload() const { return payload; }
    /// \endcond

private:
    using tag = internal::tag;

    explicit value(tag value_tag_, const size_t* payload_, const char* text_)
        : value_tag(value_tag_)
        , payload(payload_)
        , text(text_) {
    }

    inline void assert_tag([[maybe_unused]] tag expected) const { assert(expected == value_tag); }

    inline void assert_tag_2([[maybe_unused]] tag e1, [[maybe_unused]] tag e2) const {
        assert(e1 == value_tag || e2 == value_tag);
    }

    tag value_tag;
    const size_t* payload;
    const char* text;

    friend class document;
};

} // namespace huse::json::sajson

namespace huse::impl {

using RawDValue = huse::json::sajson::value;

} // namespace huse::impl
