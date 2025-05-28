#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <deque>
//#include "BitIoStream.hpp"
#include "WordBasedText.h"

using namespace std;

class BackwardEncoder {
public:
    BackwardEncoder(WordBasedText*);
    BackwardEncoder(int);
    BackwardEncoder(WordBasedText*,vector<uint32_t>);
    vector<uint32_t> get_dic(){return dic;};
    void checkDecode();
    void checkDecodeChar(WordBasedText*);
    virtual void serial(string){};
    void serializeChar(uint8_t*);
    void loadChar(uint8_t*);
protected:
    int n; // text length in characters
    unsigned char* outChar;
    uint32_t* codeStream;
    WordBasedText* text;
    char* inputText;
    unsigned NYT;
    vector<uint32_t> out;
    vector<uint32_t> dic;
    ~BackwardEncoder(){
        delete[] inputText;
        };
};

// Our method
class AdaptiveN : public BackwardEncoder {
protected:
    vector<int> m;
    vector<int> rev;
    vector<int> ind;
    vector<int> f,f1;
    UniversalCode* C;
    vector<uint8_t> bestDigitsSz;
    int j;
    const static int max_var_digit=12,init_j=26;
    void update1(int);
    void update_code(uint32_t);
    void update_decode(uint32_t);
    void update_code_char(uint32_t);
    void update_decode_char(uint32_t);
    void precalcBestBlocks();
    void BCS(uint8_t[], uint32_t, uint64_t, uint64_t, uint64_t, uint64_t);
    const static int max_digit=40;
    int bestFullBitsSz = 10000000,bestBlocksNum;
    static const int sigma = 256;
public:
    int mc=0;
    uint32_t* PSum;
    uint64_t  getSum(uint32_t, uint32_t);
    double rate=5;
    AdaptiveN(WordBasedText*,UniversalCode*);
    AdaptiveN(UniversalCode*,int);
    AdaptiveN(WordBasedText*,UniversalCode*,vector<uint32_t>);
    double encode();
    double encode1();
    double encode2();
    double encode2_char();
    void print_code();
    void update();
    int decode();
    int decode1();
    int decode2();
    ~AdaptiveN(){delete[] outChar;};
};

class AdaptiveNDecode: public AdaptiveN {
private:
    int n;
public:
    int decode2_char();
    AdaptiveNDecode(int,UniversalCode*);
    ~AdaptiveNDecode(){delete[] outChar;};
};

typedef struct {
    unsigned up,      // next node up the tree
        down,         // pair of down nodes
        symbol,       // node symbol value
        weight;       // node weight
} HTable;

typedef struct {
    unsigned esc,     // the current tree height
        root,         // the root of the tree
        size,         // the alphabet size
        *map;         // mapping for symbols to nodes
    HTable table[1];  // the coding table starts here
} HCoder;

// Classic Vitter algorithm
class Vitter: public BackwardEncoder {
protected:
    FILE *In, *Out;
    unsigned char ArcBit = 0;
    uint32_t ArcChar = 0;
    HCoder *huff;
    WordBasedText* text;
    int len=0;
    const int size = (1<<24);
    void update(string);
    HCoder *huff_init (unsigned size, unsigned root);
    void arc_put1 (unsigned bit);
    unsigned huff_split (HCoder *, unsigned);
    unsigned huff_leader (HCoder *, unsigned);
    unsigned huff_slide (HCoder *, unsigned);
    void huff_increment (HCoder *, unsigned);
    void huff_scale (HCoder *, unsigned);
    void huff_sendid (HCoder *, unsigned);
    void huff_encode (HCoder *, unsigned);
public:
    Vitter(WordBasedText*);
    Vitter(int);
    ~Vitter(){delete[] huff;};
    double encode();
    int encode_char();
};

class VitterDecode: public Vitter {
public:
    VitterDecode(int);
    ~VitterDecode(){delete[] outChar;};;
    unsigned huff_decode (HCoder*);
    void decode_char();
private:
    unsigned arc_get1();
    unsigned huff_readid (HCoder*);
    uint32_t outPos,inPos;
};

// Modified Vitter algorithm
class VitterM : public BackwardEncoder {
private:
    FILE *In, *Out;
    unsigned char ArcBit = 0;
    int ArcChar = 0;
    HCoder *huff;
    int len=0,mc=0;
    const int size = (1<<24);
    void update(string);
    HCoder *huff_init (unsigned size, unsigned root);
    void arc_put1 (unsigned bit);
    unsigned arc_get1 ();
    unsigned huff_split (HCoder *, unsigned);
    unsigned huff_leader (HCoder *, unsigned);
    unsigned huff_slide (HCoder *, unsigned);
    void huff_increment (HCoder *, unsigned);
    void huff_sendid (HCoder *, unsigned);
    void huff_encode (HCoder *, unsigned);
    unsigned huff_decode (HCoder *);
    unsigned huff_readid (HCoder*);
    unsigned char *buffer;
public:
    VitterM(WordBasedText*);
    double encode();
    unsigned decode ();
    int serialize(uint8_t*);
    void load(uint8_t*,int);
    ~VitterM(){delete huff;};
};


class Gagie : public BackwardEncoder {
private:
protected:
    char code_type; // 0 - Huffman, 1 - Huffman smoothed, 2 - Shannon, 3 - Shannon smoothed
    uint32_t *freqs;
    UniversalCode* C;
    uint8_t bufferShift;
    uint32_t streamPos;
    uint64_t buffer;
    void BlockEncode(int);
    static const int sigma = 256;
    uint32_t inPos;
public:
    Gagie(WordBasedText*,char);
    Gagie(int text_len):BackwardEncoder(text_len){};
    int encode_char();
    int encode_char1();
    void encode_word();
    double entropy();
    ~Gagie();
};

class GagieDecode : public Gagie {
private:
    void BlockDecode(int);
public:
    GagieDecode(int,int);
    void decode_char();
    virtual void decode_char1();
    double entropy();
    ~GagieDecode(){
        delete[] outChar;
     /*   delete[] freqs;
        delete[] codeStream;
        delete C;*/
    };
protected:
    uint32_t outPos;
};

class CanonicalDecode : public GagieDecode {
private:
    void BlockDecode(int);
    CHuffman* C;
public:
    CanonicalDecode(int a,int b):GagieDecode(a,b){};
    //void decode_char();
    void decode_char1();
    void decode_char();
    //double entropy();
    ~CanonicalDecode(){};
};

void print_vector(vector<uint32_t>,string);
void print_vector(uint32_t*,uint32_t);
