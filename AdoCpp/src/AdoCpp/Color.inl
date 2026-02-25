#pragma once
#include "Color.h"
#include <cassert>
#include <algorithm>

namespace AdoCpp
{
    namespace priv
    {
        constexpr double positiveRemainder_(const double a, const double b)
        {
            assert(b > 0.0 && "Cannot calculate remainder with non-positive divisor");
            const double val = a - static_cast<double>(static_cast<int>(a / b)) * b;
            return val >= 0.0 ? val : val + b;
        }
    } // namespace priv

    // Most of the code is copied from "SFML/Graphics/Color.inl".
    constexpr Color::Color(const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue,
                           const std::uint8_t alpha) : r(red), g(green), b(blue), a(alpha)
    {
    }

    constexpr Color::Color(const std::uint32_t color) :
        r(static_cast<std::uint8_t>((color & 0xff000000) >> 24)),
        g(static_cast<std::uint8_t>((color & 0x00ff0000) >> 16)),
        b(static_cast<std::uint8_t>((color & 0x0000ff00) >> 8)), a(static_cast<std::uint8_t>(color & 0x000000ff))
    {
    }

    constexpr Color::Color(const std::string& color)
    {
        const auto charToDigit16 = [](const char c)
        {
            return static_cast<std::uint8_t>('0' <= c && c <= '9'       ? c - '0'
                                                 : 'A' <= c && c <= 'F' ? c - 'A' + 10
                                                                        : c - 'a' + 10);
        };
        const size_t hash = color[0] == '#';
        r = charToDigit16(color[hash + 0]) * 16 + charToDigit16(color[hash + 1]);
        g = charToDigit16(color[hash + 2]) * 16 + charToDigit16(color[hash + 3]);
        b = charToDigit16(color[hash + 4]) * 16 + charToDigit16(color[hash + 5]);
        if (color.size() == hash + 8)
            a = charToDigit16(color[hash + 6]) * 16 + charToDigit16(color[hash + 7]);
    }
    constexpr Color Color::fromHSV(const double h, const double s, const double v)
    {
        auto _abs = [](const double d) { return d > 0.0 ? d : -d; };
        const double c = v * s, x = c * (1 - _abs(priv::positiveRemainder_(h / 60, 2) - 1)), m = v - c;
        auto [r1, g1, b1] = [&]() -> std::tuple<double, double, double>
        {
            if (h < 60)
                return {c, x, 0};
            if (h < 120)
                return {x, c, 0};
            if (h < 180)
                return {0, c, x};
            if (h < 240)
                return {0, x, c};
            if (h < 300)
                return {x, 0, c};
            return {c, 0, x};
        }();
        return {static_cast<uint8_t>((r1 + m) * 255), static_cast<uint8_t>((g1 + m) * 255),
                static_cast<uint8_t>((b1 + m) * 255)};
    }
    constexpr std::tuple<double, double, double> Color::toHSV() const
    {
        const double r1 = r / 255.0, g1 = g / 255.0, b1 = b / 255.0;
        const double cMax = std::max({r1, g1, b1}), cMin = std::min({r1, g1, b1}), delta = cMax - cMin;
        auto H = [&]() -> double
        {
            if (delta == 0)
                return 0;
            if (cMax == r1)
                return 60 * priv::positiveRemainder_((g1 - b1) / delta, 6);
            if (cMax == g1)
                return 60 * ((b1 - r1) / delta + 2);
            // if (cMax == b1)
            return 60 * ((r1 - g1) / delta + 4);
        };
        auto S = [&]() -> double
        {
            if (cMax == 0)
                return 0;
            return delta / cMax;
        };
        auto V = [&]() -> double { return cMax; };
        return {H(), S(), V()};
    }

    constexpr std::uint32_t Color::toInteger() const
    {
        return static_cast<std::uint32_t>((r << 24) | (g << 16) | (b << 8) | a);
    }

    constexpr std::string Color::toString(const bool hash, const bool uppercase,
                                          const ToStringAlphaMode alphaMode) const
    {
        std::string str;
        if (hash)
            str.push_back('#');
        const auto digit16ToChar = [uppercase](const std::uint8_t v)
        {
            return static_cast<char>(v < 10 ? v + '0' : uppercase ? v - 10 + 'A' : v - 10 + 'a');
        };
        str.push_back(digit16ToChar(r / 16)), str.push_back(digit16ToChar(r % 16));
        str.push_back(digit16ToChar(g / 16)), str.push_back(digit16ToChar(g % 16));
        str.push_back(digit16ToChar(b / 16)), str.push_back(digit16ToChar(b % 16));

        using enum ToStringAlphaMode;
        if (alphaMode != Ignore && (alphaMode == Show || (alphaMode == Auto && a != 255)))
            str.push_back(digit16ToChar(a / 16)), str.push_back(digit16ToChar(a % 16));
        return str;
    }

    constexpr bool operator==(const Color left, const Color right)
    {
        return (left.r == right.r) && (left.g == right.g) && (left.b == right.b) && (left.a == right.a);
    }

    constexpr bool operator!=(const Color left, const Color right) { return !(left == right); }

    constexpr Color operator+(const Color left, const Color right)
    {
        const auto clampedAdd = [](std::uint8_t lhs, std::uint8_t rhs)
        {
            const int intResult = int{lhs} + int{rhs};
            return static_cast<std::uint8_t>(intResult < 255 ? intResult : 255);
        };

        return {clampedAdd(left.r, right.r), clampedAdd(left.g, right.g), clampedAdd(left.b, right.b),
                clampedAdd(left.a, right.a)};
    }

    constexpr Color operator-(const Color left, const Color right)
    {
        const auto clampedSub = [](std::uint8_t lhs, std::uint8_t rhs)
        {
            const int intResult = int{lhs} - int{rhs};
            return static_cast<std::uint8_t>(intResult > 0 ? intResult : 0);
        };

        return {clampedSub(left.r, right.r), clampedSub(left.g, right.g), clampedSub(left.b, right.b),
                clampedSub(left.a, right.a)};
    }

    constexpr Color operator*(const Color left, const Color right)
    {
        const auto scaledMul = [](std::uint8_t lhs, std::uint8_t rhs)
        {
            const auto uint16Result = static_cast<std::uint16_t>(std::uint16_t{lhs} * std::uint16_t{rhs});
            return static_cast<std::uint8_t>(uint16Result / 255u);
        };

        return {scaledMul(left.r, right.r), scaledMul(left.g, right.g), scaledMul(left.b, right.b),
                scaledMul(left.a, right.a)};
    }

    constexpr Color& operator+=(Color& left, const Color right) { return left = left + right; }

    constexpr Color& operator-=(Color& left, const Color right) { return left = left - right; }

    constexpr Color& operator*=(Color& left, const Color right) { return left = left * right; }
} // namespace AdoCpp
