#include "include\Adaptive.hpp"
#include "include\UniversalCode.h"

int globf=0;

Gagie::Gagie(WordBasedText* w,char c): BackwardEncoder(w),code_type(c) {
    n = text->Nchar;
    freqs = new uint32_t[sigma];
    fill(freqs,freqs+sigma,1);
    text->text_rewind();
};

GagieDecode::GagieDecode(int text_len,int sc):Gagie(text_len) {
    freqs = new uint32_t[sigma];
    code_type=sc;
    fill(freqs,freqs+sigma,1);
    outChar = new unsigned char[n];
};

Gagie::~Gagie(){
    delete[] freqs;
    delete[] codeStream;
   // delete C;
};

void Gagie::BlockEncode(int blockSize) {
	for(int i=0; i < blockSize; i++) {
        unsigned char c = inputText[inPos++];
        uint64_t code = C->codewords[c];
        uint8_t codeLen = C->lengths[c];
        bufferShift -= codeLen;
		buffer |= code << bufferShift;
		if (bufferShift <= 32) {
				codeStream[streamPos++] = buffer >> 32;
				buffer <<= 32;
				bufferShift += 32;
		}
		freqs[c]++;
    }
}

void CanonicalDecode::BlockDecode(int cntCodes) {
uint64_t* stream64 = (uint64_t*)codeStream;
int32_t lookupShift = 64 - LOOKUP_BITS;
	for (int32_t i = 0; i < cntCodes; i++) {
			uint32_t blockCode = buffer >> 32;
			int32_t l = C->t_first[buffer >> lookupShift];
			while (blockCode >= C->t_limit[l])	{
				l++;
			}
			bufferShift += l;
			buffer <<= l;
			blockCode >>= 32 - l;
			blockCode += C->t_firstCodeDiff[l];
			unsigned char c = C->freq_char[blockCode].second;
			outChar[outPos++] = c;
            freqs[c]++;
		if (bufferShift > 64) {
			bufferShift -= 64;
			streamPos++;
			buffer |= stream64[streamPos] << bufferShift;
		}
		buffer |= stream64[streamPos + 1] >> (64 - bufferShift);
	}
}

void GagieDecode::BlockDecode(int blockSize) {
uint8_t lookupShift = 64 - C->maxClen;
    for(int i=0; i < blockSize; i++) {
        uint32_t x = buffer >> lookupShift;
        uint8_t len = C->decodeL[x];
        unsigned char c = C->decode[x];
        outChar[outPos++] = c;
        bufferShift += len;
        buffer <<= len;
        if (bufferShift >= 32)	{
            bufferShift -= 32;
			buffer |= ((uint64_t)codeStream[++streamPos]) << bufferShift;
        }
        freqs[c]++;
    }
}

int Gagie::encode_char() {
int blockSize=ceil((double)sigma*log2(n)),i;
    bufferShift = 64;
    streamPos = buffer = 0;
    if(code_type>3) {
        fill(freqs,freqs+sigma,code_type==4?1:0);
        C = new Shannon(sigma,code_type&1);
    } else {
        C = new Huffman(sigma,blockSize,code_type&1);
    }
    inPos = 0;
    for(i=0;i<n-blockSize;) {
        BlockEncode(blockSize);
        C->update_code(freqs,i+=blockSize);
    }
    BlockEncode(n-i+5);

    return streamPos<<2;
}

int Gagie::encode_char1() {
int blockSize=50,i;
    bufferShift = 64;
    streamPos = buffer = inPos = 0;
    if(code_type>3) {
        fill(freqs,freqs+sigma,code_type==4?1:0);
        C = new Shannon(sigma,code_type&1);
    } else {
        C = new Huffman(sigma,blockSize,code_type&1);
    }
    for(i=0;i<n-blockSize;) {
        BlockEncode(blockSize);
        C->update_code(freqs,i+=blockSize);
        blockSize*=3;
    }
    BlockEncode(n-i+5);
    if(code_type>1 && code_type<4)
        for (int32_t i = 0; i < streamPos; i += 2) {
            swap(codeStream[i], codeStream[i + 1]);
        }
    globf=0;

    return streamPos<<2;
}

void GagieDecode::decode_char() {
int blockSize=ceil((double)sigma*log2(n)),i;
    bufferShift = outPos = 0;
    streamPos = 1;
    if(code_type>3) {
        fill(freqs,freqs+sigma,code_type==4?1:0);
        C = new Shannon(sigma,code_type==4?1<<24:1<<(int)ceil(log2(sigma)+log2(log2(n))),code_type&1);
    } else {
        C = new Huffman(sigma,blockSize,code_type&1);
        C -> initDecode();
    }
    buffer = (((uint64_t)codeStream[0])<<32)|codeStream[1];
    for(i=0;i<n-blockSize;) {
        BlockDecode(blockSize);
        C->update_decode(freqs,i+=blockSize);
    }
    BlockDecode(n-i+5);
 }

void GagieDecode::decode_char1() {
int blockSize=50,i;
    bufferShift = outPos = 0;
    streamPos = 1;
    if(code_type>3) {
        fill(freqs,freqs+sigma,code_type==4?1:0);
        C = new Shannon(sigma,code_type==4?1<<24:1<<(int)ceil(log2(sigma)+log2(log2(n))),code_type&1);
    } else {
        C = new Huffman(sigma,blockSize,code_type&1);
        C -> initDecode();
    }
    buffer = (((uint64_t)codeStream[0])<<32)|codeStream[1];
    for(i=0;i<n-blockSize;) {
        BlockDecode(blockSize);
        C->update_decode(freqs,i+=blockSize);
        blockSize *= 3;
    }
    BlockDecode(n-i+5);
 }

void CanonicalDecode::decode_char1() {
int blockSize=50,i;
uint64_t* stream64 = (uint64_t*)codeStream;
    bufferShift = outPos = 0;
    streamPos = 0;
    C = new CHuffman(sigma,blockSize,code_type&1);
    C -> initDecode();
    buffer = stream64[0];
    for(i=0;i<n-blockSize;) {
        BlockDecode(blockSize);
        C->update_decode(freqs,i+=blockSize);
        blockSize *= 3;
    }
    BlockDecode(n-i+5);
 }

void CanonicalDecode::decode_char() {
int blockSize=ceil((double)sigma*log2(n)),i;
uint64_t* stream64 = (uint64_t*)codeStream;
    bufferShift = outPos = 0;
    streamPos = 0;
    C = new CHuffman(sigma,blockSize,code_type&1);
    C -> initDecode();
    buffer = stream64[0];
    for(i=0;i<n-blockSize;) {
        BlockDecode(blockSize);
        C->update_decode(freqs,i+=blockSize);
    }
    BlockDecode(n-i+5);
 }

double Gagie::entropy() {
double ent=0;
    for(int i=0;i<sigma;i++) {
        if(freqs[i]>0)
            ent-=(double)freqs[i]*log2((double)freqs[i]/n);
    }
    return ent;
}
