#include "challenge13.hpp"

#include "helper.hpp"
#include "print.hpp"
#include "3rdParty/ctre/include/ctre.hpp"

#include <algorithm>
#include <ranges>

namespace {
struct Equation {
    std::int64_t AFactor;
    std::int64_t BFactor;
    std::int64_t Result;

    Equation& operator-=(const Equation& that) noexcept {
        AFactor -= that.AFactor;
        BFactor -= that.BFactor;
        Result  -= that.Result;
        return *this;
    }

    Equation operator-(const Equation& that) const noexcept {
        auto ret{*this};
        return ret -= that;
    }

    Equation& operator/=(std::int64_t divider) noexcept {
        AFactor /= divider;
        BFactor /= divider;
        Result  /= divider;
        return *this;
    }

    friend Equation operator*(std::int64_t factor, Equation eq) noexcept {
        eq.AFactor *= factor;
        eq.BFactor *= factor;
        eq.Result  *= factor;
        return eq;
    }
};

struct ClawMachine {
    Equation A;
    Equation B;

    std::int64_t costOfWinning(void) const noexcept {
        auto b = A.AFactor * B - B.AFactor * A;

        if ( b.Result % b.BFactor != 0 ) {
            return 0;
        } //if ( b.Result % b.BFactor != 0  )

        b      /= b.BFactor;

        auto a  = A - A.BFactor * b;

        if ( a.Result % a.AFactor != 0 ) {
            return 0;
        } //if ( a.Result % a.AFactor != 0  )

        a /= a.AFactor;

        return 3 * a.Result + b.Result;
    }

    void bumpForPart2(void) noexcept {
        A.Result += 10'000'000'000'000;
        B.Result += 10'000'000'000'000;
        return;
    }
};

auto parse(const std::vector<std::string_view>& input) {
    auto buttonRegEx = ctre::match<R"(Button .: X\+(\d+), Y\+(\d+))">;
    auto prizeRegEx  = ctre::match<R"(Prize: X=(\d+), Y=(\d+))">;

    std::vector<ClawMachine> ret;
    for ( auto machineInput : input | std::views::chunk(4) ) {
        throwIfInvalid(std::ranges::distance(machineInput) >= 3);
        auto iter       = std::ranges::begin(machineInput);
        auto aMatch     = buttonRegEx(*iter);
        auto bMatch     = buttonRegEx(*++iter);
        auto prizeMatch = prizeRegEx(*++iter);

        throwIfInvalid(aMatch && bMatch && prizeMatch);

        auto matchesToEquation = [&]<std::size_t I>(void) noexcept {
            return Equation{convert(aMatch.get<I>().view()), convert(bMatch.get<I>().view()),
                            convert(prizeMatch.get<I>().view())};
        };

        ret.push_back(ClawMachine{matchesToEquation.operator()<1>(), matchesToEquation.operator()<2>()});
    } //for ( auto inputLine : input )
    return ret;
}
} //namespace

bool challenge13(const std::vector<std::string_view>& input) {
    auto clawMachines = parse(input);

    const auto sum1 =
        std::ranges::fold_left(clawMachines | std::views::transform(&ClawMachine::costOfWinning), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    std::ranges::for_each(clawMachines, &ClawMachine::bumpForPart2);
    const auto sum2 =
        std::ranges::fold_left(clawMachines | std::views::transform(&ClawMachine::costOfWinning), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 37901 && sum2 == 77'407'675'412'647;
}
