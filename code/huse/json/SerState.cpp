// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "SerState.hpp"
#include "Limits.hpp"
#include "../Exception.hpp"

#include <pojobuf/json/writer.hpp>
#include <pojobuf/ostream_string_sink.hpp>

#include <cmath>
#include <exception>
#include <cassert>
#include <ostream>

namespace huse::json {

struct SerState::Impl {
    using WriterType = pojobuf::json::writer<
        pojobuf::ostream_string_sink<>,
        pojobuf::json::count_only_stack,
        pojobuf::invalid_value_strategy::skip
    >;
    WriterType writer;
};
static_assert(alignof(SerState::Impl) <= alignof(std::max_align_t));
static_assert(sizeof(SerState::Impl) <= SerState::ImplBuf_Size);

namespace {

struct JsonRedirectStreambuf final : public std::streambuf
{
    SerState::Impl::WriterType& writer;
    JsonRedirectStreambuf(SerState::Impl& impl) : writer(impl.writer) {}

    int_type overflow(int_type ch) override {
        const auto c = char(ch);
        auto e = pojobuf::json::util::escape_utf8_byte(c);
        if (e.data()) {
            writer.sink.add(e);
        }
        else {
            writer.sink.add(c);
        }
        return ch;
    }

    std::streamsize xsputn(const char_type* s, std::streamsize num) override {
        writer.write_escaped_utf8_string(std::string_view(s, num));
        return num;
    }

    [[noreturn]] void throwSeekException() {
        throw SerException("Seek is not supported by JSON string streams");
    }

    [[noreturn]] pos_type seekpos(pos_type, std::ios_base::openmode) override {
        throwSeekException();
    }

    [[noreturn]] pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override {
        throwSeekException();
    }
};

} // namespace

SerState::SerState(std::ostream& out, bool pretty)
    : m_sink(out)
    , m_impl(new (m_implBuf) Impl{.writer{m_sink, pretty}})
{}

SerState::~SerState() {
    if (std::uncaught_exceptions()) return; // nothing smart to do
    assert(curStackDepth() == 0);
}

void SerState::writeRawJsonValue(std::string_view json) {
    m_impl->writer.add_raw_json_value(json);
}

void SerState::writeValue(bool val) {
    if (val) {
        m_impl->writer.add_literal_element<pojobuf::value_tag::true_>();
    }
    else {
        m_impl->writer.add_literal_element<pojobuf::value_tag::false_>();
    }
}

void SerState::writeValue(std::nullptr_t) {
    m_impl->writer.add_literal_element<pojobuf::value_tag::null>();
}

void SerState::discardTop() noexcept {
    m_impl->writer.discard_pending_key();
}

template <typename T>
void SerState::writeNumber(T num) {
    if (!m_impl->writer.add_number_element(num)) {
        throwException("Invalid JSON number value");
    }
}

void SerState::writeValue(short val) { writeNumber(val); }
void SerState::writeValue(unsigned short val) { writeNumber(val); }
void SerState::writeValue(int val) { writeNumber(val); }
void SerState::writeValue(unsigned int val) { writeNumber(val); }
void SerState::writeValue(long val) { writeNumber(val); }
void SerState::writeValue(unsigned long val) { writeNumber(val); }
void SerState::writeValue(long long val) { writeNumber(val); }
void SerState::writeValue(unsigned long long val) { writeNumber(val); }
void SerState::writeValue(float val) { writeNumber(val); }
void SerState::writeValue(double val) { writeNumber(val); }

void SerState::writeValue(std::string_view val) {
    m_impl->writer.add_string_element(val);
}

void SerState::topObjectPushKey(std::string_view k) {
    m_impl->writer.add_object_key(k);
}

void SerState::pushObjectValue() {
    m_impl->writer.open_compound_element<pojobuf::value_tag::object>();
}
void SerState::topObjectClose() {
    m_impl->writer.close_compound_element<pojobuf::value_tag::object>();
}
void SerState::pushArrayValue() {
    m_impl->writer.open_compound_element<pojobuf::value_tag::array>();
}
void SerState::topArrayClose() {
    m_impl->writer.close_compound_element<pojobuf::value_tag::array>();
}

struct SerState::JsonOStream {
    JsonOStream(SerState::Impl& impl)
        : streambuf(impl)
        , stream(&streambuf)
    {}

    JsonRedirectStreambuf streambuf;
    std::ostream stream;
};

std::ostream& SerState::pushStringStream() {
    if (!m_stringStream) {
        m_stringStream = std::make_unique<std::optional<JsonOStream>>();
    }
    assert(!*m_stringStream);
    m_stringStream->emplace(*m_impl);
    m_impl->writer.prepare_for_val();
    m_sink.add('"');
    return (*m_stringStream)->stream;
}

void SerState::topStringStreamClose() {
    assert(!!m_stringStream && !!*m_stringStream);
    m_stringStream->reset();
    m_sink.add('"');
}

uint32_t SerState::curStackDepth() const noexcept {
    return m_impl->writer.cur_depth();
}

void SerState::setRenderCompact() noexcept {
    m_impl->writer.set_render_compact();
}

bool SerState::topIsCompact() const noexcept {
    return m_impl->writer.cur_depth_is_compact();
}

} // namespace huse::json
