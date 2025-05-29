#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <cstring>
#include "include\Adaptive.hpp"

using std::uint32_t;
using namespace std;

void print_vector(uint32_t* a,uint32_t n) {
    cout<<"======= Print vector ========= "<<endl;
    for(int i=0;i<n;i++)
        cout<<a[i]<<" ";
    cout<<endl;
}

void BackwardEncoder::serializeChar(uint8_t* ext_buf) {
    *(uint32_t*)ext_buf = n;
    memcpy(ext_buf+4,codeStream,n);
}

void BackwardEncoder::loadChar(uint8_t* ext_buf) {
    n = *(uint32_t*)ext_buf;
    memcpy(codeStream,ext_buf+4,n);
}

// Build the encoder/decoder for the text w
BackwardEncoder::BackwardEncoder(CharBasedText* w) {
    text=w;
    n = text->Nchar;
    codeStream = new uint32_t[n/4+10];
    const std::string tmp = text->buffer.str();
    inputText = new char[n];
    memcpy(inputText,tmp.c_str(),n);
};

BackwardEncoder::BackwardEncoder(CharBasedText* w,vector<uint32_t> d) {
    text = w;
    dic = d;
};

BackwardEncoder::BackwardEncoder(int text_len): n(text_len) {
    codeStream = new uint32_t[n/4+10];
    inputText = new char[n];
};

// Count the number of characters that coincide in the input and decoded text
void BackwardEncoder::checkDecodeChar(CharBasedText *text) {
int i,len=n;
    text->text_rewind();
    for(;text->get_char()==outChar[i] && i<n;i++);
    cout<<i<<" symbols decoded correctly."<<endl;
}

