#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

static uint32_t p1_step0(uint32_t lo, uint32_t hi) {
    return (hi ^ UINT32_C(0x70b3)) + UINT32_C(8) * lo;
}

static uint32_t p1_step1(uint32_t hi, uint32_t mid) {
    return UINT32_C(3) * (hi - mid) + UINT32_C(0xeb26);
}

static uint32_t p1_step2(uint32_t s0, uint32_t s1) {
    return (UINT32_C(5) * s0) ^ (s1 + UINT32_C(0xaad5));
}

static uint32_t p3_eval(uint32_t idx, uint32_t x) {
    switch (idx) {
        case 0: return UINT32_C(5) * x + UINT32_C(0x31d0);
        case 1: return UINT32_C(3) * x + UINT32_C(0xcf75);
        case 2: return (UINT32_C(2) * x) ^ UINT32_C(0x21c3);
        case 3: return UINT32_C(5) * x + UINT32_C(0x31989);
        case 4: return (UINT32_C(2) * x) ^ UINT32_C(0x3ec1);
        case 5: return UINT32_C(5) * x + UINT32_C(0x171f6);
        case 6: return (x ^ UINT32_C(0xe66f)) + (x >> 1);
        default: return (x ^ UINT32_C(0xb987)) + (x >> 2);
    }
}

static uint32_t p4_walk(uint32_t n, uint32_t lo, uint32_t hi, uint32_t k) {
    uint32_t mid = ((hi - lo) >> 1) + lo;
    if (mid == n) return (mid + UINT32_C(0xbe)) ^ k;
    uint32_t t = mid + k;
    if (n < mid) {
        uint32_t r = p4_walk(n, lo, mid - 1, k);
        return (UINT32_C(5) * r) ^ t ^ UINT32_C(0x19);
    } else {
        uint32_t r = p4_walk(n, mid + 1, hi, k);
        return (UINT32_C(5) * r) ^ t ^ UINT32_C(0x13);
    }
}

static const uint32_t p5_tag[12] = {
    2891, 2388, 1101, 365, 2114, 2627, 1640, 1831, 798, 607, 4, 1293
};

static const uint32_t p5_key[12] = {
    24943, 21533, 47156, 58293, 6787, 18387, 37711, 20578, 25038, 39064, 5180, 48843
};

static uint32_t p5_traverse(uint64_t challenge, uint32_t *out_tag) {
    uint32_t idx = (uint32_t)challenge & 7u;
    uint32_t acc = ((uint32_t)(challenge >> 6)) ^ ((uint32_t)(challenge >> 32));
    acc ^= p5_key[idx];
    acc ^= UINT32_C(0x9d82);
    *out_tag = p5_tag[idx];
    acc += p5_tag[idx];

    uint32_t steps = ((uint32_t)(challenge >> 3)) & 3u;
    for (uint32_t i = 1; i <= steps; i++) {
        idx++;
        acc ^= p5_key[idx];
        *out_tag = p5_tag[idx];
        acc += p5_tag[idx];
    }
    return acc;
}

static uint32_t p6_op(int k, uint32_t x) {
    switch (k) {
        case 0: return (x ^ UINT32_C(0xfe97)) + UINT32_C(4) * x;
        case 1: return ((x >> 5) ^ x) + UINT32_C(0x53a7);
        case 2: return (((x << 5) + UINT32_C(0xbb10)) ^ (UINT32_C(5) * x));
        case 3: return ((x << 3) | (x >> 29)) ^ UINT32_C(0x47d0);
        case 4: return (x >> 2) | (x << 30);
        default: return (x + UINT32_C(0x9b10)) ^ (x >> 3);
    }
}

static const int p6_tab[8] = {4, 4, 2, 0, 5, 1, 4, 3};

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[1];
    uint32_t hi  = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t lo  = (uint32_t)challenge;

    uint32_t s1 = p1_step1(hi, mid);
    uint32_t s0 = p1_step0(lo, hi);
    values[0] = p1_step2(s0, s1);
    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[6];
    uint64_t c = challenge;

    values[0] = (uint32_t)(((c >> 18) & UINT64_C(0x1f)) + UINT64_C(8));
    for (int i = 1; i <= 5; i++) {
        uint32_t u = (uint32_t)((c >> (15 + 2 * (i - 1))) & UINT64_C(0xf));
        values[i] = ((values[i - 1] * UINT32_C(2)) ^ u) + (uint32_t)(8 * i);
    }

    return write_answer(out, cap, values, 6);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[2];
    uint32_t idx = (uint32_t)challenge & 7u;
    uint32_t x = ((uint32_t)(challenge >> 3)) ^ ((uint32_t)(challenge >> 32));

    values[0] = idx;
    values[1] = p3_eval(idx, x);
    return write_answer(out, cap, values, 2);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[1];
    uint32_t n = (uint32_t)challenge & 0x7fu;
    uint32_t k = ((uint32_t)(challenge >> 7)) ^ ((uint32_t)(challenge >> 32));

    values[0] = p4_walk(n, 0, 0x7f, k);
    return write_answer(out, cap, values, 1);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[2];
    uint32_t tag = 0;

    values[0] = p5_traverse(challenge, &tag);
    values[1] = tag;
    return write_answer(out, cap, values, 2);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[1];
    uint32_t x = ((uint32_t)(challenge >> 6)) ^ ((uint32_t)(challenge >> 32));
    uint32_t y = p6_op(p6_tab[(uint32_t)challenge & 7u], x);
    y ^= UINT32_C(0x9739);
    uint32_t z = p6_op(p6_tab[((uint32_t)challenge >> 3) & 7u], y);
    values[0] = z + UINT32_C(0xe254);
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t a = (uint32_t)context & UINT32_C(3);
    uint32_t s = ((uint32_t)(challenge >> 32)) ^ ((uint32_t)challenge);
    uint32_t t = ((uint32_t)(context >> 32)) ^ ((uint32_t)context);
    uint32_t m = (UINT32_C(0x73ea) * a) ^ ((uint32_t)(context >> 16));
    uint32_t x = s ^ t;
    uint32_t mix = ((x << 5) | (x >> 27)) + UINT32_C(0x73ea);
    mix ^= UINT32_C(0x5b4c);
    uint32_t b = mix ^ m;
    uint32_t c = b ^ (UINT32_C(0x5b4c) + t + a + (s >> 5));

    int n = snprintf(out, cap,
                     "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%u %u %u",
                     a, b, c);
    return (n < 0 || (size_t)n >= cap) ? 1 : 0;
}
