#ifndef AUEXC_H
#define AUEXC_H

#include <exception>

class GoogleAuthenticationException : public std::exception {
    public:
    virtual const char* what() const throw();
    virtual const char* message() const throw();
};

#endif