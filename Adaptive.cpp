#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <cstring>
#include <map>
#include <set>
#include "include\Adaptive.hpp"
//#include "include\BitIoStream.hpp"

using std::uint32_t;
using namespace std;

AdaptiveN::AdaptiveN(WordBasedText* w,UniversalCode* code): BackwardEncoder(w),C(code) {C->reset();bestDigitsSz.resize(max_digit); codeStream=C->buffer32;};
AdaptiveN::AdaptiveN(WordBasedText* w,UniversalCode* code,vector<uint32_t> d): BackwardEncoder(w,d),C(code) { cout<<"^0^"<<endl;    C->reset();};
AdaptiveN::AdaptiveN(UniversalCode* code,int text_len): C(code), BackwardEncoder(text_len) {C->reset(); bestDigitsSz.resize(max_digit);};

AdaptiveNDecode::AdaptiveNDecode(int text_len,UniversalCode* code): AdaptiveN(code,text_len) {
    n = text_len;outChar = new unsigned char[n];
    C->buffer32=codeStream;C->buffer=(unsigned char*)codeStream;
};

/*void print_map(map<int,int> a) {
    cout<<"======= Print map ========="<<endl;
    for(auto it=a.begin();it!=a.end();it++)
        cout<<it->first<<" "<<it->second<<endl;
}

void print_map(map<int,map<int,int>> a) {
    cout<<"======= Print map ========="<<endl;
    for(auto it=a.begin();it!=a.end();it++) {
        cout<<endl<<it->first<<"------------------"<<endl;
        for(auto it1=it->second.begin();it1!=it->second.end();it1++)
            cout<<it1->first<<" "<<it1->second<<" | ";
    }
}

void print_map(map<int,string> a,string s) {
    cout<<"======= Print map "<<s<<" ========="<<endl;
    for(auto it=a.begin();it!=a.end();it++)
        cout<<it->first<<" "<<it->second<<endl;
}

void print_map(map<string,int> a,string s) {
    cout<<"======= Print map "<<s<<" ========="<<endl;
    for(auto it=a.begin();it!=a.end();it++)
        cout<<it->first<<" "<<it->second<<endl;
}

void print_vector(vector<uint32_t> a,string s) {
    cout<<"======= Print vector ========= "<<s<<endl;
    for(auto it=a.begin();it!=a.end();it++)
        cout<<*it<<" ";
    cout<<endl;
}

void print_vector(vector<int> a,string s) {
    cout<<"======= Print vector ========= "<<s<<endl;
    for(auto it=a.begin();it!=a.end();it++)
        cout<<*it<<" ";
    cout<<endl;
}*/


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

BackwardEncoder::BackwardEncoder(WordBasedText* w) {
    text=w;
    n = text->Nchar;
    codeStream = new uint32_t[n/4+10];
    const std::string tmp = text->buffer.str();
    inputText = new char[n];
    memcpy(inputText,tmp.c_str(),n);
 //   NYT=text->getMaxSymb()+1;
    //out.resize(text->Nwords+1);
};

BackwardEncoder::BackwardEncoder(WordBasedText* w,vector<uint32_t> d) {
    text=w;
    dic = d;
    NYT=text->getMaxSymb()+1;
    out.resize(text->Nwords+1);
};

BackwardEncoder::BackwardEncoder(int text_len): n(text_len) {
    codeStream = new uint32_t[n/4+10];
    inputText = new char[n];
};

// Lines numbering according to Algorithm 1 from the paper
void AdaptiveN::update1(int symbol) {
 //   cout<<"symbol="<<symbol<<" ind="<<ind[symbol]<<" f="<<f[symbol]<<" mfs="<<m[f[symbol]]<<endl;
    rev[ind[symbol]]=rev[m[f[symbol]]];     // 2
    ind[rev[m[f[symbol]]]]=ind[symbol];     // 3
    ind[symbol]=m[f[symbol]];               // 4
    rev[m[f[symbol]]]=symbol;               // 5
    if(rev[m[f[symbol]]+1]>=0)              // 6
        if(f[rev[m[f[symbol]]+1]]==f[symbol])   // 7
            m[f[symbol]]++;                     //8
        else
            m[f[symbol]]=-1;                    // 9
    f[symbol]++;                                // 10
    if(m[f[symbol]]==-1)                        // 11
        m[f[symbol]]=ind[symbol];               // 12
}

// Lines numbering according to Algorithm 2 from the paper
double AdaptiveN::encode1() {
//cout<<"===== Adaptive encoding started. ======"<<endl;
std::string word;
double ent=0,prob;
int i=0,mc=0;                               // Line 4 in Alg. 2
unsigned NYT=text->getMaxSymb()+1;
C->curCodeSz=0;
	try {
	    C->reset();                         // Initialize the code buffer
	    ind.resize(NYT+1);                  // Lines 1-3 in Alg. 2
	    rev.resize(NYT+1,-1);
	    f.resize(NYT+1);
	    m.resize(text->Nwords,-1);
	    ind[NYT]=0;
	    rev[0]=NYT;
	    m[0]=0;
		while(i<text->Nwords) {             // 5
			int symbol=text->numbers[i];    // line 6 in Alg. 2
            if(!f[symbol]) {                // 7
                C->flush_to_byte(ind[NYT]); // 8
                C->curCodeSz++;             // 9
                update1(NYT);
                mc++;                       // 10
                rev[mc]=symbol;             // 12
                ind[symbol]=mc;             // 13
                f[symbol]=0;                // 14
                m[0]=mc;                    // 15
             } else
                C->flush_to_byte(ind[symbol]);  // 16
            update1(symbol);                    // 17
            i++;
		}
		int r=C->code_size();
		for(int i=0;i<10;i++)                   // write the tail to fill the 32-bit word with zeros
            C->flush_to_byte(0);
        return r;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	return 1;
}

// Lines numbering according to Algorithms 2 and 4 from the paper
double AdaptiveN::encode2() {
//cout<<"===== Adaptive encoding started. ======"<<endl;
std::string word;
int i=0,totSz=0;
unsigned NYT=text->getMaxSymb()+1;
mc=0;                                   // Line 4 in Alg. 2
	try {
	    j=-1;
	    C->reset();                     // Initialize the code buffer
	    ind.resize(NYT+1);              // Lines 1-3 in Alg. 2
	    rev.resize(NYT+1);
	    rev.assign(NYT+1,-1);
	    f.resize(NYT+1);
	    f.assign(NYT+1,0);
	    m.resize(text->Nwords);
	    m.assign(text->Nwords,-1);
	    ind[NYT]=0;
	    rev[0]=NYT;
	    m[0]=0;
		while(i<text->Nwords) {             // 2.5
            if(i==j) {                      // 4.5.1
                update_code(mc);
            }
			int symbol=text->numbers[i];    // 2.6
            if(!f[symbol]) {                // 2.7
                C->flush_to_byte(ind[NYT]); // 2.8
                C->curCodeSz++;
                update1(NYT);               // 2.9
                mc++;                       // 2.10
                rev[mc]=symbol;             // 2.12
                ind[symbol]=mc;             // 2.13
                f[symbol]=0;                // 2.14
                m[0]=mc;                    // 2.15
            } else {
                if(j==-1)
                    j=i+1+log(text->Nwords)/8;  // 4.4.1
                C->flush_to_byte(ind[symbol]);  // 2.16
            }
            update1(symbol);                    // 2.17
            i++;
		}
		totSz+=C->code_size();
		for(int i=0;i<10;i++)
            C->flush_to_byte(0);
        return totSz;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	return 1;
}

double AdaptiveN::encode2_char() {
//cout<<"===== Adaptive encoding started. ======"<<endl;
std::string word;
int i=0;
unsigned NYT=sigma;
	try {
	    j=50;
	    C->reset();                     // Initialize the code buffer
	    text->text_rewind();
	    ind.resize(NYT+1);              // Lines 1-3 in Alg. 2
	    rev.resize(NYT+1);
	    //rev.assign(NYT+1,-1);
	    for(int i=0;i<NYT;i++)
            ind[i]=rev[i]=i;
	    f.resize(NYT+1);
	    f.assign(NYT+1,1);
	    update_code_char(sigma);
	    m.resize(n,0);
	    //m.assign(sigma,0);
	    //m[0]=0;
		for(int i=0;i<n;i++) {             // 2.5
            if(i==j) {
 //               cout<<endl<<j<<endl;
                update_code_char(sigma);
 //               cout<<"!!!"<<text->Nchar;
            }
			//char symbol=text->get_char();    // 2.6
			char symbol=inputText[i];
            C->flush_to_byte(ind[symbol]);  // 2.16
            update1(symbol);                    // 2.17
		}
		for(int i=0;i<4;i++)
            C->flush_to_byte(0);
        return C->code_size();
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	return 1;
}

// Alg. 3 + Alg. 4 from the paper
int AdaptiveNDecode::decode2_char() {
//cout<<"===== Adaptive decoding started. ======"<<endl;
std::string word;
unsigned NYT=sigma;
	try {
	   // cout<<endl<<"Decoding."<<endl;
	    j=50;
	    C->reset();                     // Initialize the code buffer
	    //text->text_rewind();
	    ind.resize(NYT+1);              // Lines 1-3 in Alg. 2
	    rev.resize(NYT+1);
	    //rev.assign(NYT+1,-1);
	    for(int i=0;i<NYT;i++)
            ind[i]=rev[i]=i;
	    f.resize(NYT+1);
	    f.assign(NYT+1,1);
	    update_decode_char(sigma);
	    m.resize(n,0);
		for(int i=0;i<n;i++) {       // Alg. 3, line 5
            if(i==j) {                          // Alg. 4, line 5.1
               update_decode_char(sigma);
            }
            char c=C->get_symbol();              // 6
            char symbol=rev[c];
            update1(symbol);                // 15
            outChar[i]=symbol;                  // 16
            //cout<<symbol;
        }
        return 1;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	return 1;
}

// Algorithm 3 from the paper
int AdaptiveN::decode1() {
//cout<<"===== Adaptive decoding started. ======"<<endl;
std::string word;
int mc=0;                               // Line 4 in Alg. 3
	try {
	    C->reset();                     // Clear the codeword set
	    C->codeIncrease();              // Add 1st codeword to the code
	    ind.resize(NYT+1,0);                // Lines 1-3
	    fill(ind.begin(), ind.end(), 0);
	    rev.resize(NYT+1,-1);
	    fill(rev.begin(), rev.end(), -1);
	    f.resize(NYT+1,0);
	    fill(f.begin(), f.end(), 0);
	    m.resize(text->Nwords);
	    m.assign(text->Nwords,-1);
	    ind[NYT]=0;
	    rev[0]=NYT;
	    f[NYT]=0;
	    m[0]=0;
	    out[0]=0;
	    update1(NYT);                       // Decode the 1st symbol. It is always NYT
	    mc++;
	    ind[0]=mc;
        rev[mc]=0;
        f[0]=0;
        m[0]=mc;
	    update1(0);
		for(int i=1;i<text->Nwords;i++) {   // 5
            int c=C->get_symbol();          // 6
            int symbol=rev[c];
             if(symbol==NYT) {              // 7
                symbol=mc++;                // 8,9
                C->codeIncrease();
                update1(NYT);               // 10
                rev[mc]=symbol;             // 11
                ind[symbol]=mc;             // 12
                f[symbol]=0;                // 13
                m[0]=mc;                    // 14
             }
             update1(symbol);               // 15
             out[i]=symbol;                 // 16
        }
        return 1;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	return 1;
}

// Alg. 3 + Alg. 4 from the paper
int AdaptiveN::decode2() {
//cout<<"===== Adaptive decoding started. ======"<<endl;
std::string word;
mc=0;                                       // Line 4 in Alg. 3
	try {
        j=-1;
	    C->reset();                         // Clear the codeword set
	    C->codeIncrease();                  // Add 1st codeword to the code
	    ind.resize(NYT+1,0);                // Lines 1-3
	    fill(ind.begin(), ind.end(), 0);
	    rev.resize(NYT+1,-1);
	    fill(rev.begin(), rev.end(), -1);
	    f.resize(NYT+1,0);
	    fill(f.begin(), f.end(), 0);
	    m.resize(text->Nwords);
	    m.assign(text->Nwords,-1);
	    ind[NYT]=0;
	    rev[0]=NYT;
	    f[NYT]=0;
	    m[0]=0;
	    out[0]=0;
	    update1(NYT);                       // Decode the 1st symbol. It is always NYT
	    mc++;
	    ind[0]=mc;
        rev[mc]=0;
        f[0]=0;
        m[0]=mc;
	    update1(0);
		for(int i=1;i<text->Nwords;i++) {       // Alg. 3, line 5
            if(i==j) {                          // Alg. 4, line 5.1
               update_decode(mc);
            }
            int c=C->get_symbol();              // 6
            int symbol=rev[c];
            if(symbol==NYT) {                   // 7
                symbol=mc++;                    // 8,9
                C->codeIncrease();
                update1(NYT);                   // 10
                rev[mc]=symbol;                 // 11
                ind[symbol]=mc;                 // 12
                f[symbol]=0;                    // 13
                m[0]=mc;                        // 14
             } else
                if(j==-1)                       // Line 4.1 from Alg. 4
                    j=i+1+log(text->Nwords)/8;
                update1(symbol);                // 15
                out[i]=symbol;                  // 16
            }
        return 1;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
	return 1;
}

// Compare initial text with the decoded
void BackwardEncoder::checkDecode() {
int i;
    for(i=0;text->numbers[i]==out[i] && i<text->Nwords;i++);
    cout<<"AdaptiveN words decoded correctly: "<<i<<endl;
}

void BackwardEncoder::checkDecodeChar(WordBasedText *text) {
int i,len=n;
    text->text_rewind();
//    cout<<" n="<<n<<endl;
    for(i=0;i<n;) {
        for(;text->get_char()==outChar[i] && i<n;i++);
        cout<<i<<" symbols decoded correctly."<<endl;
        unsigned char c;
        do {
            i++;
            c=text->get_char();
            cout<<outChar[i];
        } while(c!=outChar[i] && i<n);
    }
   /* for(i=0;i<n;i++)
        //if(text->get_char()!=outChar[i])
            cout<<outChar[i];*/
}

// Alg. 4 in the paper
void AdaptiveN::update_code(uint32_t mc) {
int maxCode;
    maxCode=mc*rate*1.2;            // Line 5.5
    precalcBestBlocks();            // 5.2-5.4 and 5.6
    C->code_init(bestDigitsSz,maxCode);
    j*=rate;                        // 5.7
}

void AdaptiveN::update_code_char(uint32_t m) {
    mc=m;
    precalcBestBlocks();            // 5.2-5.4 and 5.6
/*    cout<<endl;
    for(int i=0;i<mc;i++)
        cout<<f[i]<<" ";
    cout<<endl;
    for(int i=0;i<32;i++)
        cout<<(int)bestDigitsSz[i]<<"|";*/
    C->code_init(bestDigitsSz,mc);
    j*=rate;                        // 5.7
}

// Alg. 4 in the paper for decoding
void AdaptiveN::update_decode(uint32_t mc) {
    update_code(mc);
    C->buidTableDecode();
}

void AdaptiveN::update_decode_char(uint32_t mc) {
    update_code_char(mc);
    C->curCodeSz = mc;
    C->buidTableDecode();
}

void AdaptiveN::print_code() {
    for(auto it=bestDigitsSz.begin();it!=bestDigitsSz.end();it++)
        cout<<(int)*it<<" ";
    cout<<endl;
}

uint64_t AdaptiveN::getSum(uint32_t l, uint32_t r)
{
	uint64_t res = 0;

	res += r > mc ? PSum[mc] : PSum[r];
	res -= l == 0 ? 0 : PSum[l - 1];

	return res;
}

// Algorithm 5 in the paper
void AdaptiveN::BCS(uint8_t* dSz, uint32_t dN, uint64_t fullN,
	uint64_t incN, uint64_t cLen, uint64_t fullSz)
{
    if(dN>=24)
        return;
	if (fullN > mc) {
			dSz[dN] = 1;
			if (fullSz < bestFullBitsSz)
			{
				bestFullBitsSz = fullSz;
				bestBlocksNum = dN-1;
				for (uint32_t i = 0; i < max_digit; i++)
				{
					bestDigitsSz[i] = dSz[i];
				}
			}
			return;
    }
    int lowestDigit=1,highestDigit=3;
    if(dN<4)
        highestDigit=8;
    /*else
        lowestDigit=1;*/
    for(int i=lowestDigit;i<=highestDigit;i++) {
            dSz[dN] = i;
            uint64_t newSz = fullSz + (cLen + i) * getSum(fullN, fullN + incN - 1);
            BCS(dSz, dN + 1, fullN + incN, incN * ((1 << i)-1), cLen + i, newSz);
		}
}

// Precalc for searching the optimal code, Alg. 4 in the paper
void AdaptiveN::precalcBestBlocks()
{
	PSum = new uint32_t[mc + 1];
	PSum[0]=0;                                  // Line 5.2 from ALg. 4
	for (uint32_t i = 1; i <= mc; i++)          // 5.3
	{
		PSum[i] = PSum[i - 1] + f[rev[i - 1]];  // 5.4
	}

	uint8_t digitLen[max_digit]; // Bit lengths of digits of optimal code
	memset(digitLen, 1, sizeof(uint8_t) * max_digit);
    bestFullBitsSz=100000000;

	BCS(digitLen, 0, 0, 1, 0, 0); // Call the optimal search function, line 5.6 in Alg. 4

	delete[] PSum;
}
