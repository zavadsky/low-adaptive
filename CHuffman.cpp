#include <algorithm>
#include "include\UniversalCode.h"

void print_vector(uint32_t*,uint32_t);

Huffman::Huffman(uint32_t s,uint32_t n,bool sc) {
    smoothed_code = sc;
    sigma = s;
    lengths = new uint8_t[sigma]();
    codewords = new uint32_t[sigma]();
    cntPar.resize(2*sigma-1);
    decode = new uint32_t[1];
    decodeL = new uint8_t[1];
    initCode(n);
}

void Huffman::fillHuffLens(int32_t i, uint32_t depth)
	{
		if (i < sigma)
		{
			huffLens[i] = depth;
			return;
		}

		fillHuffLens(cntPar[i][0], depth + 1);
		fillHuffLens(cntPar[i][1], depth + 1);
	}

void Huffman::initCode(uint32_t n) {
    uint32_t* freq = new uint32_t[sigma];
    fill(freq,freq+sigma,1);
    update_code(freq,n);
}


void Huffman::update_code(uint32_t* freq, uint32_t n) {
 // weight, parent
 //    print_vector(freq,sigma);
		for (auto& it : cntPar) { it[0] = 0; it[1] = -1; }

		// count frequencies
        freq_char.clear();
        auto comp = [](pair<int, int> a, pair
                   <int,int> b) {
                       return a.first > b.first;
                   };
       // cout<<endl;
        if(smoothed_code) {
            double smooth=(double)1/log2(n);
            for(int i=0;i<sigma;i++)  {
                uint32_t f = (1-smooth)*freq[i] + n*smooth/sigma;
                freq_char.push_back(make_pair(f,i));
         //       cout<<f<<" "<<freq[i]<<" # ";
            }
       //     cout<<endl<<"-----------------------------------"<<endl;
        }
        else {
            for(int i=0;i<sigma;i++)
                freq_char.push_back(make_pair(freq[i],i));
        }
        sort(freq_char.begin(),freq_char.end(),comp);
		for (int32_t i = 0; i < sigma; i++)
		{
			cntPar[i][0]=freq[freq_char[i].second];
		}

		// build some tree to get huffman codes lengths
		int32_t iLeaf = sigma - 1, iNode = sigma, nextNode = sigma;

		for (int32_t iter = 0; iter < sigma - 1; iter++)
		{
			if (iter == 0 || (iLeaf > 1 && cntPar[iLeaf - 1][0] < cntPar[iNode][0]))
			{
				cntPar[nextNode][0] = cntPar[iLeaf][0] + cntPar[iLeaf - 1][0];
				cntPar[iLeaf][1] = nextNode;
				cntPar[iLeaf - 1][1] = nextNode;
				iLeaf -= 2;
				nextNode++;
			}
			else if (iLeaf == -1 || (abs(iNode - nextNode) > 1 && cntPar[iNode + 1][0] < cntPar[iLeaf][0]))
			{
				cntPar[nextNode][0] = cntPar[iNode][0] + cntPar[iNode + 1][0];
				cntPar[iNode][1] = nextNode;
				cntPar[iNode + 1][1] = nextNode;
				iNode += 2;
				nextNode++;
			}
			else
			{
				cntPar[nextNode][0] = cntPar[iLeaf][0] + cntPar[iNode][0];
				cntPar[iLeaf][1] = nextNode;
				cntPar[iNode][1] = nextNode;
				iLeaf--;
				iNode++;
				nextNode++;
			}
		}

		// now cntPar is represented as {child 1, child 2}
		for (auto& it : cntPar) { it[0] = -1; }
		for (int32_t i = int32_t(cntPar.size()) - 2; i >= 0; i--)
		{
			int32_t parent = cntPar[i][1];

			if (cntPar[parent][0] == -1)
			{
				cntPar[parent][0] = i;
			}
			else
			{
				cntPar[parent][1] = i;
			}
		}

		// prepare canonical huffman codes arrays

		huffLens.resize(sigma);
		fillHuffLens(int32_t(cntPar.size()) - 1, 0);

		t_minHuffLen = huffLens[0];
		maxClen = huffLens[sigma - 1];

		unsigned char prev = freq_char[0].second;
		codewords[prev] = 0;
		lengths[prev]=huffLens[0];

        for (int32_t i = 1; i < sigma; i++)
		{
		    uint8_t c = freq_char[i].second;
		    codewords[c] = (codewords[prev] + 1) << (huffLens[i] - huffLens[i-1]);
			lengths[c] = huffLens[i];
			prev = c;
		}
}

// Non-canonical Huffman decode
void Huffman::initDecode() {
    uint32_t dec_size = 1<<maxClen;
    delete[] decode;
    delete[] decodeL;
    decode = new uint32_t[dec_size];
    decodeL = new uint8_t[dec_size];
    for(int i=0;i<sigma;i++) {
        uint32_t x = codewords[i];
        uint8_t d = maxClen-lengths[i];
        for(uint32_t j=0;j<(1<<d);j++) {
            decode[(x<<d)+j] = i;
            decodeL[(x<<d)+j] = lengths[i];
        }
    }
}

void Huffman::update_decode(uint32_t* freq, uint32_t n) {
    update_code(freq,n);
    initDecode();
}

CHuffman::CHuffman(uint32_t a,uint32_t b,bool c):Huffman(a,b,c) {}

void CHuffman::initDecode() {
vector <int32_t> cntCodesPerLen(32, 0);
        fill(t_cntCodesPerLen,t_cntCodesPerLen+32,0);
		for (int32_t i = 0; i < sigma; i++) {
			t_cntCodesPerLen[huffLens[i]]++;
		}

		int32_t lastHuffCode, lastHuffCodeLen;

		t_firstCode[t_minHuffLen] = 0;
		t_firstHuffCode[t_minHuffLen] = 0;
		t_firstCodeDiff[t_minHuffLen] = t_firstCode[t_minHuffLen] - t_firstHuffCode[t_minHuffLen];
		lastHuffCode = t_firstHuffCode[t_minHuffLen] + t_cntCodesPerLen[t_minHuffLen] - 1;
		lastHuffCodeLen = t_minHuffLen;

		for (int32_t i = t_minHuffLen + 1; i <= maxClen; i++)
		{
            t_firstCode[i] = t_firstCode[i - 1] + t_cntCodesPerLen[i - 1];
            t_firstHuffCode[i] = (lastHuffCode + 1) << (i - lastHuffCodeLen);
			t_firstCodeDiff[i] = t_firstCode[i] - t_firstHuffCode[i];
			if (t_cntCodesPerLen[i] > 0)
			{
				lastHuffCode = t_firstHuffCode[i] + t_cntCodesPerLen[i] - 1;
				lastHuffCodeLen = i;
			}
			else
			{
			}
		}
		t_limit[maxClen] = 1ull << 32;
		lastHuffCode = t_firstHuffCode[maxClen];
		lastHuffCodeLen = maxClen;
		for (int32_t i = maxClen - 1; i >= t_minHuffLen; i--)
		{
			if (t_cntCodesPerLen[i] > 0)
			{
				t_limit[i] = (((uint64_t)lastHuffCode) << (32 - lastHuffCodeLen));
				lastHuffCode = t_firstHuffCode[i];
				lastHuffCodeLen = i;
			}
			else
			{
				t_limit[i] = (((uint64_t)lastHuffCode) << (32 - lastHuffCodeLen));
			}
		}

		for (int32_t firstBits = 0; firstBits < LOOKUP_SIZE; firstBits++)
		{
			int32_t l = t_minHuffLen;
			uint32_t blockCode = (firstBits << (32 - LOOKUP_BITS));

			while (blockCode >= t_limit[l])
			{
				l++;
			}

			t_first[firstBits] = l;
		}
}

void CHuffman::update_decode(uint32_t* freq, uint32_t n) {
    update_code(freq,n);
    initDecode();
}

