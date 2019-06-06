#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "../include/pngparser.h"

namespace py = pybind11;

template<typename T>
std::vector<T> array2vec(const pybind11::array_t<T>& arr)
{
    std::vector<T> res(arr.size());
    std::memcpy(res.data(), arr.data(), arr.size());
    return res;
}

template<typename CharT, typename TraitsT = std::char_traits<CharT>>
class vectorwrapbuf : public std::basic_streambuf<CharT, TraitsT> {
public:
    vectorwrapbuf(std::vector<CharT>& src)
    {
        this->setg(src.data(), src.data(), src.data() + src.size());
    }
};

py::array_t<char>
remove_chunks(py::array_t<char> src, std::vector<std::string> chunks, bool remove_large = false, bool remove_after_end = false)
{
    auto tmpv = array2vec(src);
    vectorwrapbuf<char> databuf(tmpv);
    std::istream in(&databuf);
    std::ostringstream os;

    PngParser p;
    p.from_streams(&in, &os);

    p.remove_chunks(std::move(chunks), remove_large, remove_after_end);

    std::string tmp = os.str();

    auto osize = (size_t) tmp.length();
    auto res = py::array_t<char>(osize);
    auto buf = res.request();

    std::memcpy(buf.ptr, tmp.c_str(), osize);

    return res;
}

PYBIND11_MODULE(_png_debloat, m)
{
    m.doc() = "Module for inspecting and optimizing content of png files";

    m.def("remove_chunks", &remove_chunks, "Remove unnecessary or large chunks from png file");

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
