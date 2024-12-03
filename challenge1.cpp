#include "challenge1.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <span>

namespace {
std::int64_t distance(std::tuple<std::int64_t, std::int64_t> input) noexcept {
    const auto [left, right] = input;
    return std::abs(left - right);
}

std::int64_t dropSame(std::span<std::int64_t>& data) noexcept {
    auto firstDifferent =
        std::ranges::find_if(data, [data](std::int64_t number) noexcept { return number != data.front(); });
    const std::int64_t ret = std::ranges::distance(data.begin(), firstDifferent);
    data                   = {firstDifferent, data.end()};
    return ret;
}
} //namespace

bool challenge1(const std::vector<std::string_view>& input) {
    std::vector<std::int64_t> left;
    std::vector<std::int64_t> right;
    left.resize(input.size());
    right.resize(input.size());

    for ( auto&& [l, r, line] : std::views::zip(left, right, input) ) {
        auto split = splitString(line, ' ');
        throwIfInvalid(std::ranges::distance(split) == 2);
        l = convert(*split.begin());
        r = convert(*std::next(split.begin()));
    } //for (auto&& [l, r, line] : std::views::zip(left, right, input))

    std::ranges::sort(left);
    std::ranges::sort(right);

    auto       distances = std::views::zip(left, right) | std::views::transform(distance);
    const auto sum1      = std::ranges::fold_left(distances, 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto calculateSimularity = [rightSpan = std::span{right}](std::int64_t number) mutable noexcept -> std::int64_t {
        if ( rightSpan.empty() ) {
            return 0;
        } //if ( rightSpan.empty() )

        while ( rightSpan.front() < number ) {
            dropSame(rightSpan);
            if ( rightSpan.empty() ) {
                return 0;
            } //if ( rightSpan.empty() )
        } //while ( rightSpan.front() < number )

        if ( rightSpan.front() > number ) {
            return 0;
        } //if ( rightSpan.front() > number )

        return number * dropSame(rightSpan);
    };

    auto       simularity = left | std::views::transform(calculateSimularity);
    const auto sum2       = std::ranges::fold_left(simularity, 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 1765812 && sum2 == 20520794;
}
