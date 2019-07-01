#include "../indicators.h"

#include <memory>
#include <cstring>

/* Constexpr-sized, flat array */
template<int N>
struct ringbuf {
    TI_REAL buf[N+1] = {0};
    int pos = 1;
    operator TI_REAL() const { return buf[pos]; }
    TI_REAL& operator[](int i) { return buf[(pos+i)%N]; }
    TI_REAL operator[](int i) const { return buf[(pos+i)%N]; }
    void step() { pos = (N+pos-1) % N; }

    TI_REAL* phbegin() { return buf; }
    TI_REAL* phend() { return buf + N; }
    int iterator_to_age(TI_REAL* it) {
        return ((it - buf) + (N - pos)) % N;
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
    TI_REAL& operator[](int i) { return buf[(pos+i)%M]; }
    TI_REAL operator[](int i) const { return buf[(pos+i)%M]; }
    void step() { pos = (M+pos-1) % M; }

    TI_REAL* phbegin() { return buf.get(); }
    TI_REAL* phend() { return buf.get() + M; }
    int iterator_to_age(TI_REAL* it) {
        return ((it - buf.get()) + (M - pos)) % M;
    }
};
