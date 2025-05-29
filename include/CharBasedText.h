#ifndef CHARBASEDTEXT_H
#define CHARBASEDTEXT_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <math.h>
#include "UniversalCode.h"

using namespace std;

class CharBasedText
{
    public:
        CharBasedText(string);
        virtual ~CharBasedText(){};
        void text_rewind(){buffer.clear(std::stringstream::goodbit); buffer.seekg(0);};
        bool eof(){return buffer.eof();};
        unsigned char get_char(){unsigned char c; buffer>>noskipws>>c; return c;}; //read the next character from the stream
        int Nchar; //total number of characters in the text
        stringstream buffer; // input text stream

    private:
        unsigned char* txt;
};



#endif // CHARBASEDTEXT_H
