#include <cstdint>
#include <algorithm>
#include "bytebuf.h"

bytebuf::bytebuf(unsigned int size)
    : buf(new byte[size]), size(size),
    machine_endianness(get_machine_endianness()),
    endian_mode(machine_endianness), pos(0)
{  }

bytebuf::~bytebuf()
{
    delete[] buf;
}

[[maybe_unused]] endian bytebuf::endianness() const {
    return endian_mode;
}

void bytebuf::endianness(endian mode)
{
    // check for numbers that are outside the range of enum endian
    if (mode >= 0 && mode <= 1)
        endian_mode = mode;
}

byte bytebuf::get_byte() const
{
    return buf[pos];
}

short bytebuf::get_short() const
{
    return this->get_short(pos);
}

int bytebuf::get_int() const
{
    return this->get_int(pos);
}

byte bytebuf::get_byte(unsigned int index) const
{
    return buf[index];
}

short bytebuf::get_short(unsigned int index) const
{
    // check if machine endianness matches this buffer's endian mode
    if (endian_mode == machine_endianness) // little endian
        // read the memory as short (read 2 bytes)
        // return in the same endian order
        return *(short*)(buf + index);
    else // big endian
        // change the endian order
        return buf[index] << 8 | buf[index + 1];

    // TODO: check if 2 bytes from data can be read (to avoid segmentation fault)
    // TODO: what happens when can only read less than 2 bytes
}

int bytebuf::get_int(unsigned int index) const
{
    // check if machine endianness matches this buffer's endian mode
    if (endian_mode == machine_endianness) // little endian
        // read the memory as int (read 4 bytes)
        // return in the same endian order
        return *(int*)(buf + index);
    else // big endian
        // change the endian order
        return buf[index] << 24 |
        buf[index + 1] << 16 |
        buf[index + 2] << 8 |
        buf[index + 3];

    // TODO: check if 4 bytes from data can be read (to avoid segmentation fault)
    // TODO: what happens when can only read less than 4 bytes
}

void bytebuf::put(byte val)
{
    buf[pos] = val;
    pos++;
}

void bytebuf::put_arr(byte* arr, unsigned int size)
{
    std::copy(arr, arr + size, buf + pos);
    pos += size;
}

void bytebuf::put_short(short val)
{
    this->put_short(val, pos);
    pos += 2;
}

void bytebuf::put_int(int val)
{
    this->put_int(val, pos);
    pos += 4;
}

void bytebuf::put(byte val, unsigned int index)
{
    buf[index] = val;
}

void bytebuf::put_arr(byte* arr, unsigned int size, unsigned int index)
{
    std::copy(arr, arr + size, buf + index);
}

void bytebuf::put_short(short val, unsigned int index)
{
    // check if endian mode matches this computer's endianness
    if (endian_mode == machine_endianness) // little endian
    {
        // copy the short the same as it is in memory
        buf[index] = *(byte*)&val;
        buf[index + 1] = val >> 8;
    }
    else // big endian
    {
        // copy the short in the opposite order
        buf[index] = val >> 8;
        buf[index + 1] = *(byte*)&val;
    }
}

void bytebuf::put_int(int val, unsigned int index)
{
    // check if endian mode matches this computer's endianness
    if (endian_mode == machine_endianness) // little endian
    {
        // copy the short the same as it is in memory
        buf[index] = val;
        buf[index + 1] = val >> 8;
        buf[index + 2] = val >> 16;
        buf[index + 3] = val >> 24;
    }
    else // big endian
    {
        // copy the short in the opposite order
        buf[index] = val >> 24;
        buf[index + 1] = val >> 16;
        buf[index + 2] = val >> 8;
        buf[index + 3] = val;
    }
}

endian bytebuf::get_machine_endianness()
{
    union {
        uint32_t i;
        char c[4];
    } b = { 0x01020304 };

    return (endian)(b.c[0] == 1);
}




unsigned short swap_endianness(const unsigned short& num)
{
    byte b[2];
    b[0] = byte(num >> uint16_t(8));
    b[1] = byte(num >> uint16_t(0));

    return *(uint32_t*)b;
}

unsigned short to_little_endian(const unsigned short& num)
{
    if (bytebuf::get_machine_endianness())
        return swap_endianness(num);
    else
        return num;
}

unsigned short to_big_endian(const unsigned short& num)
{
    if (bytebuf::get_machine_endianness())
        return num;
    else
        return swap_endianness(num);
}


unsigned int swap_endianness(const unsigned int& num)
{
    byte b[4];
    b[0] = byte(num >> 24u);
    b[1] = byte(num >> 16u);
    b[2] = byte(num >> 8u);
    b[3] = byte(num >> 0u);

    return *(uint32_t*)b;
}

unsigned int to_little_endian(const unsigned int& num)
{
    if (bytebuf::get_machine_endianness())
        return swap_endianness(num);
    else
        return num;
}

unsigned int to_big_endian(const unsigned int& num)
{
    if (bytebuf::get_machine_endianness())
        return num;
    else
        return swap_endianness(num);
}