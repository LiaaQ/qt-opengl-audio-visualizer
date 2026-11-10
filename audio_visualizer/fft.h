#pragma once

#include <cassert>
#include <cmath>
#include <complex>
#include <utility>
#include <vector>

// Minimal in-place radix-2 FFT. The size must be a power of two.
// forward() is unscaled; inverse() is scaled by 1/N, so inverse(forward(x)) == x.
class Fft {
public:
    using Complex = std::complex<float>;

    explicit Fft(int size)
        : m_n(size), m_twiddle(size / 2), m_rev(size)
    {
        assert(size > 1 && (size & (size - 1)) == 0);

        int bits = 0;
        while ((1 << bits) < size)
            ++bits;
        for (int i = 0; i < size; ++i) {
            int r = 0;
            for (int b = 0; b < bits; ++b)
                if (i & (1 << b))
                    r |= 1 << (bits - 1 - b);
            m_rev[i] = r;
        }

        const double pi = 3.14159265358979323846;
        for (int k = 0; k < size / 2; ++k) {
            const double a = 2.0 * pi * k / size;
            m_twiddle[k] = Complex(float(std::cos(a)), float(-std::sin(a)));
        }
    }

    int size() const { return m_n; }

    void forward(Complex *x) const { transform(x, false); }
    void inverse(Complex *x) const { transform(x, true); }

private:
    void transform(Complex *x, bool inv) const
    {
        for (int i = 0; i < m_n; ++i) {
            const int j = m_rev[i];
            if (i < j)
                std::swap(x[i], x[j]);
        }

        for (int len = 2; len <= m_n; len <<= 1) {
            const int half = len / 2;
            const int step = m_n / len;
            for (int i = 0; i < m_n; i += len) {
                for (int j = 0; j < half; ++j) {
                    const Complex w = m_twiddle[j * step];
                    const float wr = w.real();
                    const float wi = inv ? -w.imag() : w.imag();

                    const Complex a = x[i + j];
                    const Complex b = x[i + j + half];
                    // Manual complex multiply: std::complex operator* is slow without -ffast-math.
                    const float vr = b.real() * wr - b.imag() * wi;
                    const float vi = b.real() * wi + b.imag() * wr;

                    x[i + j]        = Complex(a.real() + vr, a.imag() + vi);
                    x[i + j + half] = Complex(a.real() - vr, a.imag() - vi);
                }
            }
        }

        if (inv) {
            const float s = 1.0f / float(m_n);
            for (int i = 0; i < m_n; ++i)
                x[i] = Complex(x[i].real() * s, x[i].imag() * s);
        }
    }

    int m_n;
    std::vector<Complex> m_twiddle;
    std::vector<int> m_rev;
};