#include "challenge11.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <unordered_map>

namespace {
struct Stone {
    std::int64_t Value;
    std::int64_t TimesToConvert;

    bool operator==(const Stone&) const noexcept = default;
};

struct StoneHash {
    static std::size_t operator()(const Stone& stone) noexcept {
        std::hash<std::int64_t> h;
        return h(stone.Value << 6 | stone.TimesToConvert);
    }
};

using Cache = std::unordered_map<Stone, std::int64_t, StoneHash>;

struct BlinkResult {
    Stone FirstStone;
    Stone SecondStone;
};

static constexpr std::int64_t NoStone = -1;

auto calcNumberOfDigits(std::int64_t number) noexcept {
    static constexpr auto logTable = [](void) noexcept {
        std::array<std::int64_t, 16> ret;
        ret[0] = 1;
        std::ranges::generate(ret | std::views::drop(1), [log = std::int64_t{1}](void) mutable noexcept {
            log *= 10;
            return log;
        });
        return ret;
    }();
    const auto log = std::ranges::upper_bound(logTable, number);
    throwIfInvalid(log != logTable.end(), "Increase Log-Table");
    return std::ranges::distance(logTable.begin(), log);
}

BlinkResult blink(Stone stone) noexcept {
    const auto  toConvert = stone.TimesToConvert - 1;
    BlinkResult ret{{stone.Value, toConvert}, {NoStone, toConvert}};

    if ( stone.Value == 0 ) {
        ret.FirstStone.Value = 1;
    } //if ( stone.Value == 0 )
    else if ( auto numDigits = calcNumberOfDigits(stone.Value); numDigits % 2 == 0 ) {
        auto divider = 1;
        for ( numDigits /= 2; numDigits; --numDigits ) {
            divider *= 10;
        } //for ( numDigits /= 2; numDigits; --numDigits )
        ret.FirstStone.Value  /= divider;
        ret.SecondStone.Value  = stone.Value % divider;
    } //else if ( auto numDigits = calcNumberOfDigits(stone.Value); numDigits % 2 == 0 )
    else {
        ret.FirstStone.Value *= 2024;
    } //else
    return ret;
}
} //namespace

bool challenge11(const std::vector<std::string_view>& input) {
    throwIfInvalid(input.size() == 1);
    auto stones = splitString(input.front(), ' ') |
                  std::views::transform([](std::string_view value) noexcept { return Stone{convert(value), 25}; }) |
                  std::ranges::to<std::vector>();

    Cache cache;
    auto  calcStonesAfterBlinking = [&cache](this auto& self, const Stone& stone) noexcept -> std::int64_t {
        if ( stone.TimesToConvert == 0 ) {
            return 1;
        } //if ( stone.TimesToConvert == 0 )

        auto iter = cache.find(stone);
        if ( iter != cache.end() ) {
            return iter->second;
        } //if ( iter != cache.end() )

        const auto blinkResult = blink(stone);
        auto       ret         = self(blinkResult.FirstStone);
        if ( blinkResult.SecondStone.Value != NoStone ) {
            ret += self(blinkResult.SecondStone);
        } //if ( blinkResult.SecondStone.Value != NoStone )

        cache.insert({stone, ret});
        return ret;
    };

    const auto sum1 = std::ranges::fold_left(stones | std::views::transform(calcStonesAfterBlinking), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::ranges::fill(stones | std::views::transform(&Stone::TimesToConvert), 75);
    const auto sum2 = std::ranges::fold_left(stones | std::views::transform(calcStonesAfterBlinking), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 197157 && sum2 == 234'430'066'982'597;
}
