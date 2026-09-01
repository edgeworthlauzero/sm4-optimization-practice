#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sm4.h>

static inline void CounterInc(uint8_t ctr[16], unsigned int delta)
{
    unsigned int carry = delta;
    for (int i = 15; i >= 0 && carry; i--)
    {
        unsigned int sum = ctr[i] + carry;
        ctr[i] = (uint8_t)sum;
        carry = sum >> 8;
    }
}

void EncCTR(const uint32_t RK[32], const uint8_t iv[16],
            const uint8_t *plaintext, uint8_t *ciphertext,
            long text_len)
{

#if defined(SIMD)

    uint8_t counter[16];
    memcpy(counter, iv, 16);

    uint8_t counters[4][16];   // 四组计数器临时变量
    uint8_t keystreams[4][16]; // 四组密钥流临时变量

    long i = 0;
    // 每次加密64字节
    for (; i + 64 <= text_len; i += 64)
    {
        // 初始化四组计数器
        memcpy(counters[0], counter, 16);
        for (int b = 1; b < 4; b++)
        {
            memcpy(counters[b], counters[b - 1], 16);
            CounterInc(counters[b], 1);
        }

        // 计算四组密钥流
        SM4SIMD(RK,
                counters[0], counters[1],
                counters[2], counters[3],
                keystreams[0], keystreams[1],
                keystreams[2], keystreams[3],
                0);

        // 异或加密
        XORSIMD((ciphertext + i), (plaintext + i), keystreams[0]);
        XORSIMD((ciphertext + i + 16), (plaintext + i + 16), keystreams[1]);
        XORSIMD((ciphertext + i + 32), (plaintext + i + 32), keystreams[2]);
        XORSIMD((ciphertext + i + 48), (plaintext + i + 48), keystreams[3]);

        // 计数器递增
        CounterInc(counter, 4);
    }
    // 最后不足64字节的部分每次加密16字节
    for (; i < text_len; i += 16)
    {
        // 计算密钥流
        SM4(RK, counter, keystreams[0], 0);
        // 异或加密
        for (int j = 0; j < 16 && i + j < text_len; j++)
            ciphertext[i + j] = plaintext[i + j] ^ keystreams[0][j];
        // 计数器递增
        CounterInc(counter, 1);
    }

#else
    uint8_t counter[16];
    uint8_t keystream[16];

    // 初始化计数器
    memcpy(counter, iv, 16);

    // 每次加密16字节
    for (long i = 0; i < text_len; i += 16)
    {
        // 计算密钥流
        SM4(RK, counter, keystream, 0);
        // 异或加密
        for (int j = 0; j < 16 && i + j < text_len; j++)
            ciphertext[i + j] = plaintext[i + j] ^ keystream[j];
        // 计数器递增
        CounterInc(counter, 1);
    }
#endif
}

#define DecCTR EncCTR