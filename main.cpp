#include <filesystem>
#include <memory>
#include <format>
#include <cstdlib>
#include <iostream>
#include <array>
#include <istream>
#include <fstream>

template<std::integral T>
constexpr T byteswap(T value) noexcept
{
    static_assert(std::has_unique_object_representations_v<T>, 
                  "T may not have padding bits");
    auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<T>(value_representation);
}

using hword = uint32_t;
static constexpr uint64_t HIGH_32_MASK = 0x00'00'00'00'FF'FF'FF'FF;
static constexpr hword HW0 = 0x00'00'00'00;
static constexpr hword A0 = 0x01'23'45'67;
static constexpr hword B0 = 0x89'ab'cd'ef;
static constexpr hword C0 = 0xfe'dc'ba'98;
static constexpr hword D0 = 0x76'54'32'10;

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

class BlockLoader {
public:
    BlockLoader(std::istream& in) : m_in{ in } {}

    bool hasMoreData() { return m_processingState != BlockLoadState::DONE; }

    void loadNextChunk(std::array<hword, 16>& block) {
        // clear the block of data
        std::memset(&block, 0x00, BUFFER_BYTES);
        std::memset(&m_inputBuffer, '\0', BUFFER_BYTES);

        switch (m_processingState) {
        case BlockLoadState::READ: {
            // read next chunk from the stream and tanspose into block
            m_in.read(m_inputBuffer, BUFFER_BYTES);
            auto charsRead = m_in.gcount();
            auto bitsRead = 8 * charsRead;
            m_messageSize += bitsRead;
            auto wordsToProcess = charsRead / sizeof(hword);
            for (size_t processedWords = 0; processedWords < wordsToProcess; processedWords++) {
                auto nextWord = HW0;
                std::memcpy(&nextWord, &m_inputBuffer[processedWords * sizeof(hword)], sizeof(hword));
                std::memcpy(&block[processedWords], &nextWord, sizeof(hword));
            }

            // handle padding byte if we are on the last word of the input
            // we append a single 1 and then all 0s until the length is correct
            auto remainingBytesToProcess = charsRead % sizeof(hword);
            if (remainingBytesToProcess > 0 || m_in.eof()) {
                hword padWord = byteswap(std::rotr(0x80'00'00'00, 8 * remainingBytesToProcess));
                std::memcpy(&padWord, &m_inputBuffer[wordsToProcess * sizeof(hword)], remainingBytesToProcess);
                std::memcpy(&block[wordsToProcess], &padWord, sizeof(hword));
                m_processingState = BlockLoadState::PAD;
            }

            // 1. pad the message to 448 % 512 bits
            if (bitsRead > 448) { break; }
        }
        case BlockLoadState::PAD: {
            // 2. append the length b bits into the remaining 2 words, low-order word first
            // now we have N words labeled M[0 ... N-1], message M is a multiple of 16 32-bit words
            auto sizeHigh = static_cast<hword>((m_messageSize >> 32) & HIGH_32_MASK);
            auto sizeLow = static_cast<hword>(m_messageSize & HIGH_32_MASK);

            // copy the next hword to the block
            std::memcpy(&block[14], &sizeLow, sizeof(hword));
            std::memcpy(&block[15], &sizeHigh, sizeof(hword));

            m_processingState = BlockLoadState::DONE;

            break;
        }
        case BlockLoadState::DONE:
            throw std::logic_error{ "you should not be loading a chunk when done" };
        }
    }

private:
    static constexpr size_t BUFFER_BYTES = 16 * sizeof(hword);

    enum class BlockLoadState { READ, PAD, DONE };

    BlockLoadState m_processingState = BlockLoadState::READ;
    char m_inputBuffer[BUFFER_BYTES] = {};
    uint64_t m_messageSize = 0;
    std::istream& m_in;
};

int main() {
    // 0. init stuff
    std::ifstream data{ "data.txt", std::ios::binary };
    BlockLoader loader{ data };

    // 3. initialize MD buffer which is ABCD 32-bit words (low-order bits first)
    hword A = byteswap(A0);
    hword B = byteswap(B0);
    hword C = byteswap(C0);
    hword D = byteswap(D0);

    // 4. process the message in 16-word blocks (512 bits)
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

    // 5. Output message, low order byte of A to high order byte of D
    std::cout << std::hex 
        << std::setw(8) << std::setfill('0') << byteswap(A)
        << std::setw(8) << std::setfill('0') << byteswap(B) 
        << std::setw(8) << std::setfill('0') << byteswap(C) 
        << std::setw(8) << std::setfill('0') << byteswap(D) << "\n";
}
