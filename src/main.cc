// Copyright 3bian Limited and CHERIoT Contributors.
// SPDX-License-Identifier: Apache-2.0

#include <compartment.h>
#include <cstdint>
#include <debug.hh>
#include <fail-simulator-on-error.h>
#include <cstdio>
//#include <cmath>
#include <limits>
#include <array>

/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "f64 Test:">;

const std::array<double, 24> testValues = {
    0.0, -0.0, 1.0, -1.0,
    0.1, 0.2, 0.3,
    1e-12, -1e-12, 1e-6, -1e-6,
    1e6, -1e6, 1e12, -1e12,
    1e300, -1e300,
    9007199254740992.0,   // 2^53 exact
    9007199254740993.0,   // 2^53 + 1 (rounds to 2^53)
    3.141592653589793, -3.141592653589793,
    2.718281828459045, -2.718281828459045,
    //(std::numeric_limits<double>::has_denorm ? std::nextafter(0.0, 1.0) : 0.0),
    std::numeric_limits<double>::min()
};

struct BinaryOp
{
    const char* label;
    double (*function)(double, double);
};

const std::array<BinaryOp, 4> binaryOps = {
    BinaryOp{"+", [](double left, double right) { return left + right; }},
    BinaryOp{"-", [](double left, double right) { return left - right; }},
    BinaryOp{"*", [](double left, double right) { return left * right; }},
    BinaryOp{"/", [](double left, double right) { return left / right; }}
};

void test_info_and_special_values()
{
    Debug::log("Double Info:");

    Debug::log("epsilon = {}", std::numeric_limits<double>::epsilon());
    Debug::log("min (normal) = {}", std::numeric_limits<double>::min());
    Debug::log("max = {}", std::numeric_limits<double>::max());

    if (std::numeric_limits<double>::has_denorm)
    {
        Debug::log("denorm_min = {}", std::numeric_limits<double>::denorm_min());
    }

    //Debug::log("nextafter(1,+inf) = %.17g", std::nextafter(1.0, 2.0));

    Debug::log("Special Values (Infinity, NaN, Signed Zero):");

    const double positiveInfinity =  1.0 /  0.0;
    const double negativeInfinity = -1.0 /  0.0;

    Debug::log("+inf = {}", positiveInfinity);
    Debug::log("-inf = {}", negativeInfinity);
    Debug::log(" NaN = {}", std::numeric_limits<double>::quiet_NaN());

    Debug::log("isinf(+inf)=%", __builtin_isnan(positiveInfinity));
    Debug::log("isinf(-inf)=%", __builtin_isinf(negativeInfinity));
    Debug::log("isnan(NaN)=%",  __builtin_isnan(std::numeric_limits<double>::quiet_NaN()));

    Debug::log("signbit(+0)=%", __builtin_signbit(0.0));
    Debug::log("signbit(-0)=%", __builtin_signbit(-0.0));

    Debug::log("1/+0 = {}", 1.0 /  0.0);
    Debug::log("1/-0 = {}", 1.0 / -0.0);

    Debug::log("");
}

void test_binary_ops(const std::array<double, testValues.size()>& values)
{
    Debug::log("Arithmetic Grid (+ - * /):");

    for (size_t i = 0; i < values.size(); ++i)
    {
        for (size_t j = 0; j < values.size(); ++j)
        {
            const double leftOperand = values[i];
            const double rightOperand = values[j];

            for (const auto& op : binaryOps)
            {
                const double result = op.function(leftOperand, rightOperand);
                Debug::log(" {} {} {} = {}", leftOperand, op.label, rightOperand, result);

                if (__builtin_isnan(result))
                {
                    Debug::log(" [NaN]");
                }

                if (__builtin_isinf(result))
                {
                    Debug::log(" [{}]", __builtin_signbit(result) ? '-' : '+');
                }
            }
        }
    }
}

void test_classic_pitfalls()
{
    Debug::log("Classic Pitfalls:");

    double sumValue = 0.1 + 0.2;
    Debug::log("0.1+0.2 = {}", sumValue);
    Debug::log("(0.1+0.2)==0.3 ? {}", sumValue == 0.3);

    double assocLeft = (1e20 + -1e20) + 3.14;
    Debug::log("(1e20+-1e20)+3.14 = {}", assocLeft);

    double assocRight = 1e20 + (-1e20 + 3.14);
    Debug::log("1e20+(-1e20+3.14) = {}", assocRight);

    double cancellation = (1e8 + 1.0) - 1e8;
    Debug::log("(1e8+1)-1e8 = {}", cancellation);

    double repeatedSum = 0.0;
    for (int i = 0; i < 10; ++i)
    {
        repeatedSum += 0.1;
    }
    Debug::log("0.1*10 = {}", repeatedSum);

    if (std::numeric_limits<double>::has_denorm)
    {
        Debug::log("min/2 = {}", std::numeric_limits<double>::min() / 2);
        Debug::log("min/4 = {}", std::numeric_limits<double>::min() / 4);
    }
}

/// Thread entry point.
void __cheri_compartment("entry_point") init()
{
    test_info_and_special_values();
    test_binary_ops(testValues);
    test_classic_pitfalls();
}