#ifndef HELPER_HPP
#define HELPER_HPP

#include <charconv>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

enum class Direction { Up = 1 << 0, Down = 1 << 1, Left = 1 << 2, Right = 1 << 3 };

inline Direction turnRight(Direction dir) noexcept {
    switch ( dir ) {
        using enum Direction;
        case Up    : return Right;
        case Down  : return Left;
        case Left  : return Up;
        case Right : return Down;
    } //switch ( dir )
    std::unreachable();
}

inline Direction turnLeft(Direction dir) noexcept {
    switch ( dir ) {
        using enum Direction;
        case Up    : return Left;
        case Down  : return Right;
        case Left  : return Down;
        case Right : return Up;
    } //switch ( dir )
    std::unreachable();
}

inline Direction turnAround(Direction dir) noexcept {
    switch ( dir ) {
        using enum Direction;
        case Up    : return Down;
        case Down  : return Up;
        case Left  : return Right;
        case Right : return Left;
    } //switch ( dir )
    std::unreachable();
}

template<std::integral T>
struct CoordinateOffset {
    T Row;
    T Column;

    constexpr bool operator==(const CoordinateOffset&) const noexcept  = default;
    constexpr auto operator<=>(const CoordinateOffset&) const noexcept = default;

    CoordinateOffset operator*(T factor) const noexcept {
        auto ret{*this};
        ret *= factor;
        return ret;
    }

    CoordinateOffset& operator*=(T factor) noexcept {
        Row    *= factor;
        Column *= factor;
        return *this;
    }
};

struct MapView;

template<std::integral T>
struct Coordinate {
    T Row;
    T Column;

    static inline T MaxRow{};
    static inline T MaxColumn{};

    constexpr bool operator==(const Coordinate&) const noexcept  = default;
    constexpr auto operator<=>(const Coordinate&) const noexcept = default;

    Coordinate left(void) const noexcept {
        return {Row, Column - 1};
    }

    Coordinate right(void) const noexcept {
        return {Row, Column + 1};
    }

    Coordinate up(void) const noexcept {
        return {Row - 1, Column};
    }

    Coordinate down(void) const noexcept {
        return {Row + 1, Column};
    }

    bool isValid(void) const noexcept {
        return Row >= 0 && Row < MaxRow && Column >= 0 && Column < MaxColumn;
    }

    Coordinate& move(Direction where) noexcept {
        switch ( where ) {
            using enum Direction;
            case Up    : *this = up(); break;
            case Down  : *this = down(); break;
            case Left  : *this = left(); break;
            case Right : *this = right(); break;
        } //switch ( where )
        return *this;
    }

    Coordinate moved(Direction where) const noexcept {
        auto ret = *this;
        return ret.move(where);
    }

    CoordinateOffset<T> operator-(Coordinate that) const noexcept {
        return {Row - that.Row, Column - that.Column};
    }

    Coordinate operator+(CoordinateOffset<T> offset) const noexcept {
        auto ret{*this};
        ret += offset;
        return ret;
    }

    Coordinate& operator+=(CoordinateOffset<T> offset) noexcept {
        Row    += offset.Row;
        Column += offset.Column;
        return *this;
    }

    static void setMaxFromMap(MapView map);
};

namespace std {
template<typename T>
struct hash<Coordinate<T>> {
    size_t operator()(const Coordinate<T>& c) const noexcept {
        std::hash<T> h;
        return h(c.Row << 8) ^ h(c.Column);
    }
};
} //namespace std

struct MapView {
    const std::vector<std::string_view>& Base;

    MapView(const std::vector<std::string_view>& base) noexcept : Base{base} {
        return;
    }

    template<std::integral T>
    auto operator[](const Coordinate<T>& coordinate) const noexcept {
        return Base[static_cast<std::size_t>(coordinate.Row)][static_cast<std::size_t>(coordinate.Column)];
    }
};

template<bool SkipEmpty = true>
constexpr auto splitString(const std::string_view data, const char delimiter) noexcept {
    auto split = data | std::views::split(delimiter) | std::views::transform([](const auto& subRange) noexcept {
                     return std::string_view{&*subRange.begin(), std::ranges::size(subRange)};
                 });
    if constexpr ( SkipEmpty ) {
        return split | std::views::filter([](const std::string_view entry) noexcept { return !entry.empty(); });
    } //if constexpr ( SkipEmpty )
    else {
        return split;
    } //else -> if constexpr ( SkipEmpty )
}

void throwIfInvalid(bool valid, const char* msg = "Invalid Data");

template<int Base = 10>
inline std::optional<std::int64_t> convertOptionally(std::string_view input) {
    if ( Base == 10 && !std::isdigit(input[0]) && input[0] != '-' ) {
        return std::nullopt;
    } //if ( Base == 10 && !std::isdigit(input[0]) && input[0] != '-' )

    std::int64_t ret    = 0;
    auto         result = std::from_chars(input.begin(), input.end(), ret, Base);
    throwIfInvalid(result.ec == std::errc{});
    return result.ptr == input.data() ? std::nullopt : std::optional{ret};
}

template<int Base = 10>
inline std::int64_t convert(std::string_view input) {
    auto result = convertOptionally<Base>(input);
    throwIfInvalid(!!result);
    return *result;
}

inline double convertDouble(std::string_view input) {
    double ret    = 0.;
    auto   result = std::from_chars(input.begin(), input.end(), ret);
    throwIfInvalid(result.ec == std::errc{});
    throwIfInvalid(result.ptr != input.data());
    return ret;
}

template<std::integral T>
void Coordinate<T>::setMaxFromMap(MapView map) {
    throwIfInvalid(!map.Base.empty());
    throwIfInvalid(!map.Base.front().empty());
    MaxRow    = static_cast<T>(map.Base.size());
    MaxColumn = static_cast<T>(map.Base.front().size());
    return;
}

#endif //HELPER_HPP
