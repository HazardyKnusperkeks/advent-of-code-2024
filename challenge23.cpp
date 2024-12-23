#include "challenge23.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <coroutine>
#include <generator>
#include <ranges>
#include <unordered_map>

using namespace std::string_view_literals;

namespace {
using NetworkMap = std::unordered_map<std::string_view, std::vector<std::string_view>>;

auto buildMap(std::span<const std::string_view> input) {
    NetworkMap map;

    for ( auto line : input ) {
        const auto dash  = line.find('-');
        auto       left  = line.substr(0, dash);
        auto       right = line.substr(dash + 1);
        map[left].push_back(right);
        map[right].push_back(left);
    } //for ( auto line : input )

    for ( auto& [name, neighbors] : map ) {
        std::ranges::sort(neighbors);
    } //for ( auto& [name, neighbors] : map )
    return map;
}

bool startsWithT(std::string_view view) noexcept {
    return view.starts_with('t');
}

NetworkMap pruneMap(NetworkMap map) noexcept {
    auto iter = map.begin();
    while ( iter != map.end() ) {
        if ( startsWithT(iter->first) ) {
            std::erase_if(iter->second, [iter](std::string_view neighbor) noexcept {
                return startsWithT(neighbor) && neighbor < iter->first;
            });
            ++iter;
        } //if ( startsWithT(iter->first).starts_with('t') )
        else if ( std::ranges::any_of(iter->second, startsWithT) ) {
            auto& [name, neigbors] = *iter;
            auto toErase           = std::ranges::lower_bound(neigbors, name);
            neigbors.erase(neigbors.begin(), toErase);
            if ( neigbors.empty() ) {
                iter = map.erase(iter);
            } //if ( neigbors.empty() )
            else {
                ++iter;
            } //else -> if ( neigbors.empty() )
        } //else if ( std::ranges::any_of(iter->second, startsWithT) )
        else {
            iter = map.erase(iter);
        } //else -> if ( startsWithT(iter->first).starts_with('t') || std::ranges::any_of(iter->second, startsWithT) )
    } //while ( iter != map.end() )
    return map;
}

template<typename Range>
std::generator<std::pair<const typename Range::value_type&, const typename Range::value_type&>>
symmetricCartesianProduct(const Range& range) noexcept {
    auto begin = std::ranges::begin(range);
    auto end   = std::ranges::end(range);

    for ( auto i = begin; i != end; ++i ) {
        for ( auto j = std::next(i); j != end; ++j ) {
            co_yield std::pair{*i, *j};
        } //for ( auto j = std::next(i); j != end; ++j )
    } //for ( auto i = begin; i != end; ++i )
}

auto countThreeCliquesWithT(const NetworkMap& map) noexcept {
    auto isClique = [&map](const NetworkMap::value_type& node) {
        const auto& [name, connectedNodes] = node;
        return std::ranges::count_if(symmetricCartesianProduct(connectedNodes),
                                     [&map](std::pair<std::string_view, std::string_view> neighborNodes) {
                                         auto iter = map.find(neighborNodes.first);
                                         if ( iter == map.end() ) {
                                             return false;
                                         } //if ( iter == map.end() )
                                         return std::ranges::binary_search(iter->second, neighborNodes.second);
                                     });
    };
    return std::ranges::fold_left(map | std::views::filter([](const NetworkMap::value_type& node) noexcept {
                                      return startsWithT(node.first);
                                  }) | std::views::transform(isClique),
                                  0, std::plus<>{});
}

std::vector<std::string_view> addElement(std::vector<std::string_view> container, std::string_view element) noexcept {
    container.insert(std::ranges::lower_bound(container, element), element);
    return container;
}

std::vector<std::string_view> findCliqueOf(const NetworkMap& map, std::string_view candidate,
                                           std::size_t lowerBoundOnClique) noexcept {
    const auto&                   neighbors = map.find(candidate)->second;
    std::vector<std::string_view> ret;
    const auto                    cliqueCandidateTemplate = addElement(neighbors, candidate);
    const auto                    maxCliqueSize           = cliqueCandidateTemplate.size();
    const auto                    candidateIndex          = static_cast<std::size_t>(std::ranges::distance(
        cliqueCandidateTemplate.begin(), std::ranges::lower_bound(cliqueCandidateTemplate, candidate)));

    auto increment = [maxCliqueSize, candidateIndex](std::vector<std::size_t>& indices) noexcept {
        /* This is not optimal, but for this problem fast enough. After the first overlow we start with (0, 0), which is
         * rather silly. And after that it does the whole cartesian product. */
        bool overflown = true;
        for ( auto iter = indices.begin(); iter != indices.end() && overflown; ++iter ) {
            overflown   = false;
            auto& index = *iter;
            ++index;

            if ( index == candidateIndex ) {
                ++index;
            } //if ( index == candidateIndex )

            if ( index == maxCliqueSize ) {
                index     = 0;
                overflown = true;
            } //if ( index == maxCliqueSize )
        } //for ( auto iter = indices.begin(); iter != indices.end() && overflown; ++iter )

        if ( overflown ) {
            indices.push_back(0);
        } //if ( overflown)
        return;
    };

    for ( std::vector<std::size_t> indicesToSkip; maxCliqueSize - indicesToSkip.size() > lowerBoundOnClique;
          increment(indicesToSkip) ) {
        auto cliqueCandidate = cliqueCandidateTemplate;
        for ( auto [index, neighbor] : neighbors | std::views::enumerate ) {
            if ( std::ranges::contains(indicesToSkip, index) ) {
                continue;
            } //if ( std::ranges::contains(indicesToSkip, index) )

            //This is undefined, but works!
            auto cliqueEnd =
                std::ranges::set_intersection(cliqueCandidate, addElement(map.find(neighbor)->second, neighbor),
                                              cliqueCandidate.begin())
                    .out;
            cliqueCandidate.erase(cliqueEnd, cliqueCandidate.end());

            if ( cliqueCandidate.size() <= lowerBoundOnClique ) {
                break;
            } //if ( cliqueCandidate.size() <= lowerBoundOnClique )
        } //for ( auto neighbor : neighbors | std::views::drop(index) )

        if ( cliqueCandidate.size() > ret.size() ) {
            ret                = std::move(cliqueCandidate);
            lowerBoundOnClique = ret.size();
        } //if ( cliqueCandidate.size() > ret.size() )
    } //for ( indicesToSkip; maxCliqueSize - indicesToSkip.size() > lowerBoundOnClique; increment(indicesToSkip) )
    return ret;
}

std::string findBiggestClique(NetworkMap& map) noexcept {
    const auto nameProjection  = &std::pair<std::string_view, std::size_t>::first;
    const auto countProjection = &std::pair<std::string_view, std::size_t>::second;
    auto       neighborCount   = map | std::views::transform([](const NetworkMap::value_type& entry) noexcept {
                             return std::pair{entry.first, entry.second.size()};
                         }) |
                         std::ranges::to<std::vector>();
    std::ranges::sort(neighborCount, {}, countProjection);

    std::vector<std::string_view> biggestCliqueMembers{"Foo"sv, "Bar"sv};
    while ( true ) {
        auto [candidate, numberOfNeighbors] = neighborCount.back();
        neighborCount.pop_back();

        if ( numberOfNeighbors <= biggestCliqueMembers.size() ) {
            break;
        } //if ( numberOfNeighbors <= biggestCliqueMembers.size() )

        auto newClique = findCliqueOf(map, candidate, biggestCliqueMembers.size());
        if ( newClique.size() > biggestCliqueMembers.size() ) {
            biggestCliqueMembers = std::move(newClique);
        } //if ( newClique.size() > biggestCliqueMembers.size() )

        auto        iter      = map.find(candidate);
        const auto& neighbors = iter->second;

        for ( auto neighbor : neighbors ) {
            auto& neighborNeighbors = map[neighbor];
            neighborNeighbors.erase(std::ranges::lower_bound(neighborNeighbors, candidate));

            auto positionOfNeighbor = std::ranges::find(neighborCount, neighbor, nameProjection);
            --positionOfNeighbor->second;
            auto newPositionOfNeighbor = std::ranges::upper_bound(neighborCount.begin(), positionOfNeighbor,
                                                                  positionOfNeighbor->second, {}, countProjection);
            std::ranges::iter_swap(positionOfNeighbor, newPositionOfNeighbor);
        } //for ( auto neighbor : neighbors )

        map.erase(iter);
    } //while ( true )

    // std::ranges::sort(biggestCliqueMembers);
    std::string ret{biggestCliqueMembers.front()};
    for ( auto member : biggestCliqueMembers | std::views::drop(1) ) {
        ret += ',';
        ret += member;
    } //for ( auto member : biggestCliqueMembers | std::views::drop(1) )
    return ret;
}
} //namespace

bool challenge23(const std::vector<std::string_view>& input) {
    auto       map             = buildMap(input);
    auto       p1Map           = pruneMap(map);
    const auto numberOfCliques = countThreeCliquesWithT(p1Map);
    myPrint(" == Result of Part 1: {:d} ==\n", numberOfCliques);

    const auto biggestClique = findBiggestClique(map);
    myPrint(" == Result of Part 2: {:s} ==\n", biggestClique);

    return numberOfCliques == 1230 && biggestClique == "abcd"sv;
}
