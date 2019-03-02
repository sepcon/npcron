#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

class BadSyntaxException : public std::exception
{
public:
    static void raise(std::string reason) __attribute__((noreturn))
    {
        throw BadSyntaxException(std::move(reason));
    }
    BadSyntaxException(std::string&& what)
    {
        _what = std::move(what);
    }
    const char* what() const noexcept override
    {
        return _what.c_str();
    }

private:
    std::string _what;
};

#endif // EXCEPTION_H
