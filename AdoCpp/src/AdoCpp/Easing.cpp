#include "Easing.h"

#include <cmath>

namespace AdoCpp
{
    const double pi = 3.1415926, c1 = 1.70158, c2 = c1 * 1.525, c3 = c1 + 1, c4 = (2 * pi) / 3, c5 = (2 * pi) / 4.5,
                 n1 = 7.5625, d1 = 2.75;

    static double out_bounce(double x)
    {
        if (x < 1 / d1)
        {
            return n1 * x * x;
        }
        if (x < 2 / d1)
        {
            return n1 * (x -= 1.5 / d1) * x + 0.75;
        }
        if (x < 2.5 / d1)
        {
            return n1 * (x -= 2.25 / d1) * x + 0.9375;
        }
        return n1 * (x -= 2.625 / d1) * x + 0.984375;
    }

    static double pow2(const double x) { return x * x; }
    static double pow3(const double x) { return x * x * x; }
    static double pow4(const double x) { return x * x * x * x; }
    static double pow5(const double x) { return x * x * x * x * x; }

    /**
     * @brief Ease.
     * y = f(x) while f is an easing function.
     * @param easing Easing function.
     * @param x x.
     * @return y.
     */
    double ease(const Easing easing, const double x)
    {
        if (x <= 0)
            return 0;
        if (x >= 1)
            return 1;

        using enum Easing;

        switch (easing)
        {
        case Linear:
            return x;
        case InSine:
            return 1 - cos(x * pi / 2);
        case OutSine:
            return sin(x * pi / 2);
        case InOutSine:
            return -(cos(pi * x) - 1) / 2;
        case InQuad:
            return pow2(x);
        case OutQuad:
            return 1 - pow2(1 - x);
        case InOutQuad:
            return x < 0.5 ? 2 * pow2(x) : 1 - pow2(-2 * x + 2) / 2;
        case InCubic:
            return pow3(x);
        case OutCubic:
            return 1 - pow3(1 - x);
        case InOutCubic:
            return x < 0.5 ? 4 * pow3(x) : 1 - pow3(-2 * x + 2) / 2;
        case InQuart:
            return pow4(x);
        case OutQuart:
            return 1 - pow4(1 - x);
        case InOutQuart:
            return x < 0.5 ? 8 * pow4(x) : 1 - pow4(-2 * x + 2) / 2;
        case InQuint:
            return pow5(x);
        case OutQuint:
            return 1 - pow5(1 - x);
        case InOutQuint:
            return x < 0.5 ? 16 * pow5(x) : 1 - pow5(-2 * x + 2) / 2;
        case InExpo:
            return pow(2, 10 * x - 10);
        case OutExpo:
            return 1 - pow(2, -10 * x);
        case InOutExpo:
            return x < 0.5 ? pow(2, 20 * x - 10) / 2 : (2 - pow(2, -20 * x + 10)) / 2;
        case InCirc:
            return 1 - sqrt(1 - x * x);
        case OutCirc:
            return sqrt(1 - (x - 1) * (x - 1));
        case InOutCirc:
            return x < 0.5 ? (1 - sqrt(1 - pow(2 * x, 2))) / 2 : (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
        case InBack:
            return c3 * pow3(x) - c1 * x * x;
        case OutBack:
            return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
        case InOutBack:
            return x < 0.5 ? (pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
                           : (pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
        case InElastic:
            return -pow(2, 10 * x - 10) * sin((x * 10 - 10.75) * c4);
        case OutElastic:
            return pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
        case InOutElastic:
            return x < 0.5 ? -(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * c5)) / 2
                           : (pow(2, -20 * x + 10) * sin((20 * x - 11.125) * c5)) / 2 + 1;
        case InBounce:
            return 1 - out_bounce(1 - x);
        case OutBounce:
            return out_bounce(x);
        case InOutBounce:
            return x < 0.5 ? (1 - out_bounce(1 - 2 * x)) / 2 : (1 + out_bounce(2 * x - 1)) / 2;
        default:
            break;
        }
        return 0;
    }
} // namespace AdoCpp
