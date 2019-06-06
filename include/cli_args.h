#ifndef PNG_DEBLOAT_CLI_ARGS_H
#define PNG_DEBLOAT_CLI_ARGS_H

#include <string>
#include <vector>
#include <map>
#include <exception>

using namespace std;

class VArg {
public:
    VArg() = default;

    virtual void set_value(const string& val) = 0;

    virtual bool expects_value() { return true; }

    virtual string get_name() = 0;
};

template<typename T = string>
class Argument : public VArg {
public:
    explicit Argument(string n) : name(n) { };

    void set_value(const string& val) override { value = val; };

    string get_name() override { return name; }

    T get() { return value; }

protected:
    string name;
    T value;
};

template<>
class Argument<bool> : public VArg {
public:
    explicit Argument(string n) : name(n) { };

    void set_value(const string& val) override { value = true; }

    bool get() { return value; }

    string get_name() override { return name; }

    bool expects_value() override { return false; }

protected:
    string name;
    bool value = false;
};

template<typename T = string>
class MultiArgument : public VArg {
public:
    explicit MultiArgument(string n) : name(n) { };

    ~MultiArgument()
    {
        value.clear();
    }

    void set_value(const string& val) override { value.emplace_back(val); }

    vector<T> get() { return value; }

    string get_name() override { return name; }

    size_t size() { return value.size(); }

    bool empty() { return value.empty(); }

    T at(size_t pos) { return value.at(pos); }

protected:
    string name;
    vector<T> value;
};

class arg_error : public runtime_error {
public:
    arg_error(const std::string& what) : runtime_error("argparse error: " + what) {}

    virtual ~arg_error() {}
};

class missing_arg_error : public arg_error {
public:
    missing_arg_error(const string& argname) : arg_error("missing value for argument " + argname) {};

    virtual ~missing_arg_error() {}
};

class ArgParser {
public:
    template<class ...T>
    ArgParser(const string& description, T... args)
    {
        desc = description;
        init(args...);
    }

    ~ArgParser()
    {
        args.clear();
        positional.clear();
    }

    string get_usage() { return desc; }

    void parse(int, char**);

    size_t nargs() { return positional.size(); }

    string get(size_t i, string def)
    {
        if (i >= 0 && i < positional.size()) {
            return positional[i];
        } else {
            return def;
        }
    }

    bool has(size_t i) { return i >= 0 && i < positional.size(); }

    string at(size_t i)
    {
        if (i >= 0 && i < positional.size()) return positional[i];
        else
            throw range_error("Positional argument index out of range");
    };

private:
    template<class ...T>
    void init(VArg* arg, T... args)
    {
        init(arg);
        init(args...);
    }

    void init(VArg* arg)
    {
        args.push_back(arg);
    }

    string desc;
    vector<VArg*> args;
    vector<string> positional;
};

#endif //PNG_DEBLOAT_CLI_ARGS_H
