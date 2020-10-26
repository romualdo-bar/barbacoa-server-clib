/*
 * Copyright (c) 2019 Andrew Masilevich (a.masilevich@gmail.com)
 *
 * The MIT License
 *
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "exit_codes.h"

#if defined(__gnu_linux__)
#include <linux/limits.h>
#else // Linux
#ifndef MAX_INPUT
#define MAX_INPUT 512
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#endif // Other

// Workaround for varying preprocessing behavior between MSVC and gcc
#define SRV_C_EXPAND_MACRO_(x) x

// suppress warning "conditional expression is constant" in the while(0) for visual c++
// http://cnicholson.net/2009/03/stupid-c-tricks-dowhile0-and-c4127/
#define SRV_C_MULTILINE_MACRO_BEGIN_ \
    do                               \
    {
#ifdef _MSC_VER
#define SRV_C_MULTILINE_MACRO_END_                            \
    __pragma(warning(push)) __pragma(warning(disable : 4127)) \
    }                                                         \
    while (0)                                                 \
    __pragma(warning(pop))
#else
#define SRV_C_MULTILINE_MACRO_END_ \
    }                              \
    while (0)
#endif

#ifndef NDEBUG

#define SRV_C_STDERR_FILENO_ stderr

#define SRV_C_TRACE_INFO_(func, call_mode, e_code, fmt_message, ...)                  \
    if (!(func))                                                                      \
    {                                                                                 \
        char szMSG_FMT[MAX_INPUT] = "";                                               \
        char szMSG[MAX_INPUT] = "";                                                   \
        snprintf(szMSG_FMT, sizeof(szMSG_FMT), fmt_message, __VA_ARGS__);             \
        snprintf(szMSG, sizeof(szMSG), "%s (%d): %s", __FILE__, __LINE__, szMSG_FMT); \
        if (call_mode && errno != 0)                                                  \
        {                                                                             \
            perror(szMSG);                                                            \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            fprintf(SRV_C_STDERR_FILENO_, "%s\n", szMSG);                             \
        }                                                                             \
        if (e_code >= 0)                                                              \
        {                                                                             \
            exit(e_code);                                                             \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            abort();                                                                  \
        }                                                                             \
    }

inline void _SRV_C_TRACE(const char* t_file, int t_line, const char* info)
{
    fprintf(SRV_C_STDERR_FILENO_, "%s (%d)", t_file, t_line);
    if (info)
    {
        fprintf(SRV_C_STDERR_FILENO_, "\t%s", info);
    }
    fprintf(SRV_C_STDERR_FILENO_, "\n");
}

#define SRV_C_TRACE_TEXT_(data) _SRV_C_TRACE(__FILE__, __LINE__, data);
#define SRV_C_TRACE_ _SRV_C_TRACE(__FILE__, __LINE__, NULL);
#else // DEBUG
#define SRV_C_TRACE_INFO_(func, call_mode, e_code, fmt_message, ...) \
    if (!(func))                                                     \
    {                                                                \
        exit(e_code);                                                \
    }

#define SRV_C_TRACE_TEXT_(data) (void)(data)
#define SRV_C_TRACE_ SRV_C_TRACE_TEXT_(NULL)
#endif

#define SRV_C_CALL_WITH_EXIT_CODE_(func, e_code) SRV_C_TRACE_INFO_(func, 1, e_code, "%s", #func)
#define SRV_C_CHECK_WITH_EXIT_CODE_(cond, e_code, fmt_message, ...) \
    SRV_C_TRACE_INFO_(cond, 0, e_code, fmt_message, __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// assert/debug

#define SRV_C_ASSERT_FMT(cond, fmt_message, ...)                                                                   \
    SRV_C_EXPAND_MACRO_(                                                                                           \
        SRV_C_MULTILINE_MACRO_BEGIN_ SRV_C_CHECK_WITH_EXIT_CODE_(cond, exit_code_error, fmt_message, __VA_ARGS__); \
        SRV_C_MULTILINE_MACRO_END_)
#define SRV_C_ASSERT(cond) SRV_C_ASSERT_FMT(cond, "%s", #cond)
#define SRV_C_ASSERT_TEXT(cond, message) SRV_C_ASSERT_FMT(cond, "%s", message)

#define SRV_C_ERROR_FMT(fmt_message, ...) SRV_C_ASSERT_FMT(0, fmt_message, __VA_ARGS__)
#define SRV_C_ERROR(message) SRV_C_ASSERT_TEXT(0, message)

#define SRV_C_AASSERT_FMT(cond, fmt_message, ...)                                                                     \
    SRV_C_EXPAND_MACRO_(SRV_C_MULTILINE_MACRO_BEGIN_ SRV_C_CHECK_WITH_EXIT_CODE_(cond, -1, fmt_message, __VA_ARGS__); \
                        SRV_C_MULTILINE_MACRO_END_)
#define SRV_C_AASSERT(cond) SRV_C_AASSERT_FMT(cond, "%s", #cond)
#define SRV_C_AASSERT_TEXT(cond, message) SRV_C_AASSERT_FMT(cond, "%s", message)

#define SRV_C_ABORT_FMT(fmt_message, ...) SRV_C_AASSERT_FMT(0, fmt_message, __VA_ARGS__)
#define SRV_C_ABORT(message) SRV_C_AASSERT_TEXT(0, message)

#define SRV_C_CALL(func)                                                                               \
    SRV_C_EXPAND_MACRO_(SRV_C_MULTILINE_MACRO_BEGIN_ SRV_C_CALL_WITH_EXIT_CODE_(func, exit_code_error) \
                            SRV_C_MULTILINE_MACRO_END_)

#define SRV_C_TRACE_TEXT(data) \
    SRV_C_EXPAND_MACRO_(SRV_C_MULTILINE_MACRO_BEGIN_ SRV_C_TRACE_TEXT_(data) SRV_C_MULTILINE_MACRO_END_)
#define SRV_C_TRACE SRV_C_EXPAND_MACRO_(SRV_C_MULTILINE_MACRO_BEGIN_ SRV_C_TRACE_ SRV_C_MULTILINE_MACRO_END_)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// math

#define SRV_C_MAX(a, b)         \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })

#define SRV_C_MIN(a, b)         \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })
