//
//  UtilityLoops.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 14/10/2023.
//

#ifndef UtilityLoops_h
#define UtilityLoops_h

namespace akml {
    template <std::size_t from, std::size_t to>
    struct for_ {
        template<template<std::size_t, std::size_t> class Fn, typename typearg>
        static void run(typearg arg) {
            Fn<from, from+1>::run(arg);
            for_<from + 1, to>::template run<Fn, typearg>(arg);
        }
        
        template<template<std::size_t> class Fn, typename typearg>
        static void run(typearg arg) {
            Fn<from>::run(arg);
            for_<from + 1, to>::template run<Fn, typearg>(arg);
        }
        
        template<template<std::size_t, std::size_t> class Fn, typename... typeargs>
        static void run(typeargs... args) {
            Fn<from, from+1>::run(args...);
            for_<from + 1, to>::template run<Fn, typeargs...>(args...);
        }
        
        template<template<std::size_t> class Fn, typename... typeargs>
        static void run(typeargs... args) {
            Fn<from>::run(args...);
            for_<from + 1, to>::template run<Fn, typeargs...>(args...);
        }
    };

    template <std::size_t to>
    struct for_<to, to> {
        template<template<std::size_t, std::size_t> class Fn, typename... typeargs>
        static void run(typeargs... args) {}
        template<template<std::size_t> class Fn, typename... typeargs>
        static void run(typeargs... args) {}
        
        template<template<std::size_t, std::size_t> class Fn, typename typearg>
        static void run(typearg arg) {}
        template<template<std::size_t> class Fn, typename typearg>
        static void run(typearg arg) {}
    };

}


#endif /* UtilityLoops_h */
