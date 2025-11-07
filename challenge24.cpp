#include "challenge24.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <bitset>
#include <functional>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

using namespace std::string_view_literals;

namespace {
enum OperationKind { And, Or, Xor };

OperationKind toOperation(std::string_view op) noexcept {
    switch ( op.front() ) {
        case 'A' : return And;
        case 'O' : return Or;
    } //switch ( op.front() )
    return Xor;
}

struct Rule {
    std::string_view LHS;
    std::string_view RHS;
    OperationKind    Operation;
};

struct Data {
    std::unordered_map<std::string_view, bool> TruthValues;
    std::unordered_map<std::string_view, Rule> Rules;
    std::size_t                                Bits;

    std::unordered_set<std::string_view> LookingFor;

    bool solveFor(std::string_view output) {
        LookingFor.clear();
        return valueOf(output);
    }

    bool valueOf(std::string_view wire) {
        if ( auto iter = TruthValues.find(wire); iter != TruthValues.end() ) {
            return iter->second;
        } //if ( auto iter = TruthValues.find(wire); iter != TruthValues.end() )

        if ( !LookingFor.insert(wire).second ) {
            throw wire;
        } //if ( !LookingFor.insert(wire).second )
        const auto& rule  = Rules[wire];
        auto        value = executeRule(rule);
        TruthValues.emplace(wire, value);
        LookingFor.erase(wire);

        return value;
    }

    bool executeRule(const Rule& rule) {
        switch ( rule.Operation ) {
            case And : return valueOf(rule.LHS) && valueOf(rule.RHS);
            case Or  : return valueOf(rule.LHS) || valueOf(rule.RHS);
            case Xor : break;
        } //switch ( rule.Operation )
        return valueOf(rule.LHS) != valueOf(rule.RHS);
    }

    std::unordered_set<std::string_view>
    buildDepenencies(std::string_view output, const std::unordered_set<std::string_view> ignore) const noexcept {
        std::unordered_set<std::string_view> ret;
        auto add = [&rules = Rules, &ret, &ignore](this auto& self, std::string_view wire) noexcept {
            if ( ignore.contains(wire) ) {
                return;
            } //if ( ignore.contains(wire) )
            auto iter = rules.find(wire);

            if ( iter == rules.end() ) {
                return;
            } //if ( iter == rules.end() )

            if ( ret.insert(iter->first).second ) {
                const auto& rule = iter->second;
                self(rule.LHS);
                self(rule.RHS);
            } //if ( ret.insert(iter->first).second )
            return;
        };
        add(output);
        return ret;
    }
};

Data parse(std::span<const std::string_view> input) noexcept {
    Data ret;
    auto firstRange   = input | std::views::take_while(std::not_fn(&std::string_view::empty));
    auto continueIter = std::ranges::transform(firstRange, std::inserter(ret.TruthValues, ret.TruthValues.end()),
                                               [](std::string_view line) noexcept {
                                                   return std::pair{line.substr(0, 3), line.back() == '1'};
                                               })
                            .in;
    ret.Rules = std::ranges::subrange{std::next(continueIter), input.end()} |
                std::views::transform([](std::string_view line) noexcept {
                    auto splitRange = splitString(line, ' ');
                    auto iter       = splitRange.begin();
                    Rule rule;
                    rule.LHS       = *iter;
                    rule.Operation = toOperation(*++iter);
                    rule.RHS       = *++iter;
                    return std::pair{*std::next(iter, 2), rule};
                }) |
                std::ranges::to<std::unordered_map>();
    ret.Bits = static_cast<std::size_t>(std::ranges::distance(
        ret.Rules | std::views::keys |
        std::views::filter([](std::string_view wire) noexcept { return wire.starts_with('z'); })));
    return ret;
}

auto findTheNumber(Data& data, std::size_t bits) {
    std::bitset<64> theNumber;

    std::string      numberData{"z00"};
    std::string_view numberName{numberData};

    auto incZ = [&numberData](void) noexcept {
        if ( ++numberData[2] == '9' + 1 ) {
            ++numberData[1];
            numberData[2] = '0';
        } //if ( ++z[2] == '9' + 1 )
        return;
    };

    for ( auto i = 0u; i < bits; ++i, incZ() ) {
        auto outputValue = data.solveFor(numberName);
        theNumber.set(i, outputValue);
    } //for ( auto i = 0u; i < bits; ++i, incZ() )

    return theNumber.to_ullong();
}

std::string findOutputsToSwap(Data& data) noexcept {
    unsigned long long x = 1;

    auto truthIter       = data.TruthValues.begin();
    while ( truthIter != data.TruthValues.end() ) {
        if ( truthIter->first.starts_with('x') || truthIter->first.starts_with('y') ) {
            truthIter->second = false;
            ++truthIter;
        } //if ( truthIter->first.starts_with('x') || truthIter->first.starts_with('y') )
        else {
            truthIter = data.TruthValues.erase(truthIter);
        } //else -> if ( truthIter->first.starts_with('x') || truthIter->first.starts_with('y') )
    } //while ( truthIter != data.TruthValues.end() )

    const auto                           allZero = data.TruthValues;
    std::unordered_set<std::string_view> fixedGates;
    std::vector<std::string_view>        swappedGates;
    std::string                          oldName;

    for ( auto i = 0u; i < data.Bits; ++i, x <<= 1 ) {
        data.TruthValues         = allZero;
        std::string thisName     = std::format("z{:02d}", i);
        const auto  dependencies = data.buildDepenencies(thisName, fixedGates);

        thisName[0]              = 'x';
        const auto xName         = thisName;
        thisName[0]              = 'y';
        const auto yName         = thisName;

        auto check               = [&data, &allZero, &i, &xName, &yName, &oldName](void) noexcept {
            auto boolRange = std::views::cartesian_product(std::views::iota(0u, 2u), std::views::iota(0u, 2u),
                                                                         std::views::iota(0u, 2u));
            for ( auto values : boolRange ) {
                data.TruthValues                                                                      = allZero;
                std::tie(data.TruthValues[xName], data.TruthValues[yName], data.TruthValues[oldName]) = values;

                try {
                    const auto result   = findTheNumber(data, i + 2);
                    const auto expected = (((std::get<0>(values) + std::get<1>(values)) /*% 2*/) << i) +
                                          (i == 0 ? 0 : std::get<2>(values) << (i - 1));
                    myPrint("i: {:d}: {:d},{:d},{:d} => Result {:d}, Expected {:d}\n", i, std::get<0>(values),
                                          std::get<1>(values), std::get<2>(values), result, expected);
                    myFlush();
                    if ( result != expected ) {
                        return false;
                    } //if ( result != expected )
                } //try
                catch ( ... ) {
                    return false;
                } //catch ( ... )
            } //for ( auto values : boolRange )
            return true;
        };

        //const auto truthValues = data.TruthValues;
        //auto       result      = findTheNumber(data, i + 1);

        if ( !check() ) {
            myPrint("Error: ");
            bool c = false;

            for ( auto [gate1, gate2] : symmetricCartesianProduct(dependencies) ) {
                std::swap(data.Rules[gate1], data.Rules[gate2]);

                if ( check() ) {
                    swappedGates.push_back(gate1);
                    swappedGates.push_back(gate2);
                    myPrint("Corrected!\n");
                    c = true;
                    break;
                } //if ( check() )
                std::swap(data.Rules[gate1], data.Rules[gate2]);
            } //for ( auto [gate1, gate2] : symmetricCartesianProduct(dependencies) )

            if ( !c ) {
                myPrint("Not Fixed!!\n");
            }
        } //if ( result != x )

        oldName    = thisName;
        oldName[0] = 'z';
        fixedGates.insert(dependencies.begin(), dependencies.end());
        //const auto thisDependencies = data.buildDepenencies(zView);
        //std::ranges::copy(thisDependencies, std::back_inserter(dependencies));
        myPrint("{:d} done\n", i);
        myFlush();
    } //for ( auto i = 0u; i < data.Bits; ++i )

    //std::ranges::sort(dependencies);
    //auto toErase = std::ranges::unique(dependencies);
    //dependencies.erase(toErase.begin(), toErase.end());
    ////dependencies = data.Rules | std::views::keys | std::ranges::to<std::vector>();
    ////std::ranges::sort(dependencies, std::greater<>{});
    //auto clearAndSwap = [&data](std::string_view first, std::string_view second) noexcept {
    //    data.clearTruthValue(first);
    //    data.clearTruthValue(second);
    //    std::swap(data.Rules[first], data.Rules[second]);
    //    return;
    //};

    //for ( auto [first, second] : symmetricCartesianProduct(dependencies) ) {
    //    clearAndSwap(first, second);
    //    auto suitableForSecondPair = [first, second](std::string_view wire) noexcept {
    //        return wire != first && wire != second;
    //    };
    //    for ( auto x = dependencies | std::views::filter(suitableForSecondPair);
    //          auto [third, fourth] : symmetricCartesianProduct(x) ) {
    //        clearAndSwap(third, fourth);
    //        auto suitableForThirdPair = [suitableForSecondPair, third, fourth](std::string_view wire) noexcept {
    //            return suitableForSecondPair(wire) && wire != third && wire != fourth;
    //        };
    //        for ( auto y = dependencies | std::views::filter(suitableForThirdPair);
    //              auto [fifth, sixth] : symmetricCartesianProduct(y) ) {
    //            clearAndSwap(fifth, sixth);
    //            auto suitableForFourthPair = [suitableForThirdPair, fifth, sixth](std::string_view wire) noexcept {
    //                return suitableForThirdPair(wire) && wire != fifth && wire != sixth;
    //            };
    //            for ( auto z = dependencies | std::views::filter(suitableForFourthPair);
    //                  auto [seventh, eighth] : symmetricCartesianProduct(z) ) {
    //                try {
    //                    clearAndSwap(seventh, eighth);
    //                    auto theNumber = findTheNumber(data, expectedResult);
    //                    clearAndSwap(seventh, eighth);
    //                    if ( theNumber == expectedResult ) {
    //                        myPrint("Number {:d} Expected {:d}\n", theNumber, expectedResult);
    //                        std::array wires{first, second, third, fourth, fifth, sixth, seventh, eighth};
    //                        std::ranges::sort(wires);
    //                        std::string ret{wires.front()};
    //                        for ( auto wire : wires | std::views::drop(1) ) {
    //                            ret += ',';
    //                            ret += wire;
    //                        } //for ( auto wire : wires | std::views::drop(1) )
    //                        return ret;
    //                    } //if ( theNumber == expectedResult )
    //                } //try
    //                catch ( const std::string_view& thrownView ) {
    //                    clearAndSwap(seventh, eighth);
    //                    if ( thrownView == first || thrownView == second ) {
    //                        //Skip innter loop, since the outer results in an endless loop evaluation.
    //                        break;
    //                    } //if ( thrownView == first || thrownView == second )
    //                } //catch ( const std::string_view& thrownView )
    //            } //for ( auto [seventh, eighth] : scp(dependencies | std::views::filter(suitableForThirdPair) )
    //            clearAndSwap(fifth, sixth);
    //        } //for ( auto [fifth, sixth] : scp(dependencies | std::views::filter(suitableForThirdPair) )
    //        static int i = 0;
    //        clearAndSwap(third, fourth);
    //        myPrint("2nd Outer Loop #{:d} done\n", ++i);
    //        myFlush();
    //    } //for ( auto [third, fourth] : symmetricCartesianProduct(dependencies |
    //    views::filter(suitableForSecondPair))) static int i = 0; clearAndSwap(first, second); myPrint("1st Outer Loop
    //    #{:d} done\n", ++i); myFlush();
    //} //for ( auto [first, second] : symmetricCartesianProduct(dependencies) )

    std::ranges::sort(swappedGates);
    std::string ret{swappedGates.front()};
    for ( auto wire : swappedGates | std::views::drop(1) ) {
        ret += ',';
        ret += wire;
    } //for ( auto wire : swappedGates | std::views::drop(1) )
    return ret;
}
} //namespace

bool challenge24(const std::vector<std::string_view>& input) {
    auto data            = parse(input);

    const auto theNumber = findTheNumber(data, data.Bits);
    myPrint(" == Result of Part 1: {:d} ==\n", theNumber);

    const auto outputsToSwap = findOutputsToSwap(data);
    myPrint(" == Result of Part 2: {:s} ==\n", outputsToSwap);

    return theNumber == 51'657'025'112'326 && outputsToSwap == "gbf,hdt,jgt,mht,nbf,z05,z09,z30"sv;
}
