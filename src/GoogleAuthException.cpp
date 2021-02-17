#include "GoogleAuthException.hpp"

const char* GoogleAuthenticationException::what() const throw()
{
    return "GoogleAuthenticationException";
}

const char* GoogleAuthenticationException::message() const throw()
{
    return "Google authentication failure";
}