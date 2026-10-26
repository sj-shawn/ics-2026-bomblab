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

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    (void)challenge;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t x_b32 = challenge & 0xffffffff;
    uint32_t x_f32 = (challenge >> 32) & 0xffffffff;
    uint32_t x_r16 = challenge >> 16;
    uint32_t S2 = (x_f32 * 5 + x_b32) ^ 0xcac5;
    uint32_t S0 = (x_r16 + 0x7032) ^ x_b32;
    uint32_t S1 = (S2 ^ 0x5d86) + 4*S0;
    values[0] = S1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    (void)challenge;
    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = 3 * ((challenge >> 47) & 0x1f);
    uint32_t i = 1;
    uint32_t bias = 6;
    while(i < 6){
        values[i] = (((challenge >> bias) & 0x3f) + i) ^ (values[i-1] * 5);
        i += 1;
        bias += 4;         
    }
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    (void)challenge;
    // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t a = challenge & 0x07;
    uint32_t b =(challenge >> 0x20) ^ (challenge >> 3);
    values[0] = a;
    switch(a){
        case 0: {values[1] = b * 3 + 0xc0fe; break;}
        case 1: {values[1] = (b * 2) ^ 0x9ed3; break;}
        case 2: {values[1] = b * 3 + 0x2b29; break;}
        case 3: {values[1] = (b >> 1) + (b ^ 0x62e8); break;}
        case 4: {values[1] = (b * 4) ^ 0x49e5; break;}
        case 5: {values[1] = b * 5 + 0x3a813; break;}
        case 6: {values[1] = b * 5 + 0x3d202; break;}
        case 7: {values[1] = (b >> 1) + (b ^ 0xef6d); break;}
    }
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    uint32_t mid = b + ((c - b) >> 1);

    if (a == mid)
        return (mid + 0x97u) ^ d;

    if (a < mid)
        return (5u * p4_walk(a, b, mid - 1u, d)) ^ (mid + d) ^ 0x09u;

    return (5u * p4_walk(a, mid + 1u, c, d)) ^ (mid + d) ^ 0x1fu;
}


int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    (void)challenge;
    // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t a = challenge & 0x7f;
    uint32_t b = 0;
    uint32_t c = 0x7f;
    uint32_t d = (challenge >> 0x20) ^ (challenge >> 0x7);
    values[0] = p4_walk(a, b, c, d);
    return write_answer(out, cap, values, count);
}


struct NodeData {
    int next;
    uint32_t x1;
    uint32_t x2;
};

static const struct NodeData nodes[] = {
    { -1, 0x0214, 0x7ef9 }, 
    {  8, 0x0470, 0xe9be }, 
    {  5, 0x0621, 0x2c2c }, 
    {  6, 0x0a4b, 0xd130 }, 
    {  9, 0x0349, 0x7f03 }, 
    {  1, 0x0021, 0x1cfe }, 
    {  2, 0x083f, 0x894f }, 
    { 11, 0x0987, 0x0fbf }, 
    {  7, 0x051a, 0x1aca }, 
    { 10, 0x0111, 0x1a61 }, 
    {  3, 0x0b3d, 0xae37 }, 
    {  0, 0x070b, 0x56e0 }, 
};

static const unsigned heads[] = {
    4, 9, 10, 3, 6, 2, 5, 1
};

static uint32_t p5_traverse(uint64_t challenge, uint32_t *extra)
{
    unsigned index = heads[challenge & 7];
    uint32_t result = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32) ^ 0xd492;
    result = 5 * (result + nodes[index].x2) ^ nodes[index].x1;
    *extra = nodes[index].x1;

    unsigned steps = (unsigned)((challenge >> 3) & 3);
    for (unsigned i = 0; i < steps; ++i) {
        index = (unsigned)nodes[index].next;
        result = 5 * (result + nodes[index].x2) ^ nodes[index].x1;
        *extra = nodes[index].x1;
    }

    return result;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    (void)challenge;
    // TODO: 根据 phase_05 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t extra;
    values[0] = p5_traverse(challenge, &extra);
    values[1] = extra;
    return write_answer(out, cap, values, count);
}

uint32_t p6_op_0(uint32_t x){
    return (x ^ (x >> 1)) + 0x203;
}
uint32_t p6_op_1(uint32_t x){
    return (x + 0x6fae) ^ (x >> 1);
}
uint32_t p6_op_2(uint32_t x){
    return (x << 30) | ((x >> 2) & 0x3fffffff);
}
uint32_t p6_op_3(uint32_t x){
    return (x ^ 0xe816) + 2 * x;
}
uint32_t p6_op_4(uint32_t x){
    return (7 * x) ^ ((x << 0x5) + 0x4eee);
}
uint32_t p6_op_5(uint32_t x){
    uint32_t rl = (x << 2) | (((x & 0xc0000000) >> 30) & 0x3);
    return rl ^ 0xc9d1;
}

typedef uint32_t (*op_func)(uint32_t);
op_func op[8] = {p6_op_1, p6_op_1, p6_op_4, p6_op_3, p6_op_5, p6_op_1, p6_op_2, p6_op_0};

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    (void)challenge;
    // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t x = (challenge >> 0x20) ^ (challenge >> 0x6);
    uint32_t idx1 = challenge & 0x7;
    uint32_t idx2 = (challenge & 0x38) / 8;
    x = op[idx1](x);
    x = x ^ 0x3c45;
    x = op[idx2](x);
    x += 0x5125;
    values[0] = x;
    return write_answer(out, cap, values, count);
}

uint32_t secret_mix(uint32_t k){
    return (k ^ 0x7479) + ((k >> 0x3) ^ 0x916f);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    uint32_t x = challenge ^ (challenge >> 0x20);
    uint32_t y = (context ^ (context >> 0x20));
    uint32_t z = ((context & 0x3) * 0x7479) ^ (context >> 0x10);
    uint32_t out_1 = context & 0x3;
    uint32_t x_1 = secret_mix(x ^ y);
    uint32_t out_2 = x_1 ^ z;
    uint32_t out_3 = out_2 ^ (0x916fu + y + out_1 + (x >> 3));
    
    int n = snprintf(
        out,
        cap,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%u %u %u",
        out_1,
        out_2,
        out_3
    );

    if (n < 0 || (size_t)n >= cap)
        return 1;

    return 0;
}
