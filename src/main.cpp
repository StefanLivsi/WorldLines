#include <iostream>
#include <chrono>
#include <iomanip>
#include "../h/WL.h"
#include "../h/_Timer.h"
#include <cmath>
#include <vector>
#include <string>
using namespace std::chrono;
using namespace std;
void Benchmark(int cnt);

int main(int argc, char* argv[]) { //argv[0] ime, //argv[1] slika // argv[2..N-1] opcije
    WL::log2_init();
    WL::exp2_init();

    if(argv[1][strlen(argv[1])-3]=='p')
    {
        WL::png=true;
    }
    else{
        WL::png = false;
    }
    if (argc > 1) {
        int w, h, comp;
        int res = WL::LoadImage(argv[1],w,h,comp,0);
        if(res < 0 ){
            std::cout << "Error loading image";
            return -1;
        }
        // Loop through the arguments
        int i = 2;
        bool simd = true;
        while(i < argc){
            uint8_t farg = 0;
            float fargf = 0;
            if(!strcmp(argv[i],"-a") || !strcmp(argv[i],"--add")) {//add
                farg = atoi(argv[++i]);
                WL::addConstant(RGBA{farg},simd);
            }else if(!strcmp(argv[i],"-s") || !strcmp(argv[i],"--sub")){//sub
                farg = atoi(argv[++i]);
                WL::SubConstant(RGBA{farg},simd);
            }else if(!strcmp(argv[i],"-i") || !strcmp(argv[i],"--isub")){//invSub
                farg = atoi(argv[++i]);
                WL::InvSubConstant(RGBA{farg},simd);
            }else if(!strcmp(argv[i],"-m") || !strcmp(argv[i],"--mul")){//mul
                farg = atoi(argv[++i]);
                WL::MulConstant(RGBA{farg},simd);
            }else if(!strcmp(argv[i],"-d") || !strcmp(argv[i],"--div")){//div
                fargf = stof(argv[++i]);
                WL::DivConstant(RGBAf{fargf},simd);
            }else if(!strcmp(argv[i],"-n") || !strcmp(argv[i],"--idiv")){//invdiv
                fargf = stof(argv[++i]);
                WL::InvDivConstant(RGBAf{fargf},simd);
            }else if(!strcmp(argv[i],"-p") || !strcmp(argv[i],"--pow")){//pow
                fargf = stof(argv[++i]);
                WL::PowConstant(RGBAf{fargf},simd);
            }else if(!strcmp(argv[i],"-l") || !strcmp(argv[i],"--log")){//log
                WL::Log(simd);
            }else if(!strcmp(argv[i],"-x") || !strcmp(argv[i],"--max")){//max
                farg = atoi(argv[++i]);
                WL::MaxConstant(RGBA{farg},simd);
            }else if(!strcmp(argv[i],"-z") || !strcmp(argv[i],"--min")){//min
                farg = atoi(argv[++i]);
                WL::MinConstant(RGBA{farg},simd);
            }else if(!strcmp(argv[i],"-b") || !strcmp(argv[i],"--abs")){//abs
                WL::Abs(simd);
            }else if(!strcmp(argv[i],"-v") || !strcmp(argv[i],"--invert")){//invert
                WL::Inverse(simd);
            }else if(!strcmp(argv[i],"-g") || !strcmp(argv[i],"--grayscale")){//grayscale
                WL::Grayscale(simd);
            }else if(!strcmp(argv[i],"-f") || !strcmp(argv[i],"--filter")){//filter
                farg = atoi(argv[++i]);
                WL::Sobel(simd, farg);
            }else if(!strcmp(argv[i],"-q") || !strcmp(argv[i],"--test")){//test
                farg = atoi(argv[++i]);
                Benchmark(farg);
            }else if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help")){//help
                cout << "format: WorldLines.exe [jpg/png file path] [options]\n"
                        "options:\n"
                        "-a --add\t [arg] increase pixels value by arg\n"
                        "-s --sub\t [arg] decrease pixels value by arg\n"
                        "-i --isub\t [arg] set pixel value to arg - pixelVal\n"
                        "-m --mul\t [arg] multiply pixel value by int arg\n"
                        "-d --div\t [arg] divide pixel value by arg\n"
                        "-n --idiv\t [arg] set pixel value to arg/pixel\n"
                        "-p --pow\t [arg] set pixel value to pixel to the power of float arg\n"
                        "-l --log\t       set pixel value to log2 of pixel\n"
                        "-x --max\t [arg] set pixel value to max(pixel,arg)\n"
                        "-z --min\t [arg] set pixel value to min(pixel,arg)\n"
                        "-b --abs\t       treat pixel value as signed and abs it\n"
                        "-v --invert\t       invert the pixel value\n"
                        "-g --grayscale\t       make pixels shades of gray\n"
                        "-f --filter\t [arg] apply Sobel's filter on pixels with arg N\n"
                        "-q --test\t [arg] benchmark all non load/save functions, arg number of iterations\n";
            }else if(!strcmp(argv[i],"-w") || !strcmp(argv[i],"--simd")){//enable simd
                farg = atoi(argv[++i]);
                simd = farg == 1;
            }
            i++;
        }

    } else {
        std::cout << "No arguments provided." << std::endl;
    }








    /*int w, h, comp;
    int res = WL::LoadImage("../world.png",w,h,comp,0);
    if(res < 0 ){
        std::cout << "Error loading image";
        return -1;
    }

    StartTimer("No Opt Sobel")
        //WL::Sobel(false,3);
    EndTimer(false);
    StartTimer("Opt Sobel")
        WL::Sobel(true,3);
    EndTimer(true);
    cout << "\t" << "No Opt Sobel:" << "\t" << size_t(relative[0]) << "ns\n";
    cout << "\t" << "Opt Sobel:     " << "\t" << size_t(relative[1]) << "ns\n";
    cout << "\tRelative: \t" << round( relative[0] / relative[1] * 100.0) / 100.0 << "x\n";*/



    if(WL::png)
        WL::WriteImagePng("../output.png");
    else
        WL::WriteImagePng("../output.jpg");

    return 0;
}
//0 nosimd, 1 simd u paru
vector<pair<double,double>> benchVals[14];
pair<double,double> average[14];
const char * operations[14]={
        "Add Constant:",
        "Sub Constant:",
        "Inv Sub Constant:",
        "Mul Constant:",
        "Div Constant:",
        "Inv Div Constant:",
        "Pow Constant:",
        "Log:",
        "Abs:",
        "Min Constant:",
        "Max Constant:",
        "Inverse:",
        "Grayscale:",
        "Sobel:"

};
void Benchmark(int cnt){
    int i = 0;
    while(cnt--) {
        i = 0;

        StartTimer("No SIMD")
            WL::addConstant(RGBA{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::addConstant(RGBA{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::SubConstant(RGBA{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::SubConstant(RGBA{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::InvSubConstant(RGBA{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::InvSubConstant(RGBA{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::MulConstant(RGBA{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::MulConstant(RGBA{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::DivConstant(RGBAf{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::DivConstant(RGBAf{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::InvDivConstant(RGBAf{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::InvDivConstant(RGBAf{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::PowConstant(RGBAf{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::PowConstant(RGBAf{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::Log(false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::Log(true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::Abs(false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::Abs(true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::MinConstant(RGBA{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::MinConstant(RGBA{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::MaxConstant(RGBA{1, 1, 1, 1}, false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::MaxConstant(RGBA{1, 1, 1, 1}, true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::Inverse(false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::Inverse(true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));



        StartTimer("No SIMD")
            WL::Grayscale(false);
        EndTimer(false)
        StartTimer("SIMD")
            WL::Grayscale(true);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));

        StartTimer("No MT")
            WL::Sobel(false, 3);
        EndTimer(false)
        StartTimer("MT")
            WL::Sobel(true, 3);
        EndTimer(true)
        benchVals[i++].emplace_back(pair(relative[0], relative[1]));

    }
    for(int j = 0; j < 14; j++){
        double totalNosimd = 0;
        double totalSimd = 0;
        for(int k = 0; k < benchVals[j].size(); k++){
            totalNosimd += benchVals[j][k].first;
            totalSimd += benchVals[j][k].second;
        }
        totalNosimd /= benchVals[j].size();
        totalSimd /= benchVals[j].size();
        cout << operations[j] << '\n';
        cout << "\t" << "No Simd:" << "\t" << size_t(round(totalNosimd)) << "ns\n";
        cout << "\t" << "Simd:     " << "\t" << size_t(round(totalSimd)) << "ns\n";
        cout << "\tRelative: \t" << round( totalNosimd / totalSimd * 100.0) / 100.0 << "x\n";
    }
}

