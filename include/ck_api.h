#pragma once

#ifdef _WIN32
  #define CK_API extern "C" __declspec(dllexport)
#else
  #define CK_API extern "C" __attribute__((visibility("default")))
#endif

