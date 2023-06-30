#pragma once
#include <string.h>
#include <chrono>
double relative[2]; //nosimd 0, simd 1;


#define StartTimer(name)	{ const char* timername = #name;\
				auto start = std::chrono::steady_clock::now();

#define EndTimer(issimd)  auto end =  std::chrono::steady_clock::now(); \
                    bool isSimd = issimd;                               \
                    if(isSimd)relative[1]=chrono::duration_cast<chrono::nanoseconds>(end - start).count(); \
                    else relative[0]=chrono::duration_cast<chrono::nanoseconds>(end - start).count();                                                    \
						} \


