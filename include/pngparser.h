#ifndef PNG_DEBLOAT_PARSER_H
#define PNG_DEBLOAT_PARSER_H

#include <string>
#include <cstdint>
#include <vector>
#include <utility>
#include <fstream>
#include <functional>

#ifdef WITH_LIBPNG
#ifdef __APPLE__
#include <pnglibconf.h>
#else
#include <libpng/pnglibconf.h>
#endif
#define USER_MAX_CHUNK_SIZE PNG_USER_CHUNK_MALLOC_MAX
#else
#define USER_MAX_CHUNK_SIZE 8000000
#endif

#define PNG_SIGNATURE_SIZE 8
#define PNG_SIGNATURE "\211PNG\r\n\032\n"

using namespace std;

const char CRITICAL_CHUNKS[][5] = {
        "IHDR",
        "PLTE",
        "IDAT",
        "IEND"
};

const char ANCILLARY_CHUNKS[][5] = {
        "tRNS",
        "gAMA",
        "cHRM",
        "sRGB",
        "iCCP",
        "iTXt",
        "tEXt",
        "zTXt",
        "bKGD",
        "pHYs",
        "sBIT",
        "sPLT",
        "hIST",
        "tIME"
};

bool is_known_chunk(const string& name);

bool is_critical_chunk(const string& name);

class PngParser {
public:
    PngParser();

    ~PngParser();

    void from_files(const string&, const string&);
    void from_streams(istream*, ostream*);

    void print_chunks();

    void remove_chunks(vector<string> chunks, bool remove_large, bool remove_after_end);

protected:
    struct PngChunk {
        string name;
        uint32_t length = 0;
        uint32_t crc = 0;
        uint64_t offset = 0;
        char* data = nullptr;

        ~PngChunk()
        {
            if (length > 0 && data != nullptr) {
                delete[] data;
            }
        }
    };

    void parse_pngstream(istream*, function<void(const PngChunk&)>);

    void write_chunk(const PngChunk&);

    void make_input();
    void make_output();

private:
    string ifile, ofile;
    istream* is = nullptr;
    ostream* os = nullptr;

    bool should_close_is = false;
    bool should_close_os = false;
};

#endif //PNG_DEBLOAT_PARSER_H
