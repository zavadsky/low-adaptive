#include <string.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <windows.h>
#include "include\UniversalCode.h"
#include "include\WordBasedText.h"
#include "include\Adaptive.hpp"

#pragma GCC diagnostic push

double m_time;
LARGE_INTEGER start, finish, freq;

uint8_t buffer1[25000000],buffer2[25000000],buffer3[25000000];

#define measure_start()  \
	QueryPerformanceFrequency(&freq); \
	QueryPerformanceCounter(&start);

#define measure_end() QueryPerformanceCounter(&finish); \
					m_time = ((finish.QuadPart - start.QuadPart) / (double)freq.QuadPart);

using namespace std;

int main(int argc, char** argv) {
    if(argc<2)
        cout<<"Incorrect number of command line arguments.";
    else {
        static const int sigma = 256;
        string ifname(argv[1]);
        int iter = atoi(argv[2]);
        int smoothed = atoi(argv[3]); // 0 - Huffman, 1 - Huffman smoothed, 2 - Huffman canonical, 3 - Huffman canonical smoothed, 4 - Shannon, 5 - Shannon smoothed
        char intervals = argv[4][0];
        std::ofstream out(ifname+".enc", std::ios::binary);
        WordBasedText *wa;
        BCMix *bmix,*bmix1;
        AdaptiveN *encmix,*encmix1;
        VitterM *encvm;
        Vitter *encv;
        int vs,vms,alg2s,alg4s,gs;             // Size of the text encoded with different algorithms
        vector<uint8_t> bmix_code{4,2,2,2,1,2}; // BCMix code for Alg. 2
        double vt=0,vmt=0,alg2t=0,alg4t=0,gt=0;  // Encoding/decoding time
        wa = new WordBasedText(ifname,1);   // Pre-process the text

        cout<<endl<<"================ Encoding. ================== iterations="<<iter<<endl;
        for(int i=0;i<iter;i++) {
                // Gagie 2022
                Gagie *g = new Gagie(wa,smoothed);
                measure_start();
                if(intervals=='f')
                    gs = g->encode_char(); // encode_char - Fixed Blocks
                else
                    gs = g->encode_char1(); // encode_char1 - Variable Length Blocks
                measure_end();
                gt += m_time;
                if(i==iter-1)
                    g->serializeChar(buffer1);
                delete g;
                cout<<".";
            }
        gt/=iter;
        cout<<endl<<"Encoding time= "<<gt<<endl;
        cout<<endl<<"Encoded size= "<<gs<<endl;
        int text_len = wa->Nchar;

        cout<<endl<<"================ Decoding. ================== iterations="<<iter<<endl;
        vt=vmt=alg2t=alg4t=gt=0;
        for(int k=0;k<iter;k++) {
                GagieDecode* g;
                if(smoothed>1 && smoothed<4)
                    g = new CanonicalDecode(text_len,smoothed);
                else
                    g = new GagieDecode(text_len,smoothed);
                g->loadChar(buffer1);
                measure_start();
                if(intervals=='f')
                    g->decode_char();       // decode_char - Fixed Blocks
                else
                    g->decode_char1();       // decode_char1 - Variable Length Blocks
                measure_end();
                gt += m_time;
                if(k==0) g->checkDecodeChar(wa);
                cout<<".";
                delete g;
        }
        gt/=iter;
        cout<<endl<<"Decoding time= "<<gt<<endl;
    }
	system("pause");
}

