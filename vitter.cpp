// Vitter's algorithm. Code from https://code.google.com/archive/p/compression-code/downloads
#include <memory.h>
#include "include\Adaptive.hpp"

FILE *In, *Out;
//unsigned char ArcBit = 0;
//int ArcChar = 0;

Vitter::Vitter(WordBasedText* w):BackwardEncoder(w) {
    huff = huff_init (size, size);
}

Vitter::Vitter(int text_len): BackwardEncoder(text_len){};

VitterDecode::VitterDecode(int text_len):Vitter(text_len) {
    outChar = new unsigned char[n];
    outPos = 0;
};

//  initialize an adaptive coder
//  for alphabet size, and count
//  of nodes to be used

HCoder* Vitter::huff_init (unsigned size, unsigned root)
{
  //  default: all alphabet symbols are used

  if( !root || root > size )
      root = size;

  //  create the initial escape node
  //  at the tree root

  if( root <<= 1 )
    root--;

  huff = (HCoder*)malloc (root * sizeof(HTable) + sizeof(HCoder));
  memset (huff->table + 1, 0, root * sizeof(HTable));
  memset (huff, 0, sizeof(HCoder));

  if( huff->size = size )
//#ifdef HUFFSTANDALONE
      huff->map = (unsigned*)calloc (size, sizeof(unsigned));
/*#else
      huff->map = zalloc (size * sizeof(unsigned));
#endif*/

  huff->esc = huff->root = root;

  return huff;
}

void Vitter::arc_put1 (unsigned bit)
{
    ArcChar <<= 1;

    if( bit )
        ArcChar |= 1;

    if( ++ArcBit < 32 )
        return;
    codeStream[len++] = ArcChar;
    ArcChar = ArcBit = 0;
}

unsigned VitterDecode::arc_get1 ()
{
    if( !ArcBit )
        ArcChar = codeStream[inPos++], ArcBit = 32;

    return ArcChar >> --ArcBit & 1;
}

// split escape node to incorporate new symbol

unsigned Vitter::huff_split (HCoder *huff, unsigned symbol)
{
unsigned pair, node;

    //  is the tree already full???

    if( pair = huff->esc )
        huff->esc--;
    else
        return 0;

    //  if this is the last symbol, it moves into
    //  the escape node's old position, and
    //  huff->esc is set to zero.

    //  otherwise, the escape node is promoted to
    //  parent a new escape node and the new symbol.

    if( node = huff->esc ) {
        huff->table[pair].down = node;
        huff->table[pair].weight = 1;
        huff->table[node].up = pair;
        huff->esc--;
    } else
        pair = 0, node = 1;

    //  initialize the new symbol node

    huff->table[node].symbol = symbol;
    huff->table[node].weight = 0;
    huff->table[node].down = 0;
    huff->map[symbol] = node;

    //  initialize a new escape node.

    huff->table[huff->esc].weight = 0;
    huff->table[huff->esc].down = 0;
    huff->table[huff->esc].up = pair;
    return node;
}

//  swap leaf to group leader position
//  return symbol's new node

unsigned Vitter::huff_leader (HCoder *huff, unsigned node)
{
unsigned weight = huff->table[node].weight;
unsigned leader = node, prev, symbol;

    while( weight == huff->table[leader + 1].weight )
        leader++;

    if( leader == node )
        return node;

    // swap the leaf nodes

    symbol = huff->table[node].symbol;
    prev = huff->table[leader].symbol;

    huff->table[leader].symbol = symbol;
    huff->table[node].symbol = prev;
    huff->map[symbol] = leader;
    huff->map[prev] = node;
    return leader;
}

//  slide internal node up over all leaves of equal weight;
//  or exchange leaf with next smaller weight internal node

//  return node's new position

unsigned Vitter::huff_slide (HCoder *huff, unsigned node)
{
unsigned next = node;
HTable swap[1];

    *swap = huff->table[next++];

    // if we're sliding an internal node, find the
    // highest possible leaf to exchange with

    if( swap->weight & 1 )
      while( swap->weight > huff->table[next + 1].weight )
          next++;

    //  swap the two nodes

    huff->table[node] = huff->table[next];
    huff->table[next] = *swap;

    huff->table[next].up = huff->table[node].up;
    huff->table[node].up = swap->up;

    //  repair the symbol map and tree structure

    if( swap->weight & 1 ) {
        huff->table[swap->down].up = next;
        huff->table[swap->down - 1].up = next;
        huff->map[huff->table[node].symbol] = node;
    } else {
        huff->table[huff->table[node].down - 1].up = node;
        huff->table[huff->table[node].down].up = node;
        huff->map[swap->symbol] = next;
    }

    return next;
}

//  increment symbol weight and re balance the tree.

void Vitter::huff_increment (HCoder *huff, unsigned node)
{
unsigned up;

  //  obviate swapping a parent with its child:
  //    increment the leaf and proceed
  //    directly to its parent.

  //  otherwise, promote leaf to group leader position in the tree

  if( huff->table[node].up == node + 1 )
    huff->table[node].weight += 2, node++;
  else
    node = huff_leader (huff, node);

  //  increase the weight of each node and slide
  //  over any smaller weights ahead of it
  //  until reaching the root

  //  internal nodes work upwards from
  //  their initial positions; while
  //  symbol nodes slide over first,
  //  then work up from their final
  //  positions.

  while( huff->table[node].weight += 2, up = huff->table[node].up ) {
    while( huff->table[node].weight > huff->table[node + 1].weight )
        node = huff_slide (huff, node);

    if( huff->table[node].weight & 1 )
        node = up;
    else
        node = huff->table[node].up;
  }
}

//  scale all weights and rebalance the tree

//  zero weight nodes are removed from the tree
//  by sliding them out the left of the rank list

void Vitter::huff_scale (HCoder *huff, unsigned bits)
{
unsigned node = huff->esc, weight, prev;

  //  work up the tree from the escape node
  //  scaling weights by the value of bits

  while( ++node <= huff->root ) {
    //  recompute the weight of internal nodes;
    //  slide down and out any unused ones

    if( huff->table[node].weight & 1 ) {
      if( weight = huff->table[huff->table[node].down].weight & ~1 )
        weight += huff->table[huff->table[node].down - 1].weight | 1;

    //  remove zero weight leaves by incrementing HuffEsc
    //  and removing them from the symbol map.  take care

    } else if( !(weight = huff->table[node].weight >> bits & ~1) )
      if( huff->map[huff->table[node].symbol] = 0, huff->esc++ )
        huff->esc++;

    // slide the scaled node back down over any
    // previous nodes with larger weights

    huff->table[node].weight = weight;
    prev = node;

    while( weight < huff->table[--prev].weight )
        huff_slide (huff, prev);
  }

  // prepare a new escape node

  huff->table[huff->esc].down = 0;
}

//  send the bits for an escaped symbol

void Vitter::huff_sendid (HCoder *huff, unsigned symbol)
{
unsigned empty = 0, max;

    //  count the number of empty symbols
    //  before the symbol in the table

    while( symbol-- )
      if( !huff->map[symbol] )
        empty++;

    //  send LSB of this count first, using
    //  as many bits as are required for
    //  the maximum possible count

    if( max = huff->size - (huff->root - huff->esc) / 2 - 1 )
      do arc_put1 (empty & 1), empty >>= 1;
      while( max >>= 1 );
}

//  encode the next symbol

void Vitter::huff_encode (HCoder *huff, unsigned symbol)
{
unsigned emit = 1, bit;
unsigned up, idx, node;

    if( symbol < huff->size )
        node = huff->map[symbol];
    else
        return;

    //  for a new symbol, direct the receiver to the escape node
    //  but refuse input if table is already full.

    if( !(idx = node) )
      if( !(idx = huff->esc) )
        return;

    //  accumulate the code bits by
    //  working up the tree from
    //  the node to the root

    while( up = huff->table[idx].up )
        emit <<= 1, emit |= idx & 1, idx = up;

    //  send the code, root selector bit first

    while( bit = emit & 1, emit >>= 1 )
        arc_put1 (bit);

    //  send identification and incorporate
    //  new symbols into the tree

    if( !node )
        huff_sendid(huff, symbol), node = huff_split(huff, symbol);

    //  adjust and re-balance the tree

    huff_increment (huff, node);
}

//  read the identification bits
//  for an escaped symbol

unsigned VitterDecode::huff_readid (HCoder *huff)
{
unsigned empty = 0, bit = 1, max, symbol;

    //  receive the symbol, LSB first, reading
    //  only the number of bits necessary to
    //  transmit the maximum possible symbol value

    if( max = huff->size - (huff->root - huff->esc) / 2 - 1 )
      do empty |= arc_get1 () ? bit : 0, bit <<= 1;
      while( max >>= 1 );

    //  the count is of unmapped symbols
    //  in the table before the new one

    for( symbol = 0; symbol < huff->size; symbol++ )
      if( !huff->map[symbol] )
        if( !empty-- )
            return symbol;

    //  oops!  our count is too big, either due
    //  to a bit error, or a short node count
    //  given to huff_init.

    return 0;
}

unsigned VitterDecode::huff_decode (HCoder *huff)
{
unsigned node = huff->root;
unsigned symbol, down;

    //  work down the tree from the root
    //  until reaching either a leaf
    //  or the escape node.  A one
    //  bit means go left, a zero
    //  means go right.

    while( down = huff->table[node].down )
      if( arc_get1 () )
        node = down - 1;  // the left child preceeds the right child
      else
        node = down;

    //  sent to the escape node???
    //  refuse to add to a full tree

    if( node == huff->esc )
      if( huff->esc )
        symbol = huff_readid (huff), node = huff_split (huff, symbol);
      else
        return 0;
    else
        symbol = huff->table[node].symbol;

    //  increment weights and rebalance
    //  the coding tree

    huff_increment (huff, node);
    return symbol;
}

double Vitter::encode() {
std::string word;
    text->text_rewind();
    ArcChar=ArcBit=0;
    while(! text->eof()) {
		word=text->get_word();
        huff_encode(huff, text->word_symbol[word]);
    }
    return len;
}

int Vitter::encode_char() {
char c;
    for(int i=0;i<n;i++) {
        huff_encode(huff, inputText[i]);
//        cout<<inputText[i];
    }
    /*text->text_rewind();
    while(! text->eof()) {
		c=text->get_char();
            huff_encode(huff,c);
    }*/
    return len<<2;
}

void VitterDecode::decode_char() {
char c;
    /*text->text_rewind();
    while(! text->eof()) {
		c=text->get_char();
    delete huff;*/
    inPos = 0;
    huff = huff_init (size, size);
    for(int i=0;i<n;i++) {
        outChar[i]=huff_decode(huff);
        //cout<<outChar[i];
    }
    //}
}
