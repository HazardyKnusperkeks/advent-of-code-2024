#include "challenge19.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_map>

namespace {
auto parseTowels(std::string_view line) noexcept {
    auto dropSpace = [](std::string_view v) noexcept {
        if ( v.starts_with(' ') ) {
            v.remove_prefix(1);
        } //if ( v.starts_with(' ') )
        return v;
    };
    auto ret = splitString(line, ',') | std::views::transform(dropSpace) | std::ranges::to<std::vector>();
    std::ranges::sort(ret);
    return ret;
}

bool isPossible(const std::string_view design, std::span<const std::string_view> towels) noexcept {
    for ( auto towel : towels ) {
        if ( design.starts_with(towel) ) {
            auto remainingDesign = design;
            remainingDesign.remove_prefix(towel.size());

            if ( remainingDesign.empty() ) {
                return true;
            } //if ( remainingDesign.empty() )

            if ( isPossible(remainingDesign, towels) ) {
                return true;
            } //if ( isPossible(remainingDesign, towels) )
        } //if ( design.starts_with(towel) )
    } //for ( auto towel : towels )
    return false;
}

std::int64_t possibilities(const std::string_view design, const std::span<const std::string_view> fullTowelRange,
                           const std::ranges::borrowed_subrange_t<std::span<const std::string_view>> towelRange,
                           std::size_t index = 0) noexcept {
    static std::unordered_map<std::string_view, std::int64_t> cache;

    if ( index == 0 ) {
        if ( auto iter = cache.find(design); iter != cache.end() ) {
            return iter->second;
        } //if ( auto iter = cache.find(design); iter != cache.end() )
    } //if ( index == 0 )

    std::int64_t sum           = 0;
    auto         projection    = [index](std::string_view v) noexcept { return v[index]; };
    auto         towelSubRange = std::ranges::equal_range(towelRange, design.front(), {}, projection);

    if ( towelSubRange.empty() ) {
        return 0;
    } //if ( towelSubRange.empty() )

    if ( towelSubRange.front().size() == index + 1 ) {
        if ( design.size() == 1 ) {
            return 1;
        } //if ( design.size() == 1 )

        sum           = possibilities(design.substr(1), fullTowelRange, fullTowelRange);
        towelSubRange = towelSubRange | std::views::drop(1);
    } //if ( towelSubRange.front() == index + 1 )

    if ( design.size() > 1 ) {
        sum += possibilities(design.substr(1), fullTowelRange, towelSubRange, index + 1);
    } //if ( design.size() > 1 )

    if ( index == 0 ) {
        cache.emplace(design, sum);
    } //if ( index == 0 )
    return sum;
}

} //namespace

bool challenge19(const std::vector<std::string_view>& input) {
    const auto towels  = parseTowels(input.front());
    const auto designs = std::span{input}.subspan(2);

    auto isPossible    = [&towels](std::string_view design) noexcept { return ::isPossible(design, towels); };
    auto possibilities = [&towels](std::string_view design) noexcept {
        std::span towelsSpan{towels};
        return ::possibilities(design, towelsSpan, towelsSpan);
    };

    auto       possibleDesigns = designs | std::views::filter(isPossible) | std::ranges::to<std::vector>();
    const auto sum1            = possibleDesigns.size();
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 = std::ranges::fold_left(possibleDesigns | std::views::transform(possibilities), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 220 && sum2 == 565'600'047'715'343;
}
