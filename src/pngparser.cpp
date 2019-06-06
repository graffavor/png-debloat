#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include "pngparser.h"
#include "logging.h"

using namespace std;

bool is_little_endian = false;

void endian_swap(uint32_t* x)
{
    if (is_little_endian) {
        auto* buf = (char*) x;
        char tmp;

        tmp = buf[0];
        buf[0] = buf[3];
        buf[3] = tmp;
        tmp = buf[1];
        buf[1] = buf[2];
        buf[2] = tmp;
    }
}

bool is_known_chunk(const string& name)
{
    for (auto& i : CRITICAL_CHUNKS) {
        if (name == i) {
            return true;
        }
    }

    for (auto& i : ANCILLARY_CHUNKS) {
        if (name == i) {
            return true;
        }
    }

    return false;
}

bool is_critical_chunk(const string& name)
{
    for (auto& i : CRITICAL_CHUNKS) {
        if (name == i) {
            return true;
        }
    }

    return false;
}

PngParser::PngParser()
{
    // test endian'ess of machine
    uint32_t endian_test = 1;
    is_little_endian = ((*(char*) (&endian_test)) != 0);
}

PngParser::~PngParser()
{
    if (should_close_is && is != nullptr) {
        if (((ofstream*)is)->is_open()) ((ofstream*)is)->close();
        delete is;
    }

    if (should_close_os && os != nullptr) {
        if (((ofstream*)os)->is_open()) ((ofstream*)os)->close();
        delete os;
    }
}

void PngParser::from_files(const string& infile, const string& outfile)
{
    ifile = infile;
    ofile = outfile;
}

void PngParser::from_streams(istream* instream, ostream* outstream)
{
    if (instream->bad()) {
        throw runtime_error("bad input");
    }

    if (outstream->bad()) {
        throw runtime_error("bad output");
    }

    is = instream;
    os = outstream;
}

void PngParser::make_input()
{
    if (is == nullptr && !ifile.empty()) {
        is = new ifstream(ifile, ios_base::binary | ios_base::in);
        should_close_is = true;
    }

    if (is == nullptr || is->bad()) {
        throw runtime_error("bad or missing input");
    }
}

void PngParser::make_output()
{
    if (os == nullptr && !ofile.empty()) {
        os = new ofstream(ofile, ios_base::binary | ios_base::out | ios_base::trunc);
        should_close_os = true;
    }

    if (os == nullptr || os->bad()) {
        throw runtime_error("bad or missing output");
    }
}

void PngParser::parse_pngstream(istream* input, function<void(const PngChunk&)> handle_fn)
{
    auto start_offset = input->tellg();

    char buf[PNG_SIGNATURE_SIZE + 1];
    buf[PNG_SIGNATURE_SIZE] = 0;
    input->read(buf, PNG_SIGNATURE_SIZE);

    if (memcmp(PNG_SIGNATURE, buf, PNG_SIGNATURE_SIZE) != 0 ) {
        throw runtime_error("not a png file: invalid signature");
    }

    while (input->good()) {
        PngChunk chunk;

        input->read((char*) &chunk.length, 4);

        if (input->eof()) {
            break;
        }

        endian_swap(&chunk.length);

        char nbuf[5] = {0, 0, 0, 0, 0};
        input->read(nbuf, 4);
        chunk.name = string(nbuf);
        chunk.offset = (uint64_t) (input->tellg() - start_offset);

        if (chunk.length > 0) {
            chunk.data = new char[chunk.length];
            input->read(chunk.data, chunk.length);
        }

        input->read((char*) &chunk.crc, 4);

        endian_swap(&chunk.crc);

        handle_fn(chunk);

        // No data should be present in png after IEND
        if (chunk.name == "IEND") {
            break;
        }
    }

    // Data after png end which technically is error
    if (!input->eof()) {
        // Create dummy chunk to handle data
        PngChunk chunk;

        chunk.name = "ERRDATIEND";

        auto cur_offset = input->tellg();
        input->seekg(0, std::ios::end);
        auto file_size = input->tellg();

        chunk.offset = (uint32_t)(cur_offset - start_offset);
        chunk.length = (uint32_t)(file_size - cur_offset);
        if (chunk.length > 0) {
            chunk.data = new char[chunk.length];
            input->read(chunk.data, chunk.length);
        }

        handle_fn(chunk);
    }
}

void PngParser::print_chunks()
{
    make_input();

    cout << endl;

    parse_pngstream(is, [](const PngChunk& chunk) -> void {
        if (chunk.name != "ERRDATIEND") {
            cout << "Chunk " << chunk.name;
            cout << " at offset 0x" << setfill('0') << setw(8) << hex << chunk.offset;
            cout << ", length " << dec << chunk.length << endl;
        } else {
            cout << "Error: additional data after IEND chunk";
            cout << " at offset 0x" << setfill('0') << setw(8) << hex << chunk.offset;
            cout << ", length " << dec << chunk.length << endl;
        }
    });

    cout << endl;
}

void PngParser::write_chunk(const PngParser::PngChunk& chunk)
{
    if (chunk.name != "ERRDATIEND") {
        uint32_t length = chunk.length;
        uint32_t crc = chunk.crc;

        endian_swap(&length);
        endian_swap(&crc);

        os->write((char*) &length, 4);
        os->write(chunk.name.c_str(), 4);
        os->write(chunk.data, chunk.length);
        os->write((char*) &crc, 4);
    } else {
        os->write(chunk.data, chunk.length);
    }
}

void PngParser::remove_chunks(vector<string> chunks, bool remove_large = false, bool remove_after_end = false)
{
    make_input();
    make_output();

    os->write(PNG_SIGNATURE, PNG_SIGNATURE_SIZE);

    parse_pngstream(is, [&](const PngChunk& chunk) -> void {
        if (chunk.name != "ERRDATIEND") {
            bool should_keep = (find(chunks.begin(), chunks.end(), chunk.name) == chunks.end());

            if (remove_large && chunk.length > USER_MAX_CHUNK_SIZE) {
                should_keep = false;
            }

            if (is_critical_chunk(chunk.name) || should_keep) {
                this->write_chunk(chunk);
            } else {
                LOG(info) << "removed chunk " << chunk.name << ", length " << chunk.length << endl;
            }
        } else if (!remove_after_end) {
            LOG(info) << "warning: additional data after IEND chunk left intact" << endl;
            this->write_chunk(chunk);
        }
    });
}