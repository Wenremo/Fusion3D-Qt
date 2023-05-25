#pragma once
#include <iostream>
#ifdef LICENCE_EXPORTS
#define LICENCE_API __declspec(dllexport) 
#else
#define LICENCE_API __declspec(dllimport) 
#endif
LICENCE_API bool isPermissionEnabled();
