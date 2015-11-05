#pragma once

#ifdef _MSC_VER
#include "cpp/xf.hpp"
#endif

#ifdef __GNUC__
#include "cpp/xf.hpp"
#endif

#ifdef _JIT
#include "rpp/xf.rpp"
#endif

#ifdef _NASM
#include "rpp/xf.rpp"
#endif

#ifdef _RVM
#include "rpp/xf.rpp"
#endif

#ifdef _GPP
#include "gpp/xf.rpp"
#endif
