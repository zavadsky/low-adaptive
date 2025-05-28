# Test environment for different methods of adaptive encoding.
Usage:
adaptive.exe file_name number_of_runs code intervals

code:
0 - Huffman, 1 - Huffman smoothed, 2 - Huffman canonical, 3 - Huffman canonical smoothed, 4 - Shannon, 5 - Shannon smoothed

intervals:
f - fixed length intervals of code update; L = [sigma*log(n)], sigma - alphabet size, n - text length
v - variable length intervals of code update; first interval = 50 characters; every next 3 times longer
