#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sm4.h>

void Padding(const uint8_t *in, uint8_t *out, long len)
{
    memcpy(out, in, len);
    int pad_len = 16 - (len % 16);
    for (int i = 0; i < pad_len; i++)
        out[len + i] = (uint8_t)pad_len;
}

void Unpadding(const uint8_t *in, uint8_t *out, long len)
{
    int pad_len = 16 - (len % 16);
    memcpy(out, in, len);
}

void EncCBC(const uint32_t RK[32], const uint8_t iv[16],
            const uint8_t *plaintext, uint8_t *ciphertext,
            long text_len)
{
    uint8_t tmp[16];    // 明文块临时变量
    uint8_t xortmp[16]; // 异或临时变量存储上一密文块

    // 初始化异或临时变量为初始向量
    memcpy(xortmp, iv, 16);

    int pad_len = 16 - (text_len % 16);    // 填充长度
    long tmptext_len = text_len + pad_len; // 填充后的明文长度

    // 明文临时变量
    uint8_t *tmptext = (uint8_t *)malloc(tmptext_len);

    // 填充明文
    Padding(plaintext, tmptext, text_len);

    // 每次加密16字节
    for (int i = 0; i < tmptext_len; i += 16)
    {
        // 异或
#if defined(SIMD)
        XORSIMD(tmp, tmptext + i, xortmp);
#else
        for (int j = 0; j < 16; j++)
            tmp[j] = tmptext[i + j] ^ xortmp[j];
#endif
        // 加密当前块
        SM4(RK, tmp, (ciphertext + i), 0);
        // 更新异或临时变量为当前密文块
        memcpy(xortmp, (ciphertext + i), 16);
    }

    free(tmptext);
}

// CBC模式解密
void DecCBC(const uint32_t RK[32], const uint8_t iv[16],
            const uint8_t *ciphertext, uint8_t *plaintext,
            long text_len)
{
    int pad_len = 16 - (text_len % 16);    // 填充长度
    long tmptext_len = text_len + pad_len; // 填充后的明文长度

    // 明文临时变量
    uint8_t *tmptext = (uint8_t *)malloc(tmptext_len);

#if defined(SIMD)

    uint8_t tmps[4][16];    // 四组明文块临时变量
    uint8_t xortmps[4][16]; // 四组异或临时变量

    // 初始化第一组异或临时变量为初始向量
    memcpy(xortmps[0], iv, 16);

    long i = 0;
    // 每次解密64字节
    for (; i + 64 <= tmptext_len; i += 64)
    {
        // 更新后三组异或临时变量为当前四组密文块中的前三组
        memcpy(xortmps[1], (ciphertext + i), 16);
        memcpy(xortmps[2], (ciphertext + i + 16), 16);
        memcpy(xortmps[3], (ciphertext + i + 32), 16);

        // 解密四组密文块
        SM4SIMD(RK,
                (ciphertext + i), (ciphertext + i + 16),
                (ciphertext + i + 32), (ciphertext + i + 48),
                tmps[0], tmps[1],
                tmps[2], tmps[3],
                1);

        // 异或
        XORSIMD((tmptext + i), tmps[0], xortmps[0]);
        XORSIMD((tmptext + i + 16), tmps[1], xortmps[1]);
        XORSIMD((tmptext + i + 32), tmps[2], xortmps[2]);
        XORSIMD((tmptext + i + 48), tmps[3], xortmps[3]);

        // 更新第一组异或临时变量为当前四组密文块中的最后一组
        memcpy(xortmps[0], (ciphertext + i + 48), 16);
    }
    // 最后不足64字节的部分每次解密16字节
    for (; i < tmptext_len; i += 16)
    {
        // 解密当前块
        SM4(RK, (ciphertext + i), tmps[0], 1);
        // 异或
        XORSIMD((tmptext + i), tmps[0], xortmps[0]);
        // 更新异或临时变量为当前密文块
        memcpy(xortmps[0], (ciphertext + i), 16);
    }

#else
    uint8_t tmp[16];    // 密文块临时变量
    uint8_t xortmp[16]; // 异或临时变量1存储上一密文块

    // 初始化异或临时变量为初始向量
    memcpy(xortmp, iv, 16);

    // 每次解密16字节
    for (int i = 0; i < tmptext_len; i += 16)
    {
        // 解密当前块
        SM4(RK, (ciphertext + i), tmp, 1);
        // 异或
        for (int j = 0; j < 16; j++)
            tmptext[i + j] = tmp[j] ^ xortmp[j];
        // 更新异或临时变量为当前密文块
        memcpy(xortmp, (ciphertext + i), 16);
    }
#endif

    // 去填充明文
    Unpadding(tmptext, plaintext, text_len);
}