#include <iostream>
#include <cli_args.h>
#include <pngparser.h>

const char usage[] =
        "Usage: png-debloat [OPTIONS] INFILE \n\n"
        "Utility for inspecting and cleaning png chunks.\n"
        "It allows to remove excess png chunks and chunks that exceeds limits defined by libpng.\n"
        "All critical chunks will be left unchanged.\n\n"
        "OPTIONS:\n"
        "  -h         -  Print this message and exit\n"
        "  -t         -  Parse png and print chunk info\n"
        "  -c NAME    -  Chunks to delete. Multiple flags may be specified to delete multiple chunks\n"
        "  -l         -  Remove LARGE chunks, i.e. chunks size of which exceeds libpng limits\n"
        "  -re        -  Remove additional data after IEND (does not affects png content)\n"
        "  -o OUTFILE -  Path to output file. By default processed png will be written to {INFILE}.out";

int main(int argc, char* argv[])
{
    Argument<bool> show_help("h");
    Argument<bool> test_png("t");
    MultiArgument<std::string> delete_chunks("c");
    Argument<bool> remove_large("l");
    Argument<bool> remove_after_end("re");
    Argument<std::string> out_path("o");

    ArgParser args(usage, &show_help, &test_png, &delete_chunks, &remove_large, &out_path, &remove_after_end);

    args.parse(argc, argv);

    if (args.nargs() < 1) {
        std::cerr << "error: missing required argument INFILE" << std::endl;
        std::cerr << args.get_usage() << std::endl;

        return 1;
    }

    if (show_help.get()) {
        std::cout << args.get_usage() << std::endl;
        return 0;
    }

    std::string infile = args.at(0);
    std::string outfile = infile + ".out";

    if (!out_path.get().empty()) {
        outfile = out_path.get();
    }

    PngParser parser;
    parser.from_files(infile, outfile);

    if (test_png.get()) {
        try {
            parser.print_chunks();
        } catch (std::exception& e) {
            std::cerr << "error: failed to process png" << std::endl;
            std::cerr << e.what() << std::endl;
            return 1;
        }

    } else if (!delete_chunks.empty() || remove_large.get()) {
        std::vector<std::string> to_remove;

        for (auto &val : delete_chunks.get()) {
            if (is_critical_chunk(val)) {
                std::cerr << "error: cannot remove critical chunk " << val << std::endl;
                return 1;
            }

            if (!is_known_chunk(val)) {
                std::cerr << "warning: unknown chunk name " << val << std::endl;
            }

            to_remove.emplace_back(val);
        }

        try {
            parser.remove_chunks(to_remove, remove_large.get(), remove_after_end.get());
        } catch (std::exception& e) {
            std::cerr << "error: failed to process png" << std::endl;
            std::cerr << e.what() << std::endl;
            return 1;
        }

    } else {
        std::cerr << "error: no task specified" << std::endl;
        std::cerr << args.get_usage() << std::endl;
        return 1;
    }

    return 0;
}