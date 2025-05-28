#include<math.h>
#include <algorithm>
#include "include\UniversalCode.h"

void print_vector(uint32_t*,uint32_t);

extern int globf;
using namespace std;

//Creates Shannon code for smoothed distribution of characters with frequencies freq and text length n



void Shannon::update_code(uint32_t* freq, uint32_t n) {
double smooth=(double)1/log2(n),cum=0,p;
vector<pair<uint32_t,uint32_t>> freq_char;
//print_vector(freq,sigma);
auto comp = [](pair<int, int> a, pair
                   <int,int> b) {
          return a.first > b.first;
    };
    for(int i=0;i<sigma;i++) {
        freq_char.push_back(make_pair(freq[i],i));
    //    printf("%d|",freq[i]);
    }
    sort(freq_char.begin(),freq_char.end(),comp);
//    printf("%.3f,%d\n",smooth,n);
    for(auto it = freq_char.begin();it!=freq_char.end();it++) {
        if(smoothed_code)
            p = (1-smooth)*it->first/n + smooth/sigma;
        else
            p = (double)it->first/(n+sigma);                 // non-smoothed
        uint8_t len=ceil(-log2(p));
  //      printf("%.3f, %.3f, %d, fr=%d|",p,cum,len,it->first);
        lengths[it->second]=len;
        codewords[it->second]=cum*(1<<len);
        cum += p;
    }
/*    for(int i=0;i<sigma;i++) {
        printf("%d %d %.0x|",i,lengths[i],codewords[i]);
    }*/
//    cout<<endl<<"=============================="<<endl;
}


/*void Shannon::update_code(uint32_t* freq, uint32_t n) {
double smooth=(double)1/log2(n),cum=0;
    multimap<uint32_t,uint32_t,greater<int>> freq_char;
    for(int i=0;i<sigma;i++)
        freq_char.insert(make_pair(freq[i],i));
    for(auto it = freq_char.begin();it!=freq_char.end();it++) {
        double p = (1-smooth)*it->first/n + smooth/sigma;
        uint8_t len=ceil(-log2(p));
        lengths[it->second]=len;
        codewords[it->second]=cum*(1<<len);
        cum += p;
    }
}*/

//Creates Shannon code for smoothed distribution of characters with frequencies freq and text length n
//and also calculates the maximum codeword length. For use during the decoding
void Shannon::update_decode(uint32_t* freq, uint32_t n) {
double smooth=(double)1/log2(n),cum=0,p;
vector<pair<uint32_t,uint32_t>> freq_char;
//print_vector(freq,sigma);
auto comp = [](pair<int, int> a, pair<int,int> b) {
          return a.first > b.first;
    };
    for(int i=0;i<sigma;i++) {
        freq_char.push_back(make_pair(freq[i],i));
    }
    sort(freq_char.begin(),freq_char.end(),comp);
    maxClen = 0;
    for(auto it = freq_char.begin();it!=freq_char.end();it++) {
        if(smoothed_code)
            p = (1-smooth)*it->first/n + smooth/sigma;
        else
            p = (double)it->first/(n+sigma);                 // non-smoothed
        uint8_t len=ceil(-log2(p));
        lengths[it->second]=len;
 //       printf("%.3f, %.3f, %d, fr=%d|",p,cum,len,it->first);
        if(len>maxClen)
            maxClen = len;
        codewords[it->second]=cum*(1<<len);
    //    printf("%d %.0x|",len,codewords[it->second]);
        cum += p;
    }
//    cout<<endl<<(int)maxClen<<"######"<<endl;
    for(int i=0;i<sigma;i++) {
        uint32_t x = codewords[i];
    //    printf("%d %d %.0x|",i,lengths[i],x);
        uint8_t d = maxClen-lengths[i];
        for(uint32_t j=0;j<(1<<d);j++) {
  /*          if(maxClen==13)
                printf("%d %d ^",d,j);*/
            decode[(x<<d)+j] = i;
            decodeL[(x<<d)+j] = lengths[i];
        }
    }
 //   cout<<endl<<"=============================="<<endl;
}

/*void Shannon::update_decode(uint32_t* freq, uint32_t n) {
double smooth=(double)1/log2(n),cum=0;
multimap<uint32_t,uint32_t,greater<int>> freq_char;
    for(int i=0;i<sigma;i++)
        freq_char.insert(make_pair(freq[i],i));
    maxClen = 0;
    for(auto it = freq_char.begin();it!=freq_char.end();it++) {
        double p = (1-smooth)*it->first/n + smooth/sigma;
        uint8_t len=ceil(-log2(p));
        lengths[it->second]=len;
        if(len>maxClen)
            maxClen = len;
        codewords[it->second]=cum*(1<<len);
        cum += p;
    }
    for(int i=0;i<sigma;i++) {
        uint32_t x = codewords[i];
        uint8_t d = maxClen-lengths[i];
        for(uint32_t j=0;j<(1<<d);j++) {
            decode[(x<<d)+j] = i;
            decodeL[(x<<d)+j] = lengths[i];
        }
    }
}*/


/*Shannon::Shannon(uint32_t* freq, uint32_t n,uint32_t s) {
    sigma = s;
//double smooth=(double)1/log2(n),cum=0;
    update_code(freq, n);
}*/

void Shannon::initCode() {
double p=(double)1/sigma,cum=0;
maxClen=ceil(log2(sigma));
//    cout<<"cum="<<(double)cum<<" sigma="<<sigma<<endl;
    for(int i=0;i<sigma;i++) {
        lengths[i]=maxClen;
        codewords[i]=cum*(1<<maxClen);
        cum+=p;
 //       cout<<i<<"|"<<(double)cum<<endl;
    }
}

//Creates Shannon code for the uniform distribution of sigma characters
Shannon::Shannon(uint32_t s,bool sc) {
    sigma = s;
    smoothed_code = sc;
lengths = new uint8_t[sigma]();
codewords = new uint32_t[sigma]();
decode = new uint32_t[1];
decodeL = new uint8_t[1];
    initCode();
}

//Creates Shannon code for the uniform distribution of sigma characters and dec_size elements in the decoding table. Used for the decoding.
Shannon::Shannon(uint32_t s,uint32_t dec_size,bool sc) {
    sigma = s;
    smoothed_code = sc;
    //dec_size=1<<20;
    //cout<<"sigma="<<sigma<<" dec_size="<<dec_size<<endl;
    lengths = new uint8_t[dec_size]();
    codewords = new uint32_t[dec_size]();
    initCode();
    decode = new uint32_t[dec_size];
    decodeL = new uint8_t[dec_size];
 //   mask = (1<<maxClen) - 1;

    for(int i=0;i<sigma;i++) {
        uint32_t x = codewords[i];
        decode[x] = i;
        decodeL[x] = lengths[i];
    }
}

void Shannon::print_code() {
     for(int i=0;i<sigma;i++)
        printf("%08x %d\n",codewords[i],lengths[i]);
}


