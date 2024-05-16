#ifndef IO_BINARY_H
#define IO_BINARY_H

// DHL
#include <iostream>
#include <cstdint>
#include <limits>
#include <boost/predef.h>

namespace IO_Binary {

inline void
I_Binary_write_uinteger32(std::ostream& out, std::uint32_t u) {
    out.write( (char*)(&u), 4);
}
// Special function to write size_t in 32b integer to ensure files
// written by 64b systems are still readable by 32b ones
inline void
I_Binary_write_size_t_into_uinteger32 (std::ostream& out, std::size_t s) {
//    CGAL_assertion_msg
//      (s <= static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()),
//       "Trying to write size_t that does not fit in uint32_t");
    I_Binary_write_uinteger32 (out, static_cast<std::uint32_t>(s));
}
inline void
I_Binary_write_integer32(std::ostream& out, std::int32_t i) {
    out.write( (char*)(&i), 4);
}
inline void
I_Binary_write_float32(std::ostream& out, float f) {
  out.write( (char*)(&f), 4);
}
inline void
I_Binary_write_bool(std::ostream& out, bool b) {
    char c = (b ? 1 : 0);
    out.write(&c, 1);
}

// Special function to read size_t from 32b integer to ensure files
inline void
I_Binary_read_uinteger32(std::istream& is, std::uint32_t& u) {
    is.read( (char*)(&u), 4);
}
// written by 64b systems are still readable by 32b ones
inline void
I_Binary_read_size_t_from_uinteger32(std::istream& is, std::size_t& s) {
    std::uint32_t s32;
    I_Binary_read_uinteger32 (is, s32);
    s = static_cast<std::size_t>(s32);
}
inline void
I_Binary_read_integer32(std::istream& is, std::int32_t& i) {
    is.read( (char*)(&i), 4);
}
inline void
I_Binary_read_float32(std::istream& is, float& f) {
  is.read( (char*)(&f), 4);
}
inline void
I_Binary_read_bool(std::istream& is, bool& b) {
    char c;
    is.read(&c, 1);
    b = (c != 0);
}

inline void
I_swap_to_big_endian( std::uint32_t& u) {
    (void) u;
#ifdef BOOST_ENDIAN_LITTLE_BYTE
  u = ((u >> 24) | (u << 24) | ((u >> 8) & 0xff00) | ((u << 8) & 0xff0000));
#endif
} // IO_Binary

inline void
I_swap_to_big_endian( std::int32_t& i) {
    // We need to use a union instead of the 2 lines below,
    // otherwise we get aliasing issues.
    // std::uint32_t& u = (std::uint32_t&)i;
    // I_swap_to_big_endian( u);
    union {
      std::int32_t  in;
      std::uint32_t ui;
    } u;
    u.in = i;
    I_swap_to_big_endian(u.ui);
    i = u.in;
}

inline void
I_swap_to_big_endian( float& f) {
    // We need to use a union instead of the 2 lines below,
    // otherwise we get aliasing issues.
    // std::uint32_t& u = (std::uint32_t&)f;
    // I_swap_to_big_endian( u);
    union {
      std::uint32_t ui;
      float           fl;
    } u;
    u.fl = f;
    I_swap_to_big_endian(u.ui);
    f = u.fl;
}

inline void
I_Binary_write_big_endian_integer32(std::ostream& out, std::int32_t i) {
    I_swap_to_big_endian( i);
    out.write( (char*)(&i), 4);
}
inline void
I_Binary_write_big_endian_float32(std::ostream& out, float f) {
  I_swap_to_big_endian( f);
  out.write( (char*)(&f), 4);
}

inline void
I_Binary_read_big_endian_integer32(std::istream& is, std::int32_t& i) {
    is.read( (char*)(&i), 4);
    I_swap_to_big_endian( i);
}
inline void
I_Binary_read_big_endian_float32(std::istream& is, float& f) {
  is.read( (char*)(&f), 4);
  I_swap_to_big_endian( f);
}

} //namespace IO_Binary

#endif // IO_BINARY_H
