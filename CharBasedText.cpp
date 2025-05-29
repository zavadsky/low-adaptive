#include "include\CharBasedText.h"

CharBasedText::CharBasedText(string fname)
{
    FILE *f;
    f = fopen(fname.c_str(),"rb");
    std::ifstream file(fname,ios::binary);
    buffer << file.rdbuf();
    file.seekg(0, std::ios::end);
    Nchar = file.tellg();
    txt = new unsigned char[Nchar+1];
    fread(txt,1,Nchar,f);
    cout<<Nchar<<" characters in file."<<endl;
    text_rewind();
}
