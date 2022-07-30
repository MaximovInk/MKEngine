#pragma once

#ifdef MK_BUILD_DLL
#define MK_API __declspec(dllexport)
#else
#define MK_API __declspec(dllimport)
#endif