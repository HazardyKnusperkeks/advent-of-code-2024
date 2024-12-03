#include "challenge2.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <functional>
#include <ranges>

namespace {
bool isSafe(std::span<const std::int64_t> distances) noexcept {
    return std::ranges::all_of(distances, [](auto level) noexcept { return level >= 1 && level <= 3; }) ||
           std::ranges::all_of(distances, [](auto level) noexcept { return level >= -3 && level <= -1; });
}

bool isDampenedSafeImpl(const std::vector<std::int64_t>& distances, bool alreadyInverted) noexcept {
    if ( distances.front() == 0 ) {
        return isSafe(std::span{distances}.subspan(1));
    } //if ( distances.front() == 0 )

    auto firstBad =
        std::ranges::find_if(distances, [](auto distance) noexcept { return distance < 1 || distance > 3; });

    if ( firstBad == distances.begin() ) {
        if ( isSafe(std::span{distances}.subspan(1)) ) {
            return true;
        } //if ( isSafe(std::span{distances}.subspan(1)) )
    } //if ( firstBad == distances.begin() )
    else if ( firstBad == std::prev(distances.end()) ) {
        if ( isSafe(std::span{distances}.subspan(0, distances.size() - 1)) ) {
            return true;
        } //if ( isSafe(std::span{distances}.subspan(0, distances.size() - 1)) ) {
    } //if ( firstBad == std::prev(distances.end()) )
    /*else */{
        const auto index = static_cast<int>(std::ranges::distance(distances.begin(), firstBad));

        for ( auto toRemoveIndex : {std::max(index - 1, 0), index} ) {
            auto       copy      = distances;
            auto       toRemove  = std::next(copy.begin(), toRemoveIndex);
            const auto carry     = *toRemove;
            auto       toMerge   = copy.erase(toRemove);
            *toMerge            += carry;
            if ( isSafe(copy) ) {
                return true;
            }
        } //
    } //else

    if ( !alreadyInverted ) {
        std::vector<std::int64_t> copy;
        copy.resize(distances.size());
        std::ranges::transform(distances, copy.begin(), [](auto distance) noexcept { return -distance; });
        return isDampenedSafeImpl(copy, /*alreadyInverted=*/true);
    } //if ( !alreadyInverted )

    myPrint("Not Fixable:");
    for ( auto d : distances ) {
        myPrint(" {:}", -d);
    }
    myPrint("\n");
    return false;
}

bool isDampenedSafe(const std::vector<std::int64_t>& distances) noexcept {
    return isDampenedSafeImpl(distances, /*alreadyInverted=*/false);
}
} //namespace

bool challenge2(const std::vector<std::string_view>& input) {
    std::vector reportsDifferences =
        input | std::views::transform([](auto line) noexcept {
            return splitString(line, ' ') |
                   std::views::transform([](std::string_view level) noexcept { return convert(level); }) |
                   std::views::slide(2) | std::views::transform([](auto window) noexcept {
                       return *std::next(window.begin()) - *window.begin();
                   }) |
                   std::ranges::to<std::vector<std::int64_t>>();
        }) |
        std::ranges::to<std::vector>();
    auto       unsafeReports       = reportsDifferences | std::views::filter(std::not_fn(isSafe));
    const auto numberOfSafeReports = std::ssize(input) - std::ranges::distance(unsafeReports);

    myPrint(" == Result of Part 1: {:d} ==\n", numberOfSafeReports);

    const auto numberOfDampenedSafeReports = std::ranges::count_if(unsafeReports, isDampenedSafe);

    const auto sum2                        = numberOfSafeReports + numberOfDampenedSafeReports;
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return numberOfSafeReports == 559 && sum2 == 601;
}
