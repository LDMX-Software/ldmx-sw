
#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <cstring>

namespace eventbuilder {

class BinaryReader {
public:
    BinaryReader(const std::vector<char>& buffer)
        : m_buffer(buffer), m_pos(0) {}

    template <typename T>
    void read(T& value) {
        if (m_pos + sizeof(T) > m_buffer.size()) {
            throw std::runtime_error("Buffer underrun during deserialization");
        }
        memcpy(&value, m_buffer.data() + m_pos, sizeof(T));
        m_pos += sizeof(T);
    }

    template <typename T>
    void read(std::vector<T>& vec, size_t count) {
        vec.resize(count);
        for (size_t i = 0; i < count; ++i) {
            read(vec[i]);
        }
    }

    size_t get_position() const { return m_pos; }
    size_t get_size() const { return m_buffer.size(); }

private:
    const std::vector<char>& m_buffer;
    size_t m_pos;
};

} // namespace eventbuilder


