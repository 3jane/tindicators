#pragma once

#include "../indicators.h"

#include <memory>
#include <cstring>
#include <algorithm>
#include <iterator>

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

    template<class Cmp = std::less<TI_REAL>>
    TI_REAL* find_max(int period) {
        assert(period <= N);
        assert(period > 0);

        Cmp cmp;
        using reverse = std::reverse_iterator<TI_REAL*>;

        if (pos + period > N) {
            const int size_part1 = N - pos;
            const int size_part2 = period - size_part1;


            auto it1 = std::max_element(reverse(buf + pos + size_part1), reverse(buf + pos), cmp);
            auto it2 = std::max_element(reverse(buf + size_part2), reverse(buf), cmp);

            if (!cmp(*it1, *it2)) { return it1.base()-1; }
            else { return it2.base()-1; }
        } else {
            auto it = std::max_element(reverse(buf + pos + period), reverse(buf + pos), cmp);
            return it.base()-1;
        }
    }
    TI_REAL* find_min(int period) {
        return find_max<std::greater<TI_REAL>>(period);
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
        assert(i < 2);
        assert(i > -1);
        return i == 0 ? a1 : a2;
    }
    TI_REAL operator[](int i) const {
        assert(i < 2);
        assert(i > -1);
        return i == 0 ? a1 : a2;
    }
    void step() { std::swap(a1, a2); }
    void operator=(TI_REAL x) { a1 = x; }

    template<class Cmp = std::less<TI_REAL>>
    TI_REAL* find_max(int period) {
        if (a1 >= a2) { return &a1; }
        else { return &a2; }
    }
    TI_REAL* find_min(int period) {
        if (a1 <= a2) { return &a1; }
        else { return &a2; }
    }

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
    explicit ringbuf() = default;
    explicit ringbuf(int M_) { resize(M_); }
    void resize(int M_) {
        assert(M_ > 0);
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

    template<class Cmp = std::less<TI_REAL>>
    TI_REAL* find_max(int period) {
        assert(period <= M);
        assert(period > 0);

        Cmp cmp;
        using reverse = std::reverse_iterator<TI_REAL*>;

        if (pos + period > M) {
            const int size_part1 = M - pos;
            const int size_part2 = period - size_part1;


            auto it1 = std::max_element(reverse(buf.get() + pos + size_part1), reverse(buf.get() + pos), cmp);
            auto it2 = std::max_element(reverse(buf.get() + size_part2), reverse(buf.get()), cmp);

            if (!cmp(*it1, *it2)) { return it1.base()-1; }
            else { return it2.base()-1; }
        } else {
            auto it = std::max_element(reverse(buf.get() + pos + period), reverse(buf.get() + pos), cmp);
            return it.base()-1;
        }
    }
    TI_REAL* find_min(int period) {
        return find_max<std::greater<TI_REAL>>(period);
    }

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
