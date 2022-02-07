#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <array>
#include "bytebuf.h"

#include <filesystem>


#ifdef _DEBUG
#define dbg_print(x) std::cout << x
#define dbg_println(x) std::cout << x << std::endl
#else
#define dbg_print(x)
#define dbg_println(x)
#endif

#define exit_err(err) std::cerr << err << std::endl; return 1

#define RESET_COL  "\033[0m"
#define RED        "\033[31m"
#define GREEN      "\033[32m"
#define YELLOW     "\033[33m"
#define BLUE       "\033[34m"
#define PURPLE     "\033[35m"
#define CYAN       "\033[36m"


inline int str_to_int(const char* str);
bool str_ends_with(const std::string& str, const std::string& pattern);
/// <summary>
///     Get the name of a file without the path or extension.
///     E.G.: C:/dir/file.txt -> file </summary>
/// <param name="path"> - The path to the file (absolute or relative) </param>
/// <returns> File name without path or extension </returns>
[[maybe_unused]] std::string file_name(const std::string& path);
/// <summary>
///     Get the full name of a file (may include path depending on input) and removes the file extension.
///     E.G.: C:/dir/file.txt -> C:/dir/file </summary>
/// <param name="path"> - The path to the file (absolute or relative) </param>
/// <returns> File path without extension </returns>
std::string file_no_ext(const std::string& path);

// og doesn't explain what this is
short swap(short value);


#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
#pragma ide diagnostic ignored "UnreachableCode"

int main(int argc, const char** argv)
{
    // using commas doesn't work sometimes.
    using std::cout;
    using std::endl;
    using std::string;

    // Program title
    cout << " ------ WAV to btsnd 0.1 alpha ------ \nbtsnd is BIG_ENDIAN wav is LITTLE_ENDIAN\n\n";

    // No arguments entered by user. There wil always be 1 argument: the path of the program
    if (argc <= 1)
    {
        cout <<
            "This program only supports RIFF wav files.\n"
            "Usage:\n"
            YELLOW "wav2btsnd -in <input_file> <optional args>\n" RESET_COL
            "The program will automatically detect the type of input file and choose which type of file to output (E.g.: .wav input will automatically output .btsnd file\n"
            "\n"
            "Optional Args:\n"
            "  " CYAN "-out " YELLOW "<outfile>" RESET_COL ": File path to create the converted file in. Default is name of input file + appropriate file extension in the same directory as the program.\n"
            "\n"
            "  " CYAN "-makeWav" RESET_COL ":       Convert.btsnd to.wav.\n"
            "      Incompatible with: " RED "-makeBtsnd\n" RESET_COL
            "\n"
            "  " CYAN "-makeBtsnd" RESET_COL ":     Convert .wav to .btsnd.\n"
            "      Incompatible with: " RED "-makeWav\n" RESET_COL
            "\n"
            "These args are only for making btsnd's:\n"
            "  " CYAN "-loopPoint " YELLOW "<loop_sample>" RESET_COL ": Specifies a specific sample to loop from there to the end, once play-through of the btsnd has finished once.\n"
            "      Incompatible with: " RED "-noLoop" RESET_COL ", " RED "-makeWav\n" RESET_COL
            "\n"
            "  " CYAN "-noLoop" RESET_COL ":        Makes it where the btsnd doesnt loop its sound.\n"
            "      Incompatible with: " RED "-loopPoint" RESET_COL ", " RED "-makeWav\n" RESET_COL
            "\n"
            "  " CYAN "-gamepadOnly" RESET_COL ":   Makes sound only hearable on gamepad.\n"
            "      Incompatible with: " RED "-tvOnly\n" RESET_COL
            "\n"
            "  " CYAN "-tvOnly" RESET_COL ":        Makes sound only hearable on tv.\n"
            "      Incompatible with: " RED "-gamepadOnly\n" RESET_COL
            "\n";
        // successful exit
        return 0;
    }

    // --- Declarations
    // These 2 bytearrays are what the headers in a wav file should look like (correct headers)
    // header_1 starts at 0th byte
    std::array<byte, 4> wav_header1 = { 0x52, 0x49, 0x46, 0x46 };
    // skips 8 bytes (file size and file type header)
    //header_2 starts at 12th byte
    std::array<byte, 32> wav_header2 = {
        0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x02, 0x00, 0x80, 0xBB, 0x00, 0x00, 0x00, 0xEE,
        0x02, 0x00, 0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61
    };

    // input file specified by user
    string input_path;
    // output file specified by user. Default is name of input file without file extension
    string output_path;

    // tells whether program will convert input to wav or btsnd
    // true = convert .wav to .btsnd // false = convert .btsnd to .wav
    bool make_btsnd = true;
    // makes .btsnd file hearable on either the tv (0), wii u gamepad (1), or both (2)
    int hearable_where = 2;
    // no hint to what range loop_point should be // TODO: figure out the acceptable range
    int loop_point = 0;
    bool silent_loop = false;

    // tells if args -makeWav or -makeBtsnd were used by the user
    bool used_make = false;


    // Go through all the arguments given by user
    for (unsigned short int i = 0; i < (unsigned short int)argc; i++)
    {
        string arg = argv[i];

        if (arg == "-in")
        {
            i++; // skip the next argument
            // set the next argument to the input file path
            input_path = argv[i];
            dbg_println("input_path = " << input_path);

            // auto-detect if converting btsnd to wav or vice versa
            if (str_ends_with(input_path, ".btsnd"))
                // convert .btsnd to wav
                make_btsnd = false;

            if (output_path.empty())
                // set the output file name the same as input file name
                output_path = file_no_ext(input_path);
        }
        else if (arg == "-out")
        {
            i++; // skip the next argument
            // set the next argument to the output file path
            output_path = argv[i];
            dbg_println("output_path = " << output_path);
        }
        else if (arg == "-makeWav")
        {
            // check if makeBtsnd is already an arg
            if (used_make)
            {
                exit_err("Cannot use -makeWav and -makeBtsnd together");
            }

            make_btsnd = false;
            used_make = true;
        }
        else if (arg == "-makeBtsnd")
        {
            // check if makeWav is already an arg
            if (used_make)
            {
                exit_err("Cannot use -makeWav and -makeBtsnd together");
            }

            make_btsnd = true;
            used_make = true;
        }
        else if (arg == "-loopPoint")
        {
            i++; // skip the next argument
            loop_point = str_to_int(argv[i]);
        }
        else if (arg == "-noLoop")
        {
            silent_loop = true;
        }
        else if (arg == "-gamepadOnly")
        {
            hearable_where = 1;
        }
        else if (arg == "-tvOnly")
        {
            hearable_where = 0;
        }
    }


    // checking for errors in arguments
    if (input_path.empty())
    {
        exit_err("Must use argument -in to provide input file");
    }
    if (silent_loop && loop_point > 0)
    {
        exit_err("Can't use -loopPoint and -noLoop together");
    }
    if ((silent_loop || loop_point > 0) && !make_btsnd)
    {
        exit_err("Can't use -loopPoint and -noLoop when the output is .wav");
    }


    // TODO: there are some issues with converting btsnd to wav and vice versa (like the output is larger than its supposed to be [by 12 bytes])


    // --- The core
    // Convert .WAV to .BTSND
    if (make_btsnd)
    {
        // set file extension of output file to .btsnd
        if (!str_ends_with(output_path, ".btsnd"))
            output_path += ".btsnd";

        std::ifstream input_file{ input_path, std::ios::binary | std::ios::in };
        dbg_print("Working directory: " << std::filesystem::current_path().generic_string());
        if (input_file.is_open())
        {
            dbg_println("Opened the file \"" << input_path << "\"");

            // get size of file
            input_file.seekg(0, std::ios::end);
            const unsigned long input_file_size = input_file.tellg();
            input_file.seekg(0, std::ios::beg);

            // load file data to memory
            bytebuf data{ input_file_size };
            input_file.read((char*)data.buf, data.size);
            input_file.close();

            // These will contain the supposed headers of the .wav file
            // They will be compared with the correct headers to see if the file is corrupted
            byte* file_header1 = new byte[wav_header1.size()];
            byte* file_header2 = new byte[wav_header2.size()];

            // put_int header data in the arrays
            std::copy(data.buf, data.buf + wav_header1.size(), file_header1);
            std::copy(data.buf + 8, data.buf + 8 + wav_header2.size(), file_header2);

            // check if the .wav file is corrupted
            // 0 means the content of the memories are the same
            if (std::memcmp(file_header1, wav_header1.data(), wav_header1.size()) != 0 &&
                std::memcmp(file_header2, wav_header2.data(), wav_header2.size()) != 0)
            {
                exit_err("Input .WAV file must be 48000khz (DAT) 16bit stereo");
            }


            bytebuf out_buf = bytebuf{ input_file_size - 44 + 8 }; // og doesn't explain where these numbers come from
            out_buf.endianness(endian::big);

            out_buf.put_int(hearable_where);

            if (silent_loop)
                loop_point = (((int)data.size - 44) / 4 + 1);

            out_buf.put_int(loop_point);

            out_buf.endianness(endian::little);
            for (unsigned long i = 44; i < data.size; i += 2)
                out_buf.put_short(data.get_short(i));


            // write output buffer to file
            std::ofstream output_file{ output_path };
            output_file.write((char*)out_buf.buf, out_buf.size);

            // write this for the btsnd to not loop
            if (silent_loop)
            {
                byte silence_loop[4] = { 0, 0, 0, 0 };
                output_file.write((char*)silence_loop, 4);
            }

            //free memory
            delete[] file_header1;
            delete[] file_header2;
            output_file.close();
        }
        else
        {
            exit_err("Cannot open file \"" << input_path << "\". Check if it exists or if the relative path is correct.");
        }
    }
    else // Convert .BTSND to .WAV
    {
        // set file extension of output file to .wav
        if (!str_ends_with(output_path, ".wav"))
            output_path += ".wav";

        std::ifstream input_file{ input_path, std::ios::binary | std::ios::in };
        dbg_print("Working directory: " << std::filesystem::current_path().generic_string());
        if (input_file.is_open())
        {
            dbg_println("Opened the file \"" << input_path << "\"");

            // get size of file
            input_file.seekg(0, std::ios::end);
            const unsigned long input_file_size = input_file.tellg();
            input_file.seekg(0, std::ios::beg);

            // load file data to memory
            bytebuf data{ input_file_size };
            input_file.read((char*)data.buf, data.size);
            input_file.close();

            bytebuf out_buf{ input_file_size - 8 + 44 };
            out_buf.endianness(endian::little);
            out_buf.put_arr(wav_header1.data(), wav_header1.size());
            out_buf.put_int((int)data.size - 8 + 0x2C); // 4 bytes
            out_buf.put_arr(wav_header2.data(), wav_header2.size());
            out_buf.put_int((int)input_file_size - 8);

            data.endianness(endian::big);
            for (int i = 8; i < data.size; i += 2)
                out_buf.put_short(swap(data.get_short(i)));

            // write output buffer to file
            std::ofstream output_file{ output_path };
            output_file.write((char*)out_buf.buf, out_buf.size);

            // free memory
            output_file.close();
        }
        else
        {
            exit_err("Cannot open file \"" << input_path << "\". Check if it exists or if the relative path is correct.");
        }
    }

    cout << "Saved to " << output_path << endl;

    // successful exit
    return 0;
}
#pragma clang diagnostic pop



inline int str_to_int(const char* str)
{
    std::stringstream ss(str);
    int i;

    if (ss >> i)
        return i;

    std::cerr << "Error converting \"" << str << "\" to int\n";
    return 0;
}

bool str_ends_with(const std::string& str, const std::string& pattern)
{
    using std::string;

    // it is impossible to match when the pattern is longer than the original string
    if (str.size() < pattern.size())
        return false;

    // the part of the str to compare with the pattern
    // starts at the offset (size of pattern) backwards from str, ends at end of str
    string ending = str.substr(str.size() - pattern.size(), str.size());
    return ending == pattern;
}

[[maybe_unused]] std::string file_name(const std::string& path)
{
    unsigned int begin = 0;
    unsigned int end = path.size();

    // try to find a / or \ for file name
    for (int i = (int)path.size() - 1; i >= 0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
        {
            begin = i + 1;
            break;
        }
    }

    // find the dot of the file extension
    for (int i = (int)path.size() - 1; i >= 0; i--)
    {
        if (path[i] == '.')
        {
            end = i;
            break;
        }
    }

    return path.substr(begin, end);
}

std::string file_no_ext(const std::string& path)
{
    unsigned int end = path.size();

    // find the dot of the file extension
    for (int i = (int)path.size() - 1; i >= 0; i--)
    {
        if (path[i] == '.')
        {
            end = i;
            break;
        }
    }

    return path.substr(0, end);
}


short swap(short value)
{
    int b1 = value & 0xff;
    int b2 = (value >> 8) & 0xff;

    return (short)(b1 << 8 | b2 << 0);
}
