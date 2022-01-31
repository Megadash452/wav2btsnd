#include <iostream>
#include <sstream>
#include <string>


#ifdef _DEBUG
    #define dbg_print(x) std::cout << x
    #define dbg_println(x) std::cout << x << std::endl
    #define print
#else
    #define dbg_print(x)
    #define dbg_println(x)
#endif

#define exit_err(err) std::cerr << err << std::endl; return 1

#define RESET_COL  "\033[m"
#define YELLOW     "\033[33m"


typedef unsigned char byte;

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
    // starts at the offeset (size of pattern) backwards from str, ends at end of str
    string ending = str.substr(str.size() - pattern.size(), str.size());
    return ending == pattern;
}


int main(int argc, const char** argv)
{
    using std::cout, std::endl, std::string;

    // TODO: test out if space-separated args with quotes works

    // Program title
    cout << "WAV to btsnd 0.1. btsnd is BIG_ENDIAN wav is LITTLE_ENDIAN\n";

    // No arguments entered by user. There wil always be 1 argument: the path of the program
    if (argc <= 1)
    {
        cout <<
            "Usage:\n"
            "wav2btsnd -in <input_file> <optional args>\n"
            "\n"
            "Optional Args:\n"
            "-out <outfile: str>: File path to create the converted file in, defualt is a bootSound.btsnd created in the same directory as the program.\n"
            "\n"
            "-makeWav:       Convert .btsnd to .wav.\n"
            YELLOW"The -makeWav argument is not necessary because the program automatically detects filetypes, so if input is .btsnd it will automatically convert to .wav. The argument can be used to force convert to .wav\n"
            RESET_COL"\n"
            "-makeBtsnd:     Convert .wav to .btsnd.\n"
            YELLOW"Similar situation as -makeWav. Argument can be used to force convert to .btsnd.\n"
            RESET_COL"\n"
            "\n"
            "The following commands are only for making btsnd's:\n"
            "-loopPoint <sampleforlooping: 4-byte-int>: Specifies a specific sample to loop from there to the end, once playthrough of the btsnd has finished once. (cant be used with -noLoop or -makeWav)\n"
            "\n"
            "-noLoop:        Makes it where the btsnd doesnt loop its sound. (cant be used with -loopPoint or -makeWav)\n"
            "\n"
            "-gamepadOnly:   Makes sound only hearable on gamepad. (cant be used with -tvOnly)\n"
            "\n"
            "-tvOnly:        Makes sound only hearable on tv. (cant be used with -gamepadOnly)\n"
            "\n";
        // successful exit
        return 0;
    }

    // Declarations
    byte wav_header1[4] = { 0x52, 0x49, 0x46, 0x46 };
    byte wav_header2[32] = {
        0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x02, 0x00, 0x80,  0xBB, 0x00, 0x00, 0x00, 0xEE,
        0x02, 0x00, 0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61
    };

    // input file specified by user
    string input_path = "";
    // output file specified by user. Default is "bootsound" in the same directory as the program
    string output_path = "bootsound";

    // tells whether program will convert input to wav or btsnd
    // true = convert .wav to .btsnd // false = convert .btsnd to .wav
    bool make_btsnd = true;
    // makes .btsnd file hearable on (?) // TODO: figure out what 2 means
    unsigned short int hearable_where = 2;
    // no hint to what range loop_point should be // TODO: figure out the acceptable range
    int loop_point = 0;
    bool silent_loop = false;

    // tels if arg -makeWav was used by the user
    bool used_makeWav = false;


    // Go through all the arguments given by user
    for (unsigned short int i = 0; i < argc; i++)
    {
        const char* arg = argv[i];

        if (arg == "-in")
        {
            i++; // skip the next argument
            // set the next argument to the input file path
            input_path = argv[i];
            dbg_println("input_path = " << input_path);

            // auto detect if converting btsnd to wav or vice versa
            if (str_ends_with(input_path, ".btsnd"))
                // convert .btsnd to wav
                make_btsnd = false;
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
            make_btsnd = false;
            used_makeWav = true;
        }
        else if (arg == "-makeBtsnd")
        {
            // check if makeWav is already an arg
            if (used_makeWav)
                exit_err("Cannot use -makeWav and -makeBtsnd together");
            
            make_btsnd = true;
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
    if (input_path == "")
    {
        exit_err("No input file was provided. Specify an input file with -in");
    }
    if (silent_loop == true && loop_point > 0)
    {
        exit_err("Cannot use -loopPoint and -noLoop together");
    }
    if ((silent_loop == true || loop_point > 0) && make_btsnd == false)
    {
        exit_err("Cannot use -loopPoint and -noLoop when the output is .wav");
    }


    // The core
    // Convert .WAV to .BTSND
    if (make_btsnd)
    {
        // set file extension of output file to .btsnd
        if (!str_ends_with(output_path, ".btsnd"))
            output_path += ".btsnd";
    }
    else // Convert .BTSND
    {
        // set file extension of output file to .wav
        if (!str_ends_with(output_path, ".wav"))
            output_path += ".wav";
    }

    // successful exit
    return 0;
}