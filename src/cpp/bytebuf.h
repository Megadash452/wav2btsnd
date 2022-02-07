#ifndef WAV2BTSND_BYTEBUF_H
#define WAV2BTSND_BYTEBUF_H

// The idea for the bytebuf class was taken from https://github.com/Otsoko/bytebuffer

typedef unsigned char byte;

enum endian {
    little, big
};

class bytebuf
{
    //! @brief the endianness of this computer
    endian machine_endianness;
    //! @brief mode that the buffer will read and write shorts and ints (big or little endian)
    //! Default is the same as machine_endianness
    endian endian_mode;
public:
    explicit bytebuf(unsigned int size);
    ~bytebuf();

    //! @brief c-style array of this bytebuffer
    byte* buf;
    const unsigned int size;
    //! @brief Returns the mode that the buffer will read and write shorts and ints
    //! @return Big or Little endian
    [[maybe_unused]] [[nodiscard]] endian endianness() const;
    //! @brief Set the mode that the buffer will read and write shorts and ints
    //! @param mode Big or Little endian
    [[maybe_unused]] void endianness(endian mode);
    //! @brief the position of the "cursor" of this buffer
    unsigned int pos;

    //! @brief get the byte at the current position
    [[maybe_unused]] [[nodiscard]] byte  get_byte() const;
    [[maybe_unused]] [[nodiscard]] short get_short() const;
    [[maybe_unused]] [[nodiscard]] int   get_int() const;
    [[maybe_unused]] [[nodiscard]] byte  get_byte(unsigned int index) const;
    [[maybe_unused]] [[nodiscard]] short get_short(unsigned int index) const;
    [[maybe_unused]] [[nodiscard]] int   get_int(unsigned int index) const;

    /// <summary> Copy a byte into the buffer </summary>
    [[maybe_unused]] void put      (byte  val);
    /// <summary> Copy an array of specific size into the buffer </summary>
    [[maybe_unused]] void put_arr  (byte* arr, unsigned int size);
    /// <summary> Copy the 2 bytes of a short into the buffer (affected by endianness) </summary>
    [[maybe_unused]] void put_short(short val);
    /// <summary> Copy the 4 bytes of an int into the buffer (affected by endianness) </summary>
    [[maybe_unused]] void put_int  (int   val);
    /// <summary> Copy a byte into the buffer at a specific index </summary>
    [[maybe_unused]] void put      (byte  val, unsigned int index);
    /// <summary> Copy an array of specific size into the buffer at a specific index </summary>
    [[maybe_unused]] void put_arr  (byte* arr, unsigned int size, unsigned int index);
    /// <summary> Copy the 2 bytes of a short into the buffer at a specific index (affected by endianness) </summary>
    [[maybe_unused]] void put_short(short val, unsigned int index);
    /// <summary> Copy the 4 bytes of an int into the buffer at a specific index (affected by endianness) </summary>
    [[maybe_unused]] void put_int  (int   val, unsigned int index);

    static endian get_machine_endianness();
};


/// <summary> Swap the order of the bytes of a 4-byte integer </summary>
/// <param name="num"> - unsigned 4-byte int </param>
[[maybe_unused]] unsigned short swap_endianness(const unsigned short& num);
/// <summary> Swap the endianness of the number only if machine uses Big-Endianness </summary>
/// <param name="num"> - unsigned 4-byte int </param>
[[maybe_unused]] unsigned short to_little_endian(const unsigned short& num);
/// <summary> Swap the endianness of the number only if machine uses Little-Endianness </summary>
/// <param name="num"> - unsigned 4-byte int </param>
[[maybe_unused]] unsigned short to_big_endian(const unsigned short& num);
/// <summary> Swap the order of the bytes of a 4-byte integer </summary>
/// <param name="num"> - unsigned 4-byte int </param>
[[maybe_unused]] unsigned int swap_endianness(const unsigned int& num);
/// <summary> Swap the endianness of the number only if machine uses Big-Endianness </summary>
/// <param name="num"> - unsigned 4-byte int </param>
[[maybe_unused]] unsigned int to_little_endian(const unsigned int& num);
/// <summary> Swap the endianness of the number only if machine uses Little-Endianness </summary>
/// <param name="num"> - unsigned 4-byte int </param>
[[maybe_unused]] unsigned int to_big_endian(const unsigned int& num);


#endif //WAV2BTSND_BYTEBUF_H
