#ifndef CRYPT_HPP
#define CRYPT_HPP

#if defined(__APPLE__) && defined(__MACH__)
    #include <crypt/macos.hpp>
#elif defined(__linux__)
    #include <crypt/linux.hpp>
#else
    #error "Unsupported OS"
#endif

#endif