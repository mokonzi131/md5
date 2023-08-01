#pragma once

#include <istream>
#include <array>
#include "types.h"

class BlockLoader {
public:
    BlockLoader(std::istream& in);

    bool hasMoreData();

    void loadNextChunk(std::array<hword, 16>& block);

private:
    static constexpr size_t BUFFER_BYTES = 16 * sizeof(hword);

    enum class BlockLoadState { READ, PAD, DONE };

    BlockLoadState m_processingState = BlockLoadState::READ;
    char m_inputBuffer[BUFFER_BYTES] = {};
    uint64_t m_messageSize = 0;
    std::istream& m_in;
};
