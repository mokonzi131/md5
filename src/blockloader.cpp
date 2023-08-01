#include "inc/blockloader.h"

#include <cstring>
#include "inc/bit.h"

BlockLoader::BlockLoader(std::istream& in) 
    : m_in{ in } {};

bool BlockLoader::hasMoreData() { 
    return m_processingState != BlockLoadState::DONE;
}

void BlockLoader::loadNextChunk(std::array<hword, 16>& block) {
    // clear the block of data
    std::memset(&block, 0x00, BUFFER_BYTES);
    std::memset(&m_inputBuffer, '\0', BUFFER_BYTES);

    switch (m_processingState) {
    case BlockLoadState::READ: {
        // Step 1. read message and pad to 448 % 512 bits
        // read next chunk from the input stream and transpose into our block
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

        // if we are on the last word of input, need to add a single 1-bit 
        // and then 0s for padding
        auto remainingBytesToProcess = charsRead % sizeof(hword);
        if (remainingBytesToProcess > 0 || m_in.eof()) {
            hword padWord = byteswap(std::rotr(0x80'00'00'00, 8 * remainingBytesToProcess));
            std::memcpy(&padWord, &m_inputBuffer[wordsToProcess * sizeof(hword)], remainingBytesToProcess);
            std::memcpy(&block[wordsToProcess], &padWord, sizeof(hword));
            m_processingState = BlockLoadState::PAD;
        }
        
        if (bitsRead > 448) { break; }
        [[fallthrough]];
    }
    case BlockLoadState::PAD: {
        // Step 2. append the length b bits into the remaining 2 words, low-order
        // word first, now the message M is a multiple of 16 32-bit words
        auto sizeHigh = static_cast<hword>((m_messageSize >> 32) & HIGH_32_MASK);
        auto sizeLow = static_cast<hword>(m_messageSize & HIGH_32_MASK);

        std::memcpy(&block[14], &sizeLow, sizeof(hword));
        std::memcpy(&block[15], &sizeHigh, sizeof(hword));

        m_processingState = BlockLoadState::DONE;
        break;
    }
    case BlockLoadState::DONE:
        throw std::logic_error{ "you should not be loading a chunk when done" };
    }
}
