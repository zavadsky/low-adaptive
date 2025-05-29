#include <string.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <windows.h>
#include "include\UniversalCode.h"
#include "include\CharBasedText.h"
#include "include\Adaptive.hpp"

#pragma GCC diagnostic push

double m_time;
LARGE_INTEGER start, finish, freq;

uint8_t buffer1[25000000]; // store the results of encoding

// Time measurement defined for Windows
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
        string ifname(argv[1]);
        int iter = atoi(argv[2]);
        int smoothed = atoi(argv[3]); // 0 - Huffman, 1 - Huffman smoothed, 2 - Huffman canonical, 3 - Huffman canonical smoothed, 4 - Shannon, 5 - Shannon smoothed
        char intervals;
        if(smoothed<6)
            intervals = argv[4][0]; // v/f - variable/fixed length intervals of code update
        CharBasedText *wa = new CharBasedText(ifname);   // Pre-process the text;
        int gs;             // Size of the text encoded with different algorithms
        double gt=0,ent;  // Encoding/decoding time; Entropy H0
        for(int i=0;i<iter;i++) {
                Gagie *g = new Gagie(wa,smoothed);                // Gagie 2022
                if(i==0)
                    cout<<endl<<"=========== Encoding. ============ iterations="<<iter<<endl;
                if(smoothed==6) {
                    Vitter* v = new Vitter(wa);
                    measure_start();
                    gs = v->encode_char();
                    measure_end();
                    if(i==iter-1)   // Store the result of the last iteration for decoding
                        v->serializeChar(buffer1);
                    delete v;
                } else {
                    measure_start();
                    if(intervals=='f')
                        gs = g->encode_char(); // encode_char - Fixed Blocks
                    else
                        gs = g->encode_char1(); // encode_char1 - Variable Length Blocks
                    measure_end();
                    if(i==iter-1) {// Store the result of the last iteration for decoding
                        g->serializeChar(buffer1);
                        ent = (double)g->entropy();
                    }
                    delete g;
                }
                gt += m_time;
                cout<<".";
            }
        gt/=iter;
        cout<<endl<<"Encoding time= "<<gt<<endl;
        cout<<endl<<"Encoded size= "<<gs<<" ("<<(float)gs*8/wa->Nchar<<" bits per symbol)."<<endl;
        if(smoothed<6) printf("Entropy=%.0f (%f bits per symbol).\n",ent,(float)ent*8/wa->Nchar);
        VitterDecode *decv; // Class for Vitter's code decoding
        cout<<endl<<"================ Decoding. ================== iterations="<<iter<<endl;
        gt=0;
        for(int k=0;k<iter;k++) {
                GagieDecode* g;
                if(smoothed==6) {
                    decv = new VitterDecode(wa->Nchar);
                    decv->loadChar(buffer1);    // Load the encoded bitstream from memory
                    measure_start();
                    decv->decode_char();
                } else {
                    if(smoothed>1 && smoothed<4)
                        g = new CanonicalDecode(wa->Nchar,smoothed); // canonical Huffman
                    else
                        g = new GagieDecode(wa->Nchar,smoothed);    // non-canonical Huffman or Shannon
                    g->loadChar(buffer1);       // Load the encoded bitstream from memory
                    measure_start();
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

