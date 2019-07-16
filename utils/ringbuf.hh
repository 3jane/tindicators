#pragma once

#include "../indicators.h"

#include <memory>
#include <cstring>

/*
    Ring buffer: https://en.wikipedia.org/wiki/Circular_buffer

    operator[](): indexing in the reverse order, that is, buffer[1] is the equivalent to "give me the previous value"
    step(): make the current value the previous, i.e. change the pointer to the current last value
    phbegin(), phend(): used to iterate over the underlying array (get the physical begin and end)
    iterator_to_age(): given a pointer to a buffer element, returns the distance from the current value
*/

/* Constexpr-sized, flat array */
template<int N>
struct ringbuf {
    TI_REAL buf[N] = {0};
    int pos = 0;
    operator TI_REAL() const { return buf[pos]; }
    TI_REAL& operator[](int i) {
        assert(i < N);
        assert(i > -1);
        int pos_ = pos + i;
        if (pos_ >= N) { pos_ -= N; }
        return buf[pos_];
    }
    TI_REAL operator[](int i) const {
        assert(i < N);
        assert(i > -1);
        int pos_ = pos + i;
        if (pos_ >= N) { pos_ -= N; }
        return buf[pos_];
    }
    void step() {
        pos -= 1;
        if (pos == -1) { pos += N; }
    }
    void operator=(TI_REAL x) {
        buf[pos] = x;
    }

    TI_REAL* phbegin() { return buf; }
    TI_REAL* phend() { return buf + N; }
    int iterator_to_age(TI_REAL* it) {
        assert(buf <= it && it < buf + N);
        return ((it - buf) + (N - pos)) % N;
    }
};

template<>
struct ringbuf<2> {
    TI_REAL a1, a2;
    operator TI_REAL() const { return a1; }
    TI_REAL& operator[](int i) {
        assert(i < N);
        assert(i > -1);
        return i == 0 ? a1 : a2;
    }
    TI_REAL operator[](int i) const {
        assert(i < N);
        assert(i > -1);
        return i == 0 ? a1 : a2;
    }
    void step() { std::swap(a1, a2); }
    void operator=(TI_REAL x) { a1 = x; }

    TI_REAL* phbegin() { return &a1; }
    TI_REAL* phend() { return &a1 + 2; }
    int iterator_to_age(TI_REAL* it) {
        return it == &a2;
    }
};

/* Runtime-sized, alloc'd at the heap */
template<>
struct ringbuf<0> {
    int M;
    int pos = 0;
    std::unique_ptr<TI_REAL[]> buf;
    explicit ringbuf(int M_ = 0) { resize(M_); }
    void resize(int M_) {
        M = M_;
        pos = 0;
        buf = std::make_unique<TI_REAL[]>(M);
        std::memset(buf.get(), 0, M*sizeof(TI_REAL));
    }
    operator TI_REAL() const { return buf[pos]; }
    TI_REAL operator[](int i) const {
        assert(i < M);
        assert(i > -1);
        int pos_ = pos + i;
        if (pos_ >= M) { pos_ -= M; }
        return buf[pos_];
    }
    void step() {
        pos -= 1;
        if (pos == -1) { pos += M; }
    }
    void operator=(TI_REAL x) {
        buf[pos] = x;
    }

    TI_REAL* phbegin() { return buf.get(); }
    TI_REAL* phend() { return buf.get() + M; }
    int iterator_to_age(TI_REAL* it) {
        assert(buf.get() <= it && it < buf.get() + M);
        return ((it - buf.get()) + (M - pos)) % M;
    }
};

void step() {}

template<class T, class... Ts>
void step(T& arg, Ts&... args) {
    arg.step();
    step(args...);
}
