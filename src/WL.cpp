//
// Created by Stefan on 19-Jun-23.
//

#include "../h/WL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb-master/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb-master/stb_image_write.h"

#include <stdio.h>
#include <algorithm>
#include <thread>
#include <vector>
__m128i const00 = _mm_set1_epi8(0);
int WL::h;
int WL::w;
int WL::step;
int WL::img_size;
uint8_t* WL::img;
RGBAarray WL::imgStruct;
float WL::log2_table[2*LOG2_TABLE_SIZE];
float WL::exp2_table[2*EXP2_TABLE_SIZE];
int get_cache_line_size ( int id );
bool WL::onlylog = false;
bool WL::png;
int WL::LoadImage(char const *c , int w1, int h1, int comp, int i1) {
    img = stbi_load( c, &w1, &h1, &comp, i1);
    imgStruct.r = new uint8_t[w1*h1];
    imgStruct.g = new uint8_t[w1*h1];
    imgStruct.b = new uint8_t[w1*h1];
    imgStruct.a = new uint8_t[w1*h1];

    int cnt = 0;
    img_size = w1 * h1 * comp;
    step = comp;
    for(uint8_t* p = img; p != img + img_size; p += step){
        imgStruct.r[cnt] = *(p+0);
        imgStruct.g[cnt] = *(p+1);
        imgStruct.b[cnt] = *(p+2);
        imgStruct.a[cnt] = *(p+3);
        cnt++;
        //std::cout << cnt << '\n';
    }

    WL::w = w1;
    WL::h = h1;
    return !img ? -1 : 0;
}

 void WL::addConstant(RGBA rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i]+=rgba.r;
            imgStruct.g[i]+=rgba.g;
            imgStruct.b[i]+=rgba.b;
        }
    }
    else{
        const int alignedWH = w*h - w*h % 32;
        __m256i vb1 = _mm256_set1_epi8(rgba.r);
        __m256i vb2 = _mm256_set1_epi8(rgba.g);
        __m256i vb3 = _mm256_set1_epi8(rgba.b);

        for (long long i = 0; i < alignedWH; i+=32) {
            __m256i va1 = _mm256_load_si256((__m256i*)(imgStruct.r+i));
            __m256i va2 = _mm256_load_si256((__m256i*)(imgStruct.g+i));
            __m256i va3 = _mm256_load_si256((__m256i*)(imgStruct.b+i));

            __m256i vd1 = _mm256_adds_epu8(va1,vb1);
            __m256i vd2 = _mm256_adds_epu8(va2,vb2);
            __m256i vd3 = _mm256_adds_epu8(va3,vb3);

            _mm256_store_si256((__m256i*)(imgStruct.r + i), vd1);
            _mm256_store_si256((__m256i*)(imgStruct.g + i), vd2);
            _mm256_store_si256((__m256i*)(imgStruct.b + i), vd3);

        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i]+=rgba.r;
            imgStruct.g[i]+=rgba.g;
            imgStruct.b[i]+=rgba.b;
        }
    }
}

void WL::WriteImagePng(char const* cc, uint8_t* dest) {
    int cnt = 0;
    for(uint8_t* p = img; p != img + img_size; p += step){
        *(p+0) = imgStruct.r[cnt];
        *(p+1) = imgStruct.g[cnt];
        *(p+2) = imgStruct.b[cnt];
        if(step==4) {
            *(p + 3) = imgStruct.a[cnt];
        }
        cnt++;
        //std::cout << cnt << '\n';
    }
    if(png){
        stbi_write_png(cc, w, h, step, dest, w * step);
    }

    else
        stbi_write_jpg(cc, w, h, step, dest, 100);
}

void WL::InvSubConstant(RGBA rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i]=rgba.r - imgStruct.r[i];
            imgStruct.g[i]=rgba.g - imgStruct.g[i];
            imgStruct.b[i]=rgba.b - imgStruct.b[i];
        }
    }
    else{
        const int alignedWH = w*h - w*h % 32;
        __m256i vb1 = _mm256_set1_epi8(rgba.r);
        __m256i vb2 = _mm256_set1_epi8(rgba.g);
        __m256i vb3 = _mm256_set1_epi8(rgba.b);
        for (long long i = 0; i < alignedWH; i+=32) {
            __m256i va1 = _mm256_load_si256((__m256i*)(imgStruct.r+i));
            __m256i va2 = _mm256_load_si256((__m256i*)(imgStruct.g+i));
            __m256i va3 = _mm256_load_si256((__m256i*)(imgStruct.b+i));
            __m256i vd1 = _mm256_subs_epu8(vb1,va1);
            __m256i vd2 = _mm256_subs_epu8(vb2,va2);
            __m256i vd3 = _mm256_subs_epu8(vb3,va3);
            _mm256_store_si256((__m256i*)(imgStruct.r + i), vd1);
            _mm256_store_si256((__m256i*)(imgStruct.g + i), vd2);
            _mm256_store_si256((__m256i*)(imgStruct.b + i), vd3);
        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i]-=rgba.r;
            imgStruct.g[i]-=rgba.g;
            imgStruct.b[i]-=rgba.b;
        }
    }
}

void WL::SubConstant(RGBA rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i]-=rgba.r;
            imgStruct.g[i]-=rgba.g;
            imgStruct.b[i]-=rgba.b;
            imgStruct.a[i]-=rgba.a;
        }
    }
    else{
        const int alignedWH = w*h - w*h % 32;
        __m256i vb1 = _mm256_set1_epi8(rgba.r);
        __m256i vb2 = _mm256_set1_epi8(rgba.g);
        __m256i vb3 = _mm256_set1_epi8(rgba.b);

        for (long long i = 0; i < alignedWH; i+=32) {
            __m256i va1 = _mm256_load_si256((__m256i*)(imgStruct.r+i));
            __m256i va2 = _mm256_load_si256((__m256i*)(imgStruct.g+i));
            __m256i va3 = _mm256_load_si256((__m256i*)(imgStruct.b+i));

            __m256i vd1 = _mm256_subs_epu8(va1,vb1);
            __m256i vd2 = _mm256_subs_epu8(va2,vb2);
            __m256i vd3 = _mm256_subs_epu8(va3,vb3);

            _mm256_store_si256((__m256i*)(imgStruct.r + i), vd1);
            _mm256_store_si256((__m256i*)(imgStruct.g + i), vd2);
            _mm256_store_si256((__m256i*)(imgStruct.b + i), vd3);

        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i]-=rgba.r;
            imgStruct.g[i]-=rgba.g;
            imgStruct.b[i]-=rgba.b;

        }
    }
}

void WL::MulConstant(RGBA rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i]*=rgba.r;
            imgStruct.g[i]*=rgba.g;
            imgStruct.b[i]*=rgba.b;
        }
    }
    else{
        const int alignedWH = w*h - w*h % 32;
        __m128i vb1 = _mm_set1_epi16(rgba.r);
        __m128i vb2 = _mm_set1_epi16(rgba.g);
        __m128i vb3 = _mm_set1_epi16(rgba.b);

        uint8_t* debug = new (uint8_t[32]);
        __m256i constFF = _mm256_set1_epi16(0xFF);
        int threadNum = std::thread::hardware_concurrency();
        vector<thread> threads;
        int chunk_size = alignedWH / threadNum;
        for (int i = 0; i < threadNum; ++i)
        {
            int start,end;
            start = i * chunk_size;
            end = (i + 1) * chunk_size;
            /*if(i==0){
                start = i * chunk_size;
                end = (i + 1) * chunk_size;
            }
            else if(i == (threadNum-1)){
                start = i * chunk_size;
                end = (i + 1) * chunk_size;
            }
            else{
                start = i * chunk_size;
                end = (i + 1) * chunk_size;
            }*/

            threads.emplace_back([start, end,vb1,vb2,vb3]()
                                 {
                                     for (long long i = start; i < end; i+=32) {

                                         __m256i vdr = _mm256_load_si256((__m256i*)&imgStruct.r[i]);
                                         __m256i vdg = _mm256_load_si256((__m256i*)&imgStruct.g[i]);
                                         __m256i vdb = _mm256_load_si256((__m256i*)&imgStruct.b[i]);


                                         vdr = mul(vdr,vb1);
                                         vdg = mul(vdg,vb2);
                                         vdb = mul(vdb,vb3);

                                         _mm256_store_si256((__m256i*)&imgStruct.r[i],vdr);
                                         _mm256_store_si256((__m256i*)&imgStruct.g[i],vdg);
                                         _mm256_store_si256((__m256i*)&imgStruct.b[i],vdb);

                                     }
                                 });
        }
        for(int i = 0; i < threads.size(); i++){
            threads[i].join();
        }
        /*for (long long i = 0; i < alignedWH; i+=32) {

            __m256i vdr = _mm256_load_si256((__m256i*)&imgStruct.r[i]);
            __m256i vdg = _mm256_load_si256((__m256i*)&imgStruct.g[i]);
            __m256i vdb = _mm256_load_si256((__m256i*)&imgStruct.b[i]);


            vdr = mul(vdr,vb1);
            vdg = mul(vdg,vb2);
            vdb = mul(vdb,vb3);

            _mm256_store_si256((__m256i*)&imgStruct.r[i],vdr);
            _mm256_store_si256((__m256i*)&imgStruct.g[i],vdg);
            _mm256_store_si256((__m256i*)&imgStruct.b[i],vdb);

        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i]*=rgba.r;
            imgStruct.g[i]*=rgba.g;
            imgStruct.b[i]*=rgba.b;
        }*/
    }
}

__m256i WL::mul(__m256i &v, __m128i constant) {

    __m128i lower = _mm256_castsi256_si128(v);
    __m128i higher = _mm256_extracti128_si256(v,1);
    __m128i unpackedLowerlo = _mm_unpacklo_epi8(lower,const00);
    __m128i unpackedLowerhi = _mm_unpackhi_epi8(lower,const00);
    unpackedLowerlo = _mm_mullo_epi16(unpackedLowerlo,constant);
    unpackedLowerhi = _mm_mullo_epi16(unpackedLowerhi,constant);
    lower = _mm_packus_epi16(unpackedLowerlo,unpackedLowerhi);
    __m128i unpackedHigherlo = _mm_unpacklo_epi8(higher,const00);
    __m128i unpackedHigherhi = _mm_unpackhi_epi8(higher,const00);
    unpackedHigherlo = _mm_mullo_epi16(unpackedHigherlo,constant);
    unpackedHigherhi = _mm_mullo_epi16(unpackedHigherhi,constant);
    higher = _mm_packus_epi16(unpackedHigherlo,unpackedHigherhi);
    v = _mm256_castsi128_si256(lower);
    v = _mm256_insertf128_si256(v,higher,1);
    return v;
}

void WL::DivConstant(RGBAf rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i]/=rgba.r;
            imgStruct.g[i]/=rgba.g;
            imgStruct.b[i]/=rgba.b;
        }
    }
    else {
        const int alignedWH = w * h - w * h % 16;

        __m128 vb1 = _mm_set1_ps(rgba.r);
        vb1 = _mm_rcp_ps(vb1);
        __m128 vb2 = _mm_set1_ps(rgba.g);
        vb2 = _mm_rcp_ps(vb2);
        __m128 vb3 = _mm_set1_ps(rgba.b);
        vb3 = _mm_rcp_ps(vb3);

        uint8_t *debug = new(uint8_t[32]);
        __m256i constFF = _mm256_set1_epi16(0xFF);
        for (long long i = 0; i < alignedWH; i += 16) {

            __m128i vdr = _mm_load_si128((__m128i *) &imgStruct.r[i]);
            __m128i vdg = _mm_load_si128((__m128i *) &imgStruct.g[i]);
            __m128i vdb = _mm_load_si128((__m128i *) &imgStruct.b[i]);


            vdr = mulF(vdr, vb1);
            vdg = mulF(vdg, vb2);
            vdb = mulF(vdb, vb3);

            _mm_store_si128((__m128i *) &imgStruct.r[i], vdr);
            _mm_store_si128((__m128i *) &imgStruct.g[i], vdg);
            _mm_store_si128((__m128i *) &imgStruct.b[i], vdb);

        }

        for (int i = alignedWH; i < w * h; ++i) {
            imgStruct.r[i] /= rgba.r;
            imgStruct.g[i] /= rgba.g;
            imgStruct.b[i] /= rgba.b;
            //imgStruct.a[i]*=rgba.a;
        }
    }
}

__m128i __attribute__((always_inline)) WL::mulF(__m128i &v, __m128 constant) {


    /*__m128i lower16 = _mm_unpacklo_epi8(v,const00);
    __m128i higher16 = _mm_unpackhi_epi8(v,const00);
    __m256i lower16_256 = _mm256_cvtepu16_epi32(lower16);
    __m256i higher16_256 = _mm256_cvtepu16_epi32(higher16);
    __m256 lower16_256F = _mm256_cvtepi32_ps((lower16_256));
    __m256 higher16_256F = _mm256_cvtepi32_ps(higher16_256);
    lower16_256F = _mm256_mul_ps(lower16_256F, constant);
    higher16_256F = _mm256_mul_ps(higher16_256F, constant);
    lower16_256 = _mm256_cvtps_epi32(lower16_256F);
    higher16_256 = _mm256_cvtps_epi32(higher16_256F);*/

    __m128i lower16 = _mm_unpacklo_epi8(v,const00);
    __m128i higher16 = _mm_unpackhi_epi8(v,const00);
    __m128i lower32lo = _mm_unpacklo_epi16(lower16,const00);
    __m128i lower32hi = _mm_unpackhi_epi16(lower16,const00);
    __m128i higher32lo = _mm_unpacklo_epi16(higher16,const00);
    __m128i higher32hi = _mm_unpackhi_epi16(higher16,const00);
    __m128 lower32loF = _mm_cvtepi32_ps(lower32lo);
    __m128 lower32hiF = _mm_cvtepi32_ps(lower32hi);
    __m128 higher32loF = _mm_cvtepi32_ps(higher32lo);
    __m128 higher32hiF = _mm_cvtepi32_ps(higher32hi);

    lower32loF = _mm_mul_ps(lower32loF,constant);
    lower32hiF = _mm_mul_ps(lower32hiF,constant);
    lower32lo = _mm_cvtps_epi32(lower32loF);
    lower32hi = _mm_cvtps_epi32(lower32hiF);

    higher32loF = _mm_mul_ps(higher32loF,constant);
    higher32hiF = _mm_mul_ps(higher32hiF,constant);
    higher32lo = _mm_cvtps_epi32(higher32loF);
    higher32hi = _mm_cvtps_epi32(higher32hiF);

    lower16 = _mm_packus_epi32(lower32lo,lower32hi);
    higher16 = _mm_packus_epi32(higher32lo,higher32hi);

    v = _mm_packus_epi16(lower16,higher16);


    return v;
}

void WL::InvDivConstant(RGBAf rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            if(imgStruct.r[i]!=0)
                imgStruct.r[i]=rgba.r/imgStruct.r[i];
            if(imgStruct.g[i]!=0)
                imgStruct.g[i]=rgba.g/imgStruct.g[i];
            if(imgStruct.b[i]!=0)
                imgStruct.b[i]=rgba.b/imgStruct.b[i];
        }
    }
    else {
        const int alignedWH = w * h - w * h % 16;

        __m128 vb1 = _mm_set1_ps(rgba.r);
        __m128 vb2 = _mm_set1_ps(rgba.g);
        __m128 vb3 = _mm_set1_ps(rgba.b);

        uint8_t *debug = new(uint8_t[32]);
        __m256i constFF = _mm256_set1_epi16(0xFF);
        for (long long i = 0; i < alignedWH; i += 16) {

            __m128i vdr = _mm_load_si128((__m128i *) &imgStruct.r[i]);
            __m128i vdg = _mm_load_si128((__m128i *) &imgStruct.g[i]);
            __m128i vdb = _mm_load_si128((__m128i *) &imgStruct.b[i]);


            vdr = mulFInv(vdr, vb1);
            vdg = mulFInv(vdg, vb2);
            vdb = mulFInv(vdb, vb3);

            _mm_store_si128((__m128i *) &imgStruct.r[i], vdr);
            _mm_store_si128((__m128i *) &imgStruct.g[i], vdg);
            _mm_store_si128((__m128i *) &imgStruct.b[i], vdb);

        }

        for (int i = alignedWH; i < w * h; ++i) {
            /*if(imgStruct.r[i]!=0)
            imgStruct.r[i]=rgba.r/imgStruct.r[i];
            if(imgStruct.g[i]!=0)
            imgStruct.g[i]=rgba.g/imgStruct.g[i];
            if(imgStruct.b[i]!=0)
            imgStruct.b[i]=rgba.b/imgStruct.b[i];*/
        }
    }
}
__m128 const00f = _mm_set1_ps(0.0);
__m128i evilhack = _mm_set1_epi8(1);
__m128i __attribute__((always_inline, artificial,gnu_inline)) WL::mulFInv(__m128i &v, __m128 constant) {

    v = _mm_adds_epi8(evilhack,v);
    __m128i lower16 = _mm_unpacklo_epi8(v,const00);
    __m128i higher16 = _mm_unpackhi_epi8(v,const00);
    __m128i lower32lo = _mm_unpacklo_epi16(lower16,const00);
    __m128i lower32hi = _mm_unpackhi_epi16(lower16,const00);
    __m128i higher32lo = _mm_unpacklo_epi16(higher16,const00);
    __m128i higher32hi = _mm_unpackhi_epi16(higher16,const00);
    __m128 lower32loF = _mm_cvtepi32_ps(lower32lo);
    __m128 lower32hiF = _mm_cvtepi32_ps(lower32hi);
    __m128 higher32loF = _mm_cvtepi32_ps(higher32lo);
    __m128 higher32hiF = _mm_cvtepi32_ps(higher32hi);

    lower32loF = _mm_rcp_ps(lower32loF);
    lower32hiF = _mm_rcp_ps(lower32hiF);
    lower32loF = _mm_mul_ps(lower32loF,constant);
    lower32hiF = _mm_mul_ps(lower32hiF,constant);
    lower32lo = _mm_cvtps_epi32(lower32loF);
    lower32hi = _mm_cvtps_epi32(lower32hiF);
    lower16 = _mm_packus_epi32(lower32lo,lower32hi);

    higher32loF = _mm_rcp_ps(higher32loF);
    higher32hiF = _mm_rcp_ps(higher32hiF);
    higher32loF = _mm_mul_ps(higher32loF,constant);
    higher32hiF = _mm_mul_ps(higher32hiF,constant);
    higher32lo = _mm_cvtps_epi32(higher32loF);
    higher32hi = _mm_cvtps_epi32(higher32hiF);
    higher16 = _mm_packus_epi32(higher32lo,higher32hi);

    v = _mm_packus_epi16(lower16,higher16);
    return v;
}

void WL::PowConstant(RGBAf rgba, bool simd) {
    double MIN = 0;
    double MAX = 255;
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i] = (uint8_t)std::clamp(pow(imgStruct.r[i],rgba.r),MIN,MAX);
            imgStruct.g[i] = (uint8_t)std::clamp(pow(imgStruct.g[i],rgba.g),MIN,MAX);
            imgStruct.b[i] = (uint8_t)std::clamp(pow(imgStruct.b[i],rgba.b),MIN,MAX);
        }
    }
    else{
        const int alignedWH = w*h - w*h % 16;
        __m128 vb1 = _mm_set1_ps(rgba.r);
        __m128 vb2 = _mm_set1_ps(rgba.g);
        __m128 vb3 = _mm_set1_ps(rgba.b);

        uint8_t* debug = new (uint8_t[32]);
        __m256i constFF = _mm256_set1_epi16(0xFF);
        for (long long i = 0; i < alignedWH; i+=16) {

            __m128i vdr = _mm_load_si128((__m128i*)&imgStruct.r[i]);
            __m128i vdg = _mm_load_si128((__m128i*)&imgStruct.g[i]);
            __m128i vdb = _mm_load_si128((__m128i*)&imgStruct.b[i]);


            vdr = Pow(vdr,vb1);
            vdg = Pow(vdg,vb2);
            vdb = Pow(vdb,vb3);

            _mm_store_si128((__m128i *) &imgStruct.r[i], vdr);
            _mm_store_si128((__m128i *) &imgStruct.g[i], vdg);
            _mm_store_si128((__m128i *) &imgStruct.b[i], vdb);

        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i] = (uint8_t)std::clamp(pow(imgStruct.r[i],rgba.r),MIN,MAX);
            imgStruct.g[i] = (uint8_t)std::clamp(pow(imgStruct.g[i],rgba.g),MIN,MAX);
            imgStruct.b[i] = (uint8_t)std::clamp(pow(imgStruct.b[i],rgba.b),MIN,MAX);
        }
    }
}

__m128i __attribute__((always_inline)) WL::Pow(__m128i &v, __m128 constant) {
    __m128i lower16 = _mm_unpacklo_epi8(v,const00);
    __m128i higher16 = _mm_unpackhi_epi8(v,const00);
    __m128i lower32lo = _mm_unpacklo_epi16(lower16,const00);
    __m128i lower32hi = _mm_unpackhi_epi16(lower16,const00);
    __m128i higher32lo = _mm_unpacklo_epi16(higher16,const00);
    __m128i higher32hi = _mm_unpackhi_epi16(higher16,const00);

    __m128 lower32loF = _mm_cvtepi32_ps(lower32lo);
    lower32loF = exp2f4(_mm_mul_ps(log2f4(lower32loF), constant));

    __m128 lower32hiF = _mm_cvtepi32_ps(lower32hi);
    lower32hiF = exp2f4(_mm_mul_ps(log2f4(lower32hiF), constant));

    __m128 higher32loF = _mm_cvtepi32_ps(higher32lo);
    higher32loF = exp2f4(_mm_mul_ps(log2f4(higher32loF), constant));

    __m128 higher32hiF = _mm_cvtepi32_ps(higher32hi);
    higher32hiF = exp2f4(_mm_mul_ps(log2f4(higher32hiF), constant));

    lower32lo = _mm_cvtps_epi32(lower32loF);
    lower32hi = _mm_cvtps_epi32(lower32hiF);
    higher32lo = _mm_cvtps_epi32(higher32loF);
    higher32hi = _mm_cvtps_epi32(higher32hiF);

    lower16 = _mm_packus_epi32(lower32lo,lower32hi);
    higher16 = _mm_packus_epi32(higher32lo,higher32hi);

    v = _mm_packus_epi16(lower16,higher16);

    return v;
}
__m128i constExp = _mm_set1_epi32(0x7F800000);
__m128i constMant = _mm_set1_epi32(0x007FFFFF);
__m128i const127 = _mm_set1_epi32(127);
__m128  __attribute__((always_inline)) WL::log2f4(__m128 x) {
    union f4 index, p;

    __m128i exp = constExp;
    __m128i mant = constMant;

    __m128i i = _mm_castps_si128(x);

    __m128 e = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(i, exp), 23), const127));

    index.mi = _mm_srli_epi32(_mm_and_si128(i, mant), 23 - LOG2_TABLE_SIZE_LOG2);

    p.f[0] = log2_table[index.u[0]];
    p.f[1] = log2_table[index.u[1]];
    p.f[2] = log2_table[index.u[2]];
    p.f[3] = log2_table[index.u[3]];

    return _mm_add_ps(p.m, e);
    return x;
}

void WL::log2_init() {
    unsigned i;
    for (i = 0; i < LOG2_TABLE_SIZE; i++)
        log2_table[i] = (float) log2(1.0 + i * (1.0 / (LOG2_TABLE_SIZE-1)));
}
__m128 scaleLogConst = _mm_set1_ps(200/log2(255));
__m128i  __attribute__((always_inline)) WL::log2i(__m128i v) {
    __m128i lower16 = _mm_unpacklo_epi8(v,const00);
    __m128i higher16 = _mm_unpackhi_epi8(v,const00);
    __m128i lower32lo = _mm_unpacklo_epi16(lower16,const00);
    __m128i lower32hi = _mm_unpackhi_epi16(lower16,const00);
    __m128i higher32lo = _mm_unpacklo_epi16(higher16,const00);
    __m128i higher32hi = _mm_unpackhi_epi16(higher16,const00);

    __m128 lower32loF = _mm_cvtepi32_ps(lower32lo);
    lower32loF = log2f4(lower32loF);

    __m128 lower32hiF = _mm_cvtepi32_ps(lower32hi);
    lower32hiF = log2f4(lower32hiF);

    __m128 higher32loF = _mm_cvtepi32_ps(higher32lo);
    higher32loF = log2f4(higher32loF);

    __m128 higher32hiF = _mm_cvtepi32_ps(higher32hi);
    higher32hiF = log2f4(higher32hiF);

    if(onlylog){
        lower32loF = _mm_mul_ps(lower32loF, scaleLogConst);
        lower32hiF = _mm_mul_ps(lower32hiF, scaleLogConst);
        higher32loF = _mm_mul_ps(higher32loF, scaleLogConst);
        higher32hiF = _mm_mul_ps(higher32hiF, scaleLogConst);
    }

    lower32lo = _mm_cvtps_epi32(lower32loF);
    lower32hi = _mm_cvtps_epi32(lower32hiF);
    higher32lo = _mm_cvtps_epi32(higher32loF);
    higher32hi = _mm_cvtps_epi32(higher32hiF);

    lower16 = _mm_packus_epi32(lower32lo,lower32hi);
    higher16 = _mm_packus_epi32(higher32lo,higher32hi);

    v = _mm_packus_epi16(lower16,higher16);
    /*__m128i unpackedLowerlolo = _mm_unpacklo_epi32(unpackedLowerlo,const00);
    __m128i unpackedLowerlohi = _mm_unpackhi_epi32(unpackedLowerlo,const00);
    __m128i unpackedLowerhilo = _mm_unpacklo_epi32(unpackedLowerhi,const00);
    __m128i unpackedLowerhihi = _mm_unpackhi_epi32(unpackedLowerhi,const00);
    __m128i unpackedHigherlolo = _mm_unpacklo_epi32(unpackedHigherlo,const00);
    __m128i unpackedHigherlohi = _mm_unpackhi_epi32(unpackedHigherlo,const00);
    __m128i unpackedHigherhilo = _mm_unpacklo_epi32(unpackedHigherhi,const00);
    __m128i unpackedHigherhihi = _mm_unpackhi_epi32(unpackedHigherhi,const00);*/
    return v;

}

void WL::Log(bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i]=(uint8_t)log2((double)imgStruct.r[i]+1);
            imgStruct.g[i]=(uint8_t)log2((double)imgStruct.g[i]+1);
            imgStruct.b[i]=(uint8_t)log2((double)imgStruct.b[i]+1);
        }
    }
    else {
        const int alignedWH = w * h - w * h % 16;

        uint8_t *debug = new(uint8_t[32]);
        __m256i constFF = _mm256_set1_epi16(0xFF);
        for (long long i = 0; i < alignedWH; i += 16) {

            __m128i vdr = _mm_load_si128((__m128i *) &imgStruct.r[i]);
            __m128i vdg = _mm_load_si128((__m128i *) &imgStruct.g[i]);
            __m128i vdb = _mm_load_si128((__m128i *) &imgStruct.b[i]);

            onlylog = true;

            vdr = log2i(vdr);
            vdg = log2i(vdg);
            vdb = log2i(vdb);

            onlylog = false;



            _mm_store_si128((__m128i *) &imgStruct.r[i], vdr);
            _mm_store_si128((__m128i *) &imgStruct.g[i], vdg);
            _mm_store_si128((__m128i *) &imgStruct.b[i], vdb);

        }

        for (int i = alignedWH; i < w * h; ++i) {
            imgStruct.r[i]=(uint8_t)log2((double)imgStruct.r[i]+1);
            imgStruct.g[i]=(uint8_t)log2((double)imgStruct.g[i]+1);
            imgStruct.b[i]=(uint8_t)log2((double)imgStruct.b[i]+1);
        }
    }
}
__m128 const129 = _mm_set1_ps( 129.00000f);
__m128 const126 = _mm_set1_ps(-126.99999f);
__m128  __attribute__((always_inline)) WL::exp2f4(__m128 x) {
    __m128i ipart;
    __m128 fpart, expipart;
    union f4 index, expfpart;

    x = _mm_min_ps(x, const129);
    x = _mm_max_ps(x, const126);

    /* ipart = int(x) */
    ipart = _mm_cvtps_epi32(x);

    /* fpart = x - ipart */
    fpart = _mm_sub_ps(x, _mm_cvtepi32_ps(ipart));

    /* expipart = (float) (1 << ipart) */
    expipart = _mm_castsi128_ps(_mm_slli_epi32(_mm_add_epi32(ipart, const127), 23));

    /* index = EXP2_TABLE_OFFSET + (int)(fpart * EXP2_TABLE_SCALE) */
    index.mi = _mm_add_epi32(_mm_cvtps_epi32(_mm_mul_ps(fpart, _mm_set1_ps(EXP2_TABLE_SCALE))), _mm_set1_epi32(EXP2_TABLE_OFFSET));

    expfpart.f[0] = exp2_table[index.u[0]];
    expfpart.f[1] = exp2_table[index.u[1]];
    expfpart.f[2] = exp2_table[index.u[2]];
    expfpart.f[3] = exp2_table[index.u[3]];

    return _mm_mul_ps(expipart, expfpart.m);
}

void WL::exp2_init() {
    int i;
    for (i = 0; i < EXP2_TABLE_SIZE; i++)
        exp2_table[i] = (float) pow(2.0, (i - EXP2_TABLE_OFFSET) / EXP2_TABLE_SCALE);
}

void WL::MinConstant(RGBA rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i] = min(imgStruct.r[i],rgba.r);
            imgStruct.g[i] = min(imgStruct.g[i],rgba.g);
            imgStruct.b[i] = min(imgStruct.b[i],rgba.b);
        }
    }
    else{
        const int alignedWH = w*h - w*h % 32;
        __m256i vb1 = _mm256_set1_epi8(rgba.r);
        __m256i vb2 = _mm256_set1_epi8(rgba.g);
        __m256i vb3 = _mm256_set1_epi8(rgba.b);

        for (long long i = 0; i < alignedWH; i+=32) {
            __m256i va1 = _mm256_load_si256((__m256i*)(imgStruct.r+i));
            __m256i va2 = _mm256_load_si256((__m256i*)(imgStruct.g+i));
            __m256i va3 = _mm256_load_si256((__m256i*)(imgStruct.b+i));

            __m256i vd1 = _mm256_min_epu8(va1,vb1);
            __m256i vd2 = _mm256_min_epu8(va2,vb2);
            __m256i vd3 = _mm256_min_epu8(va3,vb3);

            _mm256_store_si256((__m256i*)(imgStruct.r + i), vd1);
            _mm256_store_si256((__m256i*)(imgStruct.g + i), vd2);
            _mm256_store_si256((__m256i*)(imgStruct.b + i), vd3);

        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i] = min(imgStruct.r[i],rgba.r);
            imgStruct.g[i] = min(imgStruct.g[i],rgba.g);
            imgStruct.b[i] = min(imgStruct.b[i],rgba.b);
        }
    }
}

void WL::MaxConstant(RGBA rgba, bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i] = max(imgStruct.r[i],rgba.r);
            imgStruct.g[i] = max(imgStruct.g[i],rgba.g);
            imgStruct.b[i] = max(imgStruct.b[i],rgba.b);
        }
    }
    else{
        const int alignedWH = w*h - w*h % 32;
        __m256i vb1 = _mm256_set1_epi8(rgba.r);
        __m256i vb2 = _mm256_set1_epi8(rgba.g);
        __m256i vb3 = _mm256_set1_epi8(rgba.b);

        for (long long i = 0; i < alignedWH; i+=32) {
            __m256i va1 = _mm256_load_si256((__m256i*)(imgStruct.r+i));
            __m256i va2 = _mm256_load_si256((__m256i*)(imgStruct.g+i));
            __m256i va3 = _mm256_load_si256((__m256i*)(imgStruct.b+i));

            __m256i vd1 = _mm256_max_epu8(va1,vb1);
            __m256i vd2 = _mm256_max_epu8(va2,vb2);
            __m256i vd3 = _mm256_max_epu8(va3,vb3);

            _mm256_store_si256((__m256i*)(imgStruct.r + i), vd1);
            _mm256_store_si256((__m256i*)(imgStruct.g + i), vd2);
            _mm256_store_si256((__m256i*)(imgStruct.b + i), vd3);

        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i] = min(imgStruct.r[i],rgba.r);
            imgStruct.g[i] = min(imgStruct.g[i],rgba.g);
            imgStruct.b[i] = min(imgStruct.b[i],rgba.b);
        }
    }
}

void WL::Grayscale(bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            uint8_t temp =(uint8_t)(((int)imgStruct.r[i]+(int)imgStruct.g[i]+(int)imgStruct.b[i])/3);
            imgStruct.r[i] = temp;
            imgStruct.g[i] = temp;
            imgStruct.b[i] = temp;
        }
    }
    else{
        const int alignedWH = w * h - w * h % 32;

        __m128 vb = _mm_set1_ps(1./3);

        uint8_t *debug = new(uint8_t[32]);
        __m256i constFF = _mm256_set1_epi16(0xFF);
        for (long long i = 0; i < alignedWH; i += 32) {
            __m256i va1 = _mm256_load_si256((__m256i*)(imgStruct.r+i));
            __m256i va2 = _mm256_load_si256((__m256i*)(imgStruct.g+i));
            __m256i va3 = _mm256_load_si256((__m256i*)(imgStruct.b+i));

            __m128i va1lo = _mm256_castsi256_si128(va1);
            __m128i va2lo = _mm256_castsi256_si128(va2);
            __m128i va3lo = _mm256_castsi256_si128(va3);
            __m128i va1hi = _mm256_extractf128_si256(va1,1);
            __m128i va2hi = _mm256_extractf128_si256(va2,1);
            __m128i va3hi = _mm256_extractf128_si256(va3,1);

            __m128i vd1lo = mulF(va1lo, vb);
            __m128i vd1hi = mulF(va1hi, vb);
            __m128i vd2lo = mulF(va2lo, vb);
            __m128i vd2hi = mulF(va2hi, vb);
            __m128i vd3lo = mulF(va3lo, vb);
            __m128i vd3hi = mulF(va3hi, vb);

            va1 = _mm256_castsi128_si256(vd1lo);
            va1 = _mm256_insertf128_si256(va1,vd1hi,1);

            va2 = _mm256_castsi128_si256(vd2lo);
            va2 = _mm256_insertf128_si256(va2,vd2hi,1);

            va3 = _mm256_castsi128_si256(vd3lo);
            va3 = _mm256_insertf128_si256(va3,vd3hi,1);

            __m256i vd = _mm256_add_epi8(_mm256_add_epi8(va1,va2),va3);

            _mm256_store_si256((__m256i*)(imgStruct.r + i), vd);
            _mm256_store_si256((__m256i*)(imgStruct.g + i), vd);
            _mm256_store_si256((__m256i*)(imgStruct.b + i), vd);

            /*__m128i vdr = _mm_load_si128((__m128i *) &imgStruct.r[i]);
            __m128i vdg = _mm_load_si128((__m128i *) &imgStruct.g[i]);
            __m128i vdb = _mm_load_si128((__m128i *) &imgStruct.b[i]);


            vdr = mulF(vdr, vb);
            vdg = mulF(vdg, vb);
            vdb = mulF(vdb, vb);
            __m128i vd = _mm_add_epi8(_mm_add_epi8(vdr,vdg),vdb);

            _mm_store_si128((__m128i *) &imgStruct.r[i], vd);
            _mm_store_si128((__m128i *) &imgStruct.g[i], vd);
            _mm_store_si128((__m128i *) &imgStruct.b[i], vd);*/

        }
        for (int i = alignedWH; i < w*h; ++i) {
            uint8_t temp =(uint8_t)(((int)imgStruct.r[i]+(int)imgStruct.g[i]+(int)imgStruct.b[i])/3);
            imgStruct.r[i] = temp;
            imgStruct.g[i] = temp;
            imgStruct.b[i] = temp;
        }
    }
}

void WL::Inverse(bool simd) {
    InvSubConstant(RGBA{255,255,255,0},simd);
}

void WL::Abs(bool simd) {
    if(!simd){
        for(int i = 0; i < w * h; i++) {
            imgStruct.r[i] = abs((int8_t)imgStruct.r[i]);
            imgStruct.g[i] = abs((int8_t)imgStruct.g[i]);
            imgStruct.b[i] = abs((int8_t)imgStruct.b[i]);
        }
    }
    else{
        const int alignedWH = w*h - w*h % 32;

        for (long long i = 0; i < alignedWH; i+=32) {
            __m256i va1 = _mm256_load_si256((__m256i*)(imgStruct.r+i));
            __m256i va2 = _mm256_load_si256((__m256i*)(imgStruct.g+i));
            __m256i va3 = _mm256_load_si256((__m256i*)(imgStruct.b+i));

            __m256i vd1 = _mm256_abs_epi8(va1);
            __m256i vd2 = _mm256_abs_epi8(va2);
            __m256i vd3 = _mm256_abs_epi8(va3);

            _mm256_store_si256((__m256i*)(imgStruct.r + i), vd1);
            _mm256_store_si256((__m256i*)(imgStruct.g + i), vd2);
            _mm256_store_si256((__m256i*)(imgStruct.b + i), vd3);

        }
        for (int i = alignedWH; i < w*h; ++i) {
            imgStruct.r[i] = (int8_t)imgStruct.r[i];
            imgStruct.g[i] = (int8_t)imgStruct.g[i];
            imgStruct.b[i] = (int8_t)imgStruct.b[i];
        }
    }
}



void WL::Sobel(bool cacheOpt, int N) {
    int rows = h;
    int columns = w;
    Grayscale(true);
    uint8_t* image = new uint8_t[w*h];
    for(int i = 0; i < w*h; i ++){
        image[i] = imgStruct.r[i];
    }
    //uint8_t* edges = new uint8_t[w*h];
    if(!cacheOpt) {
        int GX[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        int GY[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

        for (int row = N/2; row < (rows - N/2); ++row) {
            for (int column = N/2; column < (columns - N/2); ++column) {
                double gx = 0;
                double gy = 0;

                for (int i = 0; i < N; ++i) {
                    for (int j = 0; j < N; ++j) {
                        int image_row = row + i - N/2;
                        int image_column = column + j - N/2;

                        double image_value = image[image_row * columns + image_column];

                        int kernel_index = i * N + j;

                        gx += image_value * GX[kernel_index];
                        gy += image_value * GY[kernel_index];
                    }
                }

                imgStruct.r[row * columns + column] =
                imgStruct.g[row * columns + column] =
                imgStruct.b[row * columns + column] =
                        sqrt(gx * gx + gy * gy);
            }
        }
    }
    else {
        int threadNum = std::thread::hardware_concurrency();
        vector<thread> threads;
        int chunk_size = (rows - N) / threadNum;
        int stride = 64;
        int GX[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        int GY[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
        for (int i = 0; i < threadNum; ++i)
        {
            int start,end;
            if(i==0){
                start = i * chunk_size + N/2;
                end = (i + 1) * chunk_size;
            }
            else if(i == (threadNum-1)){
                start = i * chunk_size;
                end = (i + 1) * chunk_size - N/2;
            }
            else{
                start = i * chunk_size;
                end = (i + 1) * chunk_size;
            }

            threads.emplace_back([start, end, N]()
                                 { sobelThread(start,end, N);});
        }
        for(int i = 0; i < threads.size(); i++){
            threads[i].join();
        }
    }
    //WriteImagePng("../outputSobel.png");
}

void WL::sobelThread(int start, int end, int N){
    int rows = h;
    int columns = w;
    int GX[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int GY[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    uint8_t* image = new uint8_t[w*h];
    for(int i = 0; i < w*h; i ++){
        image[i] = imgStruct.r[i];
    }
    for (int row = start; row <= end; ++row) {
        for (int column = N / 2; column < (columns - N / 2); ++column) {
            double gx = 0;
            double gy = 0;

            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    int image_row = row + i - N / 2;
                    int image_column = column + j - N / 2;

                    double image_value = image[image_row * columns + image_column];

                    int kernel_index = i * N + j;

                    gx += image_value * GX[kernel_index];
                    gy += image_value * GY[kernel_index];
                }
            }

            imgStruct.r[row * columns + column] =
            imgStruct.g[row * columns + column] =
            imgStruct.b[row * columns + column] =
                    sqrt(gx * gx + gy * gy);
        }
    }
}













int get_cache_line_size ( int id ) {
    // Variables to hold the contents of the 4 i386 legacy registers
    uint32_t eax = 0x8000001D; // get cache info
    uint32_t ebx;
    uint32_t ecx = id; // cache id
    uint32_t edx;

    // generates output in 4 registers eax, ebx, ecx and edx
    __asm__ (
    "cpuid" // call i386 cpuid instruction
    :   "+a" (eax), // contains the cpuid command code, 4 for cache query
    "=b" (ebx),
    "+c" (ecx), // contains the cache id
    "=d" (edx)
    );

    // See the page 3-191 of the manual.
    int cache_type = eax & 0x1F;

    // end of valid cache identifiers
    /*if ( cache_type == 0 ) {
        return -1;
    }*/


    return (ebx & 0x000B) + 1;
}

