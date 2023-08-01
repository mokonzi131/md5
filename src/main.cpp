#include <filesystem>
#include <memory>
#include <format>
#include <cstdlib>
#include <iostream>
#include <array>
#include <istream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include "inc/types.h"
#include "inc/bit.h"
#include "inc/blockloader.h"

// the md5 spec uses a table of results from the sine function
// rather than hard-coding the values, we can actually generate them
static constexpr hword hsine(size_t i) { return static_cast<hword>(4294967296 * std::abs(std::sin(i))); }
static const hword sineT[65] = {
    hsine(0), hsine(1), hsine(2), hsine(3), hsine(4), hsine(5), hsine(6), hsine(7),
    hsine(8), hsine(9), hsine(10), hsine(11), hsine(12), hsine(13), hsine(14), hsine(15),
    hsine(16), hsine(17), hsine(18), hsine(19), hsine(20), hsine(21), hsine(22), hsine(23),
    hsine(24), hsine(25), hsine(26), hsine(27), hsine(28), hsine(29), hsine(30), hsine(31),
    hsine(32), hsine(33), hsine(34), hsine(35), hsine(36), hsine(37), hsine(38), hsine(39),
    hsine(40), hsine(41), hsine(42), hsine(43), hsine(44), hsine(45), hsine(46), hsine(47),
    hsine(48), hsine(49), hsine(50), hsine(51), hsine(52), hsine(53), hsine(54), hsine(55),
    hsine(56), hsine(57), hsine(58), hsine(59), hsine(60), hsine(61), hsine(62), hsine(63), 
    hsine(64)
};

// auxilary functions defined in the spec
static hword auxF(hword x, hword y, hword z) {
    // XY v not(X) Z
    return (x & y) | (~x & z);
}

static hword auxG(hword x, hword y, hword z) {
    // XZ v Y not(Z)
    return (x & z) | (y & ~z);
}

static hword auxH(hword x, hword y, hword z) {
    // X xor Y xor Z
    return x ^ y ^ z;
}

static hword auxI(hword x, hword y, hword z) {
    // Y xor (X v not(Z))
    return y ^ (x | ~z);
}

int main(int argc, char** argv) {
    // Step 0. init input
    if (argc != 2) {
        std::cout << "USAGE: md5 <path> - where path is the file you want to hash\n";
        return EXIT_FAILURE;
    }
    auto file = std::filesystem::path(argv[1]);
    std::ifstream data{ file.string(), std::ios::binary };
    BlockLoader loader{ data };

    // Step 3. initialize MD buffer which is ABCD 32-bit words (low-order bits first)
    hword A = byteswap(A0);
    hword B = byteswap(B0);
    hword C = byteswap(C0);
    hword D = byteswap(D0);

    // Step 4. process the message in 16-word blocks (512 bits)
    std::array<hword, 16> block;
    while (loader.hasMoreData()) {
        loader.loadNextChunk(block);
        
        hword AA = A;
        hword BB = B;
        hword CC = C;
        hword DD = D;

        // round 1
        auto r1 = [&block](hword& a, hword& b, hword& c, hword& d, size_t k, size_t s, size_t i) {
            a = b + std::rotl((a + auxF(b, c, d) + block[k] + sineT[i]), s);
        };
        r1(A, B, C, D,  0, 7,  1);  r1(D, A, B, C,  1, 12,  2); r1(C, D, A, B,  2, 17,  3); r1(B, C, D, A,  3, 22,  4);
        r1(A, B, C, D,  4, 7,  5);  r1(D, A, B, C,  5, 12,  6); r1(C, D, A, B,  6, 17,  7); r1(B, C, D, A,  7, 22,  8);
        r1(A, B, C, D,  8, 7,  9);  r1(D, A, B, C,  9, 12, 10); r1(C, D, A, B, 10, 17, 11); r1(B, C, D, A, 11, 22, 12);
        r1(A, B, C, D, 12, 7, 13);  r1(D, A, B, C, 13, 12, 14); r1(C, D, A, B, 14, 17, 15); r1(B, C, D, A, 15, 22, 16);

        // round 2
        auto r2 = [&block](hword& a, hword& b, hword&c, hword&d, size_t k, size_t s, size_t i) {
            a = b + std::rotl((a + auxG(b, c, d) + block[k] + sineT[i]), s);
        };
        r2(A, B, C, D,  1,  5, 17); r2(D, A, B, C,  6,  9, 18); r2(C, D, A, B, 11, 14, 19); r2(B, C, D, A,  0, 20, 20);
        r2(A, B, C, D,  5,  5, 21); r2(D, A, B, C, 10,  9, 22); r2(C, D, A, B, 15, 14, 23); r2(B, C, D, A,  4, 20, 24);
        r2(A, B, C, D,  9,  5, 25); r2(D, A, B, C, 14,  9, 26); r2(C, D, A, B,  3, 14, 27); r2(B, C, D, A,  8, 20, 28);
        r2(A, B, C, D, 13,  5, 29); r2(D, A, B, C,  2,  9, 30); r2(C, D, A, B,  7, 14, 31); r2(B, C, D, A, 12, 20, 32);
        
        // round 3
        auto r3 = [&block](hword& a, hword& b, hword&c, hword&d, size_t k, size_t s, size_t i) {
            a = b + std::rotl((a + auxH(b, c, d) + block[k] + sineT[i]), s);
        };
        r3(A, B, C, D,  5,  4, 33);  r3(D, A, B, C,  8, 11, 34);  r3(C, D, A, B, 11, 16, 35);  r3(B, C, D, A, 14, 23, 36);
        r3(A, B, C, D,  1,  4, 37);  r3(D, A, B, C,  4, 11, 38);  r3(C, D, A, B,  7, 16, 39);  r3(B, C, D, A, 10, 23, 40);
        r3(A, B, C, D, 13,  4, 41);  r3(D, A, B, C,  0, 11, 42);  r3(C, D, A, B,  3, 16, 43);  r3(B, C, D, A,  6, 23, 44);
        r3(A, B, C, D,  9,  4, 45);  r3(D, A, B, C, 12, 11, 46);  r3(C, D, A, B, 15, 16, 47);  r3(B, C, D, A,  2, 23, 48);
        
        // round 4
        auto r4 = [&block](hword& a, hword& b, hword&c, hword&d, size_t k, size_t s, size_t i) {
            a = b + std::rotl((a + auxI(b, c, d) + block[k] + sineT[i]), s);
        };
        r4(A, B, C, D,  0,  6, 49);  r4(D, A, B, C,  7, 10, 50);  r4(C, D, A, B, 14, 15, 51);  r4(B, C, D, A,  5, 21, 52);
        r4(A, B, C, D, 12,  6, 53);  r4(D, A, B, C,  3, 10, 54);  r4(C, D, A, B, 10, 15, 55);  r4(B, C, D, A,  1, 21, 56);
        r4(A, B, C, D,  8,  6, 57);  r4(D, A, B, C, 15, 10, 58);  r4(C, D, A, B,  6, 15, 59);  r4(B, C, D, A, 13, 21, 60);
        r4(A, B, C, D,  4,  6, 61);  r4(D, A, B, C, 11, 10, 62);  r4(C, D, A, B,  2, 15, 63);  r4(B, C, D, A,  9, 21, 64);

        // increment
        A = A + AA;
        B = B + BB;
        C = C + CC;
        D = D + DD;
    }

    // Step 5. Output message, low order byte of A to high order byte of D
    std::cout << std::hex 
        << std::setw(8) << std::setfill('0') << byteswap(A)
        << std::setw(8) << std::setfill('0') << byteswap(B) 
        << std::setw(8) << std::setfill('0') << byteswap(C) 
        << std::setw(8) << std::setfill('0') << byteswap(D) << "\n";
}
