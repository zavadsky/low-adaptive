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

uint8_t buffer1[25000000];

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
        char intervals;
        if(smoothed<6)
            intervals = argv[4][0];
        std::ofstream out(ifname+".enc", std::ios::binary);
        WordBasedText *wa;
        VitterDecode *decv;
        int vs,vms,alg2s,alg4s,gs;             // Size of the text encoded with different algorithms
        vector<uint8_t> bmix_code{4,2,2,2,1,2}; // BCMix code for Alg. 2
        double vt=0,vmt=0,alg2t=0,alg4t=0,gt=0;  // Encoding/decoding time
        wa = new WordBasedText(ifname,1);   // Pre-process the text

        cout<<endl<<"================ Encoding. ================== iterations="<<iter<<endl;
        for(int i=0;i<iter;i++) {
                // Gagie 2022
                Gagie *g = new Gagie(wa,smoothed);
                if(smoothed==6) {
                    Vitter* v = new Vitter(wa);
                    measure_start();
                    gs = v->encode_char();
                    measure_end();
                    if(i==iter-1)
                        v->serializeChar(buffer1);
                    delete v;
                } else {
                    measure_start();
                    if(intervals=='f')
                        gs = g->encode_char(); // encode_char - Fixed Blocks
                    else
                        gs = g->encode_char1(); // encode_char1 - Variable Length Blocks
                    measure_end();
                    g->serializeChar(buffer1);
                    delete g;
                }
                gt += m_time;
                cout<<".";
            }
        gt/=iter;
        cout<<endl<<"Encoding time= "<<gt<<endl;
        cout<<endl<<"Encoded size= "<<gs<<" ("<<(float)gs*8/wa->Nchar<<" bits per symbol)"<<endl;

        cout<<endl<<"================ Decoding. ================== iterations="<<iter<<endl;
        vt=vmt=alg2t=alg4t=gt=0;
        for(int k=0;k<iter;k++) {
                GagieDecode* g;
                measure_start();
                if(smoothed==6) {
                    decv = new VitterDecode(wa->Nchar);
                    decv->loadChar(buffer1);
                    decv->decode_char();
                } else {
                    if(smoothed>1 && smoothed<4)
                        g = new CanonicalDecode(wa->Nchar,smoothed);
                    else
                        g = new GagieDecode(wa->Nchar,smoothed);
                    g->loadChar(buffer1);
                    if(intervals=='f')
                        g->decode_char();       // decode_char - Fixed Blocks
                    else
                        g->decode_char1();       // decode_char1 - Variable Length Blocks
                }
                measure_end();
                gt += m_time;
                if(smoothed == 6) {
                    if(k==0) decv->checkDecodeChar(wa);
                    delete decv;
                } else {
                    if(k==0) g->checkDecodeChar(wa);
                    delete g;
                }
                cout<<".";
        }
        gt/=iter;
        cout<<endl<<"Decoding time= "<<gt<<endl;
    }
	system("pause");
}

