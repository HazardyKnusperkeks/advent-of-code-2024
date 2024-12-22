#include "challenge22.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>

namespace {
std::int64_t mixAndPrune(std::int64_t secret, std::int64_t number) noexcept {
    return (secret ^ number) % 16777216;
}

using SecretNumbers    = std::vector<std::int64_t>;
using OffsetsAndPrices = std::vector<std::pair<std::int64_t, std::int64_t>>;

SecretNumbers generateSecretNumbers(std::int64_t secret) noexcept {
    SecretNumbers ret;
    ret.resize(2001);
    ret[0] = secret;
    std::ranges::generate(ret | std::views::drop(1), [&secret](void) noexcept {
        secret = mixAndPrune(secret, secret * 64);
        secret = mixAndPrune(secret, secret / 32);
        secret = mixAndPrune(secret, secret * 2048);
        return secret;
    });
    return ret;
}

OffsetsAndPrices toOffsets(const SecretNumbers& numbers) noexcept {
    OffsetsAndPrices ret;
    ret.resize(numbers.size());
    ret[0] = {0, 1234}; //Invalid change
    std::ranges::transform(numbers | std::views::slide(2), std::next(ret.begin()), [](auto secretNumbers) noexcept {
        auto previousPrice = secretNumbers.front() % 10;
        auto curretPrice   = secretNumbers.back() % 10;
        return std::pair{curretPrice, curretPrice - previousPrice};
    });
    return ret;
}

struct SearchState {
    struct PerMonkeyValidIndices {
        std::size_t              MonkeyIndex;
        std::vector<std::size_t> ValidIndices;

        bool isEmpty(void) const noexcept {
            return ValidIndices.empty();
        }
    };

    const std::vector<OffsetsAndPrices>& Offsets;
    std::int64_t&                        BestCostSum;
    std::vector<PerMonkeyValidIndices>   ValidIndices;
};

void searchBestPriceSum(SearchState& state, int level = 0) noexcept {
    if ( level == 4 ) {
        auto countBananas = [&state](const SearchState::PerMonkeyValidIndices& perMonkey) noexcept {
            return state.Offsets[perMonkey.MonkeyIndex][perMonkey.ValidIndices.front()].first;
        };
        auto sum = std::ranges::fold_left(state.ValidIndices | std::views::transform(countBananas), 0, std::plus<>{});
        state.BestCostSum = std::max(state.BestCostSum, sum);
        return;
    } //if ( level == 4 )

    auto applyPlusOne = [](SearchState::PerMonkeyValidIndices& perMonkey) noexcept {
        auto plusOne = [](std::size_t& index) noexcept {
            ++index;
            return;
        };
        std::ranges::for_each(perMonkey.ValidIndices, plusOne);
    };

    std::ranges::for_each(state.ValidIndices, applyPlusOne);

    for ( auto nextOffset : std::views::iota(-9, 10) ) {
        SearchState nextState = state;

        for ( auto [inner, outer] : std::views::zip(nextState.ValidIndices, state.ValidIndices) ) {
            auto& [monkeyIndex, validIndices] = inner;
            auto& [_, outerValidIndexes]      = outer;
            // std::ranges::for_each(validIndices, plusOne);
            std::erase_if(validIndices, [&nextState, nextOffset, monkeyIndex](std::size_t index) noexcept {
                return nextState.Offsets[monkeyIndex][index].second != nextOffset;
            });
            auto newEnd = std::ranges::set_difference(outerValidIndexes, validIndices, outerValidIndexes.begin()).out;
            outerValidIndexes.erase(newEnd, outerValidIndexes.end());
        } //for ( auto& [monkeyIndex, validIndices] : nextState.ValidIndices )

        std::erase_if(nextState.ValidIndices, &SearchState::PerMonkeyValidIndices::isEmpty);

        if ( std::ssize(nextState.ValidIndices) * 9 > state.BestCostSum ) {
            searchBestPriceSum(nextState, level + 1);
        } //if ( std::ssize(nextState.ValidIndices) * 9 > state.BestCostSum )
    } //for ( auto nextOffset : std::views::iota(-9, 10) )
    return;
}
} //namespace

bool challenge22(const std::vector<std::string_view>& input) {
    const auto secretNumbers = input | std::views::transform(convert<10>) |
                               std::views::transform(generateSecretNumbers) | std::ranges::to<std::vector>();
    const auto sum1 = std::ranges::fold_left(
        secretNumbers | std::views::transform([](const SecretNumbers& numbers) noexcept { return numbers.back(); }), 0,
        std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto   offsetsAndPrices = secretNumbers | std::views::transform(toOffsets) | std::ranges::to<std::vector>();
    std::int64_t bestPriceSum     = 0;
    SearchState  state{
        offsetsAndPrices, bestPriceSum,
        std::views::iota(0zu, offsetsAndPrices.size()) |
            std::views::transform([allIndices = std::views::iota(0zu, secretNumbers.front().size() - 4) |
                                                std::ranges::to<std::vector>()](std::size_t monkeyIndex) noexcept {
                return SearchState::PerMonkeyValidIndices{monkeyIndex, allIndices};
            }) |
            std::ranges::to<std::vector>()};

    searchBestPriceSum(state);
    myPrint(" == Result of Part 2: {:d} ==\n", bestPriceSum);

    return sum1 == 15'608'699'004 && bestPriceSum == 1791;
}
