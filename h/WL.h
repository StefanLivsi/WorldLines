//
// Created by Stefan on 19-Jun-23.
//

#ifndef WORLDLINES_WL_H
#define WORLDLINES_WL_H
#include <emmintrin.h>
#include <immintrin.h>
#include <iostream>

#define LOG2_TABLE_SIZE_LOG2 8
#define LOG2_TABLE_SIZE (1 << LOG2_TABLE_SIZE_LOG2)
#define LOG2_TABLE_SCALE ((float) ((LOG2_TABLE_SIZE)-1))
#define EXP2_TABLE_SIZE_LOG2 9
#define EXP2_TABLE_SIZE (1 << EXP2_TABLE_SIZE_LOG2)
#define EXP2_TABLE_OFFSET (EXP2_TABLE_SIZE/2)
#define EXP2_TABLE_SCALE ((float) ((EXP2_TABLE_SIZE/2)-1))

using namespace std;
struct RGBA{
    RGBA(uint8_t par){
        r = par;
        g = par;
        b = par;
    }
    RGBA(uint8_t par1, uint8_t par2, uint8_t par3, uint8_t par4){
        r = par1;
        g = par2;
        b = par3;
    }
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};
struct RGBAf{
    RGBAf(float par){
        r = par;
        g = par;
        b = par;
    }
    RGBAf(float par1, float par2, float par3, float par4){
        r = par1;
        g = par2;
        b = par3;
    }
    float r;
    float g;
    float b;
    float a;
};
struct RGBAarray{
    uint8_t* r;
    uint8_t* g;
    uint8_t* b;
    uint8_t* a;
};
union f4 {
    int32_t i[4];
    uint32_t u[4];
    float f[4];
    __m128 m;
    __m128i mi;
};
class WL {
public:
     WL(){}
     static int LoadImage(char const*,int,int,int,int);
     static void addConstant(RGBA,bool simd = true);
     static void WriteImagePng(char const* cc, uint8_t* dest = img);
     static void SubConstant(RGBA, bool simd = true);
     static void InvSubConstant(RGBA, bool simd = true);
     static void MulConstant(RGBA, bool simd = true);
     static void DivConstant(RGBAf, bool simd = true);
     static void InvDivConstant(RGBAf, bool simd = true);
     static void PowConstant(RGBAf rgba, bool simd = true);
     static void MinConstant(RGBA rgba, bool simd = true);
     static void MaxConstant(RGBA rgba, bool simd = true);
     static void Abs(bool simd = true);
     static void Grayscale(bool simd = true);
     static void Inverse(bool simd = true);
     static void log2_init();
     static void exp2_init();
     static void Log(bool simd = true);
     static void Sobel(bool cacheOpt = true, int N = 3);
     static bool png;

private:
    static uint8_t* img;
    static int img_size;
    static int step;
    static int w;
    static int h;
    static RGBAarray imgStruct;
    static float log2_table[2*LOG2_TABLE_SIZE];
    static float exp2_table[2*EXP2_TABLE_SIZE];
    static bool onlylog;

    static inline __m256i __attribute__((always_inline, artificial,gnu_inline)) mul(__m256i&v, __m128i constant);
    static inline __m128i __attribute__((always_inline, artificial,gnu_inline)) mulF(__m128i&v, __m128 constant);
    static inline __m128i __attribute__((always_inline, artificial,gnu_inline)) mulFInv(__m128i&v, __m128 constant);
    static inline __m128i __attribute__((always_inline, artificial,gnu_inline)) Pow(__m128i&v, __m128 constant);
    static inline __m128  __attribute__((always_inline, artificial,gnu_inline)) log2f4(__m128 v);
    static inline __m128i  __attribute__((always_inline, artificial,gnu_inline)) log2i(__m128i v);
    static inline __m128  __attribute__((always_inline, artificial,gnu_inline)) exp2f4(__m128 v);
    static void sobelThread(int start, int end, int N);

};


#endif //WORLDLINES_WL_H
