#include "null_io.hpp"

#include <ostream>
#include <streambuf>

class NullBuf final : public std::streambuf {
    std::streamsize xsputn(const char_type*, std::streamsize n) override {
        return n;
    }

    int_type overflow(int_type ch) override {
        return traits_type::not_eof(ch);
    }
};

NullBuf null_buf;

std::ostream null_out(&null_buf);
