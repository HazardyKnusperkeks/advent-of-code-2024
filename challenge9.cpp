#include "challenge9.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>

namespace {
using FileBlock               = std::int16_t;

constexpr FileBlock FreeBlock = -1;

std::vector<FileBlock> parse(std::string_view input) noexcept {
    std::vector<FileBlock> fileBlocks;
    fileBlocks.resize(input.size() * 9, FreeBlock); //Upper bound

    auto      toInsert  = fileBlocks.begin();
    bool      freeBlock = false;
    FileBlock nextFile  = 0;

    for ( const auto& digit : input ) {
        auto blockSize = digit - '0';

        if ( freeBlock ) {
            std::advance(toInsert, blockSize);
            freeBlock = false;
        } //if ( freeBlock )
        else {
            toInsert = std::ranges::fill(toInsert, std::next(toInsert, blockSize), nextFile);
            ++nextFile;
            freeBlock = true;
        } //else -> if ( freeBlock )
    } //for ( const auto& line : input )

    //Remove trailing empty Blocks.
    fileBlocks.erase(toInsert, fileBlocks.end());
    return fileBlocks;
}

std::int64_t moveFileBlocksAndBuildCheckSum(const std::vector<FileBlock>& fileBlocks) {
    throwIfInvalid(!fileBlocks.empty());
    auto front = fileBlocks.begin();
    auto back  = std::prev(fileBlocks.end());

    throwIfInvalid(*back != FreeBlock);

    std::int64_t checkSum = 0;
    std::int64_t position = 0;

    while ( std::ranges::distance(front, back) >= 0 ) {
        auto value = *front;
        ++front;

        if ( value == FreeBlock ) {
            value = *back;
            do {
                --back;
            } while ( *back == FreeBlock );
        } //if ( value == FreeBlock )

        checkSum += value * position;
        ++position;
    } //while ( std::ranges::distance(front,back) >= 0 )
    return checkSum;
}

std::int64_t moveFilesAndBuildCheckSum(std::vector<FileBlock>& fileBlocks) {
    auto front           = std::ranges::find(fileBlocks, FreeBlock);
    auto back            = fileBlocks.rbegin();

    const auto rend      = fileBlocks.rend();
    auto       end       = fileBlocks.end();
    const auto notFree   = [](FileBlock block) noexcept { return block != FreeBlock; };

    auto lastFileChecked = *back + 1;

    while ( std::ranges::distance(front, back.base()) >= 0 ) {
        const auto fileToMoveBegin = back;
        auto       freeOrNewFile   = [&fileToMoveBegin](FileBlock block) noexcept { return block != *fileToMoveBegin; };
        const auto fileToMoveEnd   = std::ranges::find_if(fileToMoveBegin, rend, freeOrNewFile);
        const auto fileSize        = std::ranges::distance(fileToMoveBegin, fileToMoveEnd);

        if ( *fileToMoveBegin >= lastFileChecked ) {
            //This file was already moved!
            back = std::ranges::find_if(fileToMoveEnd, rend, notFree);
            continue;
        } //if ( *fileToMoveBegin >= lastFileChecked )
        else {
            lastFileChecked = *fileToMoveBegin;
        } //else -> if ( *fileToMoveBegin >= lastFileChecked )

        auto beginOfFreeBlock = front;
        auto endOfFreeBlock   = std::ranges::find_if(beginOfFreeBlock, end, notFree);
        auto freeBlockSize    = std::ranges::distance(beginOfFreeBlock, endOfFreeBlock);

        while ( fileSize > freeBlockSize && std::ranges::distance(endOfFreeBlock, fileToMoveEnd.base()) > 0 ) {
            beginOfFreeBlock = std::ranges::find(endOfFreeBlock, end, FreeBlock);
            endOfFreeBlock   = std::ranges::find_if(beginOfFreeBlock, end, notFree);
            freeBlockSize    = std::ranges::distance(beginOfFreeBlock, endOfFreeBlock);
        } //while ( fileSize > freeBlockSize && std::ranges::distance(endOfFreeBlock, candidateToMoveEnd.base()) > 0 )

        if ( std::ranges::distance(endOfFreeBlock, fileToMoveEnd.base()) >= 0 && fileSize <= freeBlockSize ) {
            std::ranges::fill(beginOfFreeBlock, std::next(beginOfFreeBlock, fileSize), *fileToMoveBegin);
            front = std::ranges::find(front, end, FreeBlock);

            if ( fileToMoveBegin.base() == end ) {
                end = fileBlocks.erase(
                    std::next(std::ranges::find_last_if(fileBlocks.begin(), fileToMoveEnd.base(), notFree).begin()),
                    end);
            } //if ( fileToMoveBegin.base() == end )
            else {
                std::ranges::fill(fileToMoveBegin, fileToMoveEnd, FreeBlock);
            } //else -> if ( fileToMoveBegin.base() == end )
        } //if ( std::ranges::distance(endOfFreeBlock, candidateToMoveEnd.base()) >= 0 && fileSize <= freeBlockSize )
        back = std::ranges::find_if(fileToMoveEnd, rend, notFree);
    } //while ( std::ranges::distance(front, back.base()) >= 0 )

    return std::ranges::fold_left(
        fileBlocks | std::views::enumerate | std::views::filter([](auto indexAndFileNumber) noexcept {
            return std::get<1>(indexAndFileNumber) != FreeBlock;
        }) | std::views::transform([](auto indexAndFileNumber) noexcept {
            auto [index, fileNumber] = indexAndFileNumber;
            return index * fileNumber;
        }),
        0LL, std::plus<>{});
}
} //namespace

bool challenge9(const std::vector<std::string_view>& input) {
    throwIfInvalid(input.size() == 1);
    auto fileBlocks = parse(input.front());

    const auto sum1 = moveFileBlocksAndBuildCheckSum(fileBlocks);

    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 = moveFilesAndBuildCheckSum(fileBlocks);

    myPrint(" == Result of Part 2: {:d} ==\n", sum2);
    return sum1 == 6'519'155'389'266 && sum2 == 6'547'228'115'826;
}
