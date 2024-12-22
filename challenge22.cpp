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
    ret[0] = {1234, 0}; //Invalid change
    std::ranges::transform(numbers | std::views::slide(2), std::next(ret.begin()), [](auto secretNumbers) noexcept {
        auto previousPrice = secretNumbers.front() % 10;
        auto curretPrice   = secretNumbers.back() % 10;
        return std::pair{curretPrice, previousPrice - curretPrice};
    });
    return ret;
}

auto tupleToArray(std::tuple<std::int64_t, std::int64_t, std::int64_t, std::int64_t> t) noexcept {
    return std::array{std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)};
}

std::int64_t findPrice(const OffsetsAndPrices& offsetsAndPrices, std::array<std::int64_t, 4> changeSequence) noexcept {
    auto range = std::ranges::search(offsetsAndPrices, changeSequence, {}, &OffsetsAndPrices::value_type::second);
    return range.empty() ? 0 : range.back().first;
}
} //namespace

#include <chrono>

bool challenge22(const std::vector<std::string_view>& input) {
    const auto secretNumbers = input | std::views::transform(convert<10>) |
                               std::views::transform(generateSecretNumbers) | std::ranges::to<std::vector>();
    const auto sum1 = std::ranges::fold_left(
        secretNumbers | std::views::transform([](const SecretNumbers& numbers) noexcept { return numbers.back(); }), 0,
        std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto       validChanges     = std::views::iota(-9, 10);
    auto       changeSequences  = std::views::cartesian_product(validChanges, validChanges, validChanges, validChanges);
    const auto offsetsAndPrices = secretNumbers | std::views::transform(toOffsets) | std::ranges::to<std::vector>();
    const auto sumDisintegrateable =
        std::ranges::max(changeSequences | std::views::transform(tupleToArray) |
                         std::views::transform([&offsetsAndPrices, bestPriceSum = std::int64_t{0}](
                                                   const std::array<std::int64_t, 4>& changeSequence) mutable noexcept {
                             static int i                         = 0;
                             using Clock                          = std::chrono::system_clock;
                             auto           now                   = Clock::now();
                             std::int64_t   priceSum              = 0;
                             constexpr auto averagePricePerSecret = 3; //A realstic upper bound.
                             std::int64_t   reachablePriceSum = averagePricePerSecret * std::ssize(offsetsAndPrices);
                             for ( const OffsetsAndPrices& offsets : offsetsAndPrices ) {
                                 auto price         = findPrice(offsets, changeSequence);
                                 reachablePriceSum -= averagePricePerSecret - price;
                                 if ( reachablePriceSum < bestPriceSum ) {
                                     break;
                                 } //if ( reachablePriceSum < bestPriceSum )
                                 priceSum += price;
                             } //for ( const OffsetsAndPrices& offsets : offsetsAndPrices )
                             bestPriceSum = std::max(bestPriceSum, priceSum);
                             auto dur     = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - now);
                             myPrint("Change Sequence {:6d} done after {:7.2f}ms with {:4d} (Best {:4d})\n", ++i,
                                     dur.count() / 1000., priceSum, bestPriceSum);
                             return priceSum;
                         }));
    myPrint(" == Result of Part 2: {:d} ==\n", sumDisintegrateable);

    return sum1 == 15'608'699'004 && sumDisintegrateable == 1791;
}
