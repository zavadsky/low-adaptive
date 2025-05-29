#ifndef RMD_H
#define RMD_H
#include <stdint.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include "CharBasedText.h";

using namespace std;

class CharBasedText;

#define MAX_BUF_SIZE 10000000

// Universal code
class UniversalCode {
public:
    UniversalCode(){buffer = (unsigned char*)calloc(MAX_BUF_SIZE,1);buffer32=(uint32_t*)buffer;};
    virtual int encode(vector<uint64_t>){};
    virtual void code_init(vector<uint8_t>,int){};
    vector<uint32_t> code;
    uint32_t out[100];
    virtual void flush_to_byte(uint32_t){};
    int cur_byte=0,cur_bit=7;
    virtual void reset() {cur_byte=0;cur_bit=7;};
    virtual int process8bytes(int){};
    unsigned char* get_out(){return buffer;};
    virtual int code_size(){return 0;};
    virtual uint32_t get_symbol(){};
    virtual void buidTableDecode(){};
    virtual ~UniversalCode(){delete[] buffer;};
    int serialize(uint8_t* external_buf){memcpy(external_buf,buffer,code_size());    return code_size();};
    void load(uint8_t* external_buf,int csize){memcpy(buffer,external_buf,csize);};
    virtual void codeIncrease(){};
    virtual uint8_t getDigitsNum(){};
    uint32_t* codewords;
    uint8_t* lengths;
    virtual void update_code(uint32_t*, uint32_t){};
    virtual void update_decode(uint32_t*, uint32_t){};
    virtual void print_code(){};
    virtual void initDecode(){};
    unsigned char* buffer;
    uint32_t *buffer32,*decode;
    uint8_t *decodeL;
    uint8_t maxClen;
protected:
    uint32_t sigma;
    bool smoothed_code=0;
};


class Shannon: public UniversalCode {
public:
    Shannon(uint32_t,bool);
    Shannon(uint32_t,uint32_t,bool);
    ~Shannon(){delete[] codewords; delete[] lengths; delete[] decodeL; delete[] decode;};
    void initCode();
    void print_code();
    void reset(){cur_bit=0;};
    int code_size(){return ceil((double)cur_bit/8);};
    void update_code(uint32_t*, uint32_t);
    void update_decode(uint32_t*, uint32_t);
private:
};


#define CODES_PER_BITSTREAM 4
#define LOOKUP_BITS 8
#define LOOKUP_SIZE (1 << LOOKUP_BITS)
#define LOOKUP_MASK (LOOKUP_SIZE - 1)

class Huffman: public UniversalCode {
public:
 	void precompute(uint8_t* huffTable, uint32_t huffTableLen, uint32_t dictSize);

	void huffDecode(unsigned char* codes, uint32_t cntCodes, uint32_t dictSize, uint32_t* stream, uint32_t streamLen);
    Huffman(uint32_t,uint32_t,bool);
    ~Huffman(){
        delete[] codewords; delete[] lengths; delete[] decodeL; delete[] decode;
    };
    void initCode(uint32_t);
    void print_code(){};
    void reset(){cur_bit=0;};
    int code_size(){return ceil((double)cur_bit/8);};
    void update_code(uint32_t*, uint32_t);
    virtual void update_decode(uint32_t*, uint32_t);
    void initDecode();
    vector<pair<uint32_t,uint32_t>> freq_char;
protected:
    uint32_t t_minHuffLen;
    vector <array<int32_t, 2> > cntPar;
    vector <uint32_t> huffLens;
    void fillHuffLens(int32_t, uint32_t);
};

// Canonical Huffman code
class CHuffman: public Huffman {
public:
    uint32_t t_cntCodesPerLen[32];
    uint32_t t_firstCode[32], t_firstHuffCode[32];
    int32_t t_firstCodeDiff[32]; // = t_firstCode[i] - t_firstHuffCode[i] (2 operations -> 1 operation)
    uint64_t t_limit[32];
    int8_t t_first[LOOKUP_SIZE];

	void update_decode(uint32_t*, uint32_t);

    CHuffman(uint32_t,uint32_t,bool);
    void initDecode();
    ~CHuffman(){};
private:

};



#endif
