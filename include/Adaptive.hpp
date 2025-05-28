#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <deque>
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
    ~VitterDecode(){
        delete[] outChar;
    };
    unsigned huff_decode (HCoder*);
    void decode_char();
private:
    unsigned arc_get1();
    unsigned huff_readid (HCoder*);
    uint32_t outPos,inPos;
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
        //delete[] outChar;
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
    void decode_char1();
    void decode_char();
    ~CanonicalDecode(){};
};

void print_vector(vector<uint32_t>,string);
void print_vector(uint32_t*,uint32_t);
