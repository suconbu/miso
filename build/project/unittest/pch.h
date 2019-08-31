//
// pch.h
// Header for standard system include files.
//

#pragma once

// http://forums.codeguru.com/showthread.php?312742-Visual-C-Debugging-How-to-manage-memory-leaks
#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_CLIENTBLOCK
#endif

//#define MISO_HEADER_ONLY
#include "miso/miso.h"
#include "gtest/gtest.h"

#include <crtdbg.h>
#include <stdint.h>
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_CLIENTBLOCK
#endif

#define TEST_TRACE(f, ...) _RPTN(_CRT_WARN, "[%s:%d] " f "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
