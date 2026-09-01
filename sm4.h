#pragma once
#include <stdint.h>
#include <string.h>

#if defined(SIMD)
#include <emmintrin.h>
#include <tmmintrin.h>
#endif

#if defined(TABLE)
static uint32_t L0[256], L1[256], L2[256], L3[256];
static uint32_t L0Prime[256], L1Prime[256], L2Prime[256], L3Prime[256];
#elif defined(SIMD)
static __m128i SSIMD[16];
static __m128i ByteReMask;
#endif

static const uint8_t S[256] = {
    0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7,
    0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
    0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3,
    0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a,
    0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
    0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95,
    0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
    0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba,
    0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
    0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b,
    0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
    0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2,
    0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
    0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52,
    0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
    0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5,
    0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
    0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55,
    0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
    0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60,
    0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
    0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f,
    0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
    0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f,
    0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
    0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd,
    0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
    0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e,
    0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
    0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20,
    0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48};

static const uint32_t FK[4] = {
    0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc};

static const uint32_t CK[32] = {
    0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
    0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
    0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
    0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
    0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
    0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
    0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
    0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279};

static inline uint32_t Substitution(uint32_t A)
{
    return (S[(A >> 24) & 0xFF] << 24) |
           (S[(A >> 16) & 0xFF] << 16) |
           (S[(A >> 8) & 0xFF] << 8) |
           S[A & 0xFF];
}

#if defined(SIMD)
static inline __m128i SubstitutionSMID(__m128i A)
{
    __m128i out = _mm_setzero_si128();    // 初始化输出
    __m128i l_mask = _mm_set1_epi8(0x0F); // 低4位掩码
    __m128i h_mask = _mm_set1_epi8(0xF0); // 高4位掩码
    __m128i l = _mm_and_si128(A, l_mask); // 提取低4位

    for (int h = 0; h < 16; h++)
    {
        __m128i mask = _mm_cmpeq_epi8(
            _mm_and_si128(A, h_mask),     // 提取目标高4位
            _mm_set1_epi8((char)(h << 4)) // 提取当前高4位
        );
        __m128i value = _mm_shuffle_epi8(SSIMD[h], l); // 过S盒
        out = _mm_or_si128(
            _mm_and_si128(mask, value), // 命中字节取S盒值
            _mm_andnot_si128(mask, out) // 未命中字节保持原值
        );
    }
    return out;
}
#endif

static inline uint32_t LoopLeftShift(uint32_t X, int n)
{
    return (X << n) | (X >> (32 - n));
}

#if defined(SIMD)
static inline __m128i LoopLeftShiftSIMD(__m128i X, int n)
{
    return _mm_or_si128(_mm_slli_epi32(X, n),
                        _mm_srli_epi32(X, 32 - n));
}
#endif

static inline uint32_t LinearTransform(uint32_t B)
{
    return B ^ LoopLeftShift(B, 2) ^ LoopLeftShift(B, 10) ^
           LoopLeftShift(B, 18) ^ LoopLeftShift(B, 24);
}

#if defined(TABLE)
static inline uint32_t LinearTransformTable(uint32_t A)
{
    return (L0[(A >> 24) & 0xFF]) ^
           (L1[(A >> 16) & 0xFF]) ^
           (L2[(A >> 8) & 0xFF]) ^
           L3[A & 0xFF];
}
#elif defined(SIMD)
static inline __m128i LinearTransformSIMD(__m128i B)
{
    __m128i tmp;
    tmp = _mm_xor_si128(B, LoopLeftShiftSIMD(B, 2));
    tmp = _mm_xor_si128(tmp, LoopLeftShiftSIMD(B, 10));
    tmp = _mm_xor_si128(tmp, LoopLeftShiftSIMD(B, 18));
    tmp = _mm_xor_si128(tmp, LoopLeftShiftSIMD(B, 24));
    return tmp;
}
#endif

static inline uint32_t LinearTransformPrime(uint32_t B)
{
    return B ^ LoopLeftShift(B, 13) ^ LoopLeftShift(B, 23);
}

#if defined(TABLE)
static inline uint32_t LinearTransformPrimeTable(uint32_t A)
{
    return (L0Prime[(A >> 24) & 0xFF]) ^
           (L1Prime[(A >> 16) & 0xFF]) ^
           (L2Prime[(A >> 8) & 0xFF]) ^
           L3Prime[A & 0xFF];
}
#elif defined(SIMD)
static inline __m128i LinearTransformPrimeSIMD(__m128i B)
{
    __m128i tmp;
    tmp = _mm_xor_si128(B, LoopLeftShiftSIMD(B, 13));
    tmp = _mm_xor_si128(tmp, LoopLeftShiftSIMD(B, 23));
    return tmp;
}
#endif

static inline uint32_t T(uint32_t X0, uint32_t X1, uint32_t X2,
                         uint32_t X3, uint32_t K)
{
#if defined(TABLE)
    return X0 ^ LinearTransformTable(X1 ^ X2 ^ X3 ^ K);
#else
    return X0 ^ LinearTransform(Substitution(X1 ^ X2 ^ X3 ^ K));
#endif
}

#if defined(SIMD)
static inline __m128i TSIMD(__m128i X0, __m128i X1, __m128i X2,
                            __m128i X3, __m128i K)
{
    __m128i tmp;
    tmp = _mm_xor_si128(X1, X2);
    tmp = _mm_xor_si128(tmp, X3);
    tmp = _mm_xor_si128(tmp, K);
    tmp = SubstitutionSMID(tmp);
    tmp = LinearTransformSIMD(tmp);
    tmp = _mm_xor_si128(X0, tmp);
    return tmp;
}
#endif

static inline uint32_t TPrime(uint32_t X0, uint32_t X1, uint32_t X2,
                              uint32_t X3, uint32_t K)
{
#if defined(TABLE)
    return X0 ^ LinearTransformPrimeTable(X1 ^ X2 ^ X3 ^ K);
#else
    return X0 ^ LinearTransformPrime(Substitution(X1 ^ X2 ^ X3 ^ K));
#endif
}

#if defined(SIMD)
static inline __m128i ByteRe(__m128i X)
{
    return _mm_shuffle_epi8(X, ByteReMask);
}

static inline void Transpose(__m128i *W0, __m128i *W1,
                             __m128i *W2, __m128i *W3)
{
    __m128i tmp0 = _mm_unpacklo_epi32(*W0, *W1);
    __m128i tmp1 = _mm_unpackhi_epi32(*W0, *W1);
    __m128i tmp2 = _mm_unpacklo_epi32(*W2, *W3);
    __m128i tmp3 = _mm_unpackhi_epi32(*W2, *W3);
    *W0 = _mm_unpacklo_epi64(tmp0, tmp2);
    *W1 = _mm_unpackhi_epi64(tmp0, tmp2);
    *W2 = _mm_unpacklo_epi64(tmp1, tmp3);
    *W3 = _mm_unpackhi_epi64(tmp1, tmp3);
}
#endif

#if defined(TABLE)
void Precompute()
{
    // 预计算S盒的线性变换表
    for (int i = 0; i < 256; i++)
    {
        uint32_t B0 = (uint32_t)S[i] << 24;
        uint32_t B1 = (uint32_t)S[i] << 16;
        uint32_t B2 = (uint32_t)S[i] << 8;
        uint32_t B3 = (uint32_t)S[i];
        L0[i] = LinearTransform(B0);
        L1[i] = LinearTransform(B1);
        L2[i] = LinearTransform(B2);
        L3[i] = LinearTransform(B3);
        L0Prime[i] = LinearTransformPrime(B0);
        L1Prime[i] = LinearTransformPrime(B1);
        L2Prime[i] = LinearTransformPrime(B2);
        L3Prime[i] = LinearTransformPrime(B3);
    }
}
#elif defined(SIMD)
void Precompute()
{
    // 预计算S盒的SIMD版本
    uint8_t tmp[16] __attribute__((aligned(16)));
    for (int h = 0; h < 16; h++) // 遍历高4位
    {
        for (int l = 0; l < 16; l++)               // 遍历低4位
            tmp[l] = S[(h << 4) | l];              // 过S盒
        SSIMD[h] = _mm_load_si128((__m128i *)tmp); // 拼接
    }
    // 设置字节重排掩码
    ByteReMask = _mm_set_epi8(12, 13, 14, 15, 8, 9, 10, 11,
                              4, 5, 6, 7, 0, 1, 2, 3);
}
#endif

void GenKey(const uint8_t K[16], uint32_t RK[32])
{
    uint32_t MK[4], IK[4];

    // 切分密钥
    for (int i = 0; i < 4; i++)
        MK[i] = ((uint32_t)K[4 * i] << 24) |
                ((uint32_t)K[4 * i + 1] << 16) |
                ((uint32_t)K[4 * i + 2] << 8) |
                ((uint32_t)K[4 * i + 3]);

    // 初始化
    for (int i = 0; i < 4; i++)
        IK[i] = MK[i] ^ FK[i];

    // 轮密钥生成
    for (int i = 0; i < 4; i++)
    {
        RK[i] = TPrime(IK[i], IK[(i + 1) % 4],
                       IK[(i + 2) % 4], IK[(i + 3) % 4], RK[i]);
        IK[i] = RK[i];
    }
    for (int i = 4; i < 32; i += 4)
    {
        RK[i] = TPrime(RK[i - 4], RK[i - 3], RK[i - 2], RK[i - 1], CK[i]);
        RK[i + 1] = TPrime(RK[i - 3], RK[i - 2], RK[i - 1], RK[i], CK[i + 1]);
        RK[i + 2] = TPrime(RK[i - 2], RK[i - 1], RK[i], RK[i + 1], CK[i + 2]);
        RK[i + 3] = TPrime(RK[i - 1], RK[i], RK[i + 1], RK[i + 2], CK[i + 3]);
    }
}

void SM4(const uint32_t RK[32],
         const uint8_t in[16], uint8_t out[16],
         int flag)
{
    uint32_t X[4];

    // 切分输入
    for (int i = 0; i < 4; i++)
        X[i] = ((uint32_t)in[4 * i] << 24) |
               ((uint32_t)in[4 * i + 1] << 16) |
               ((uint32_t)in[4 * i + 2] << 8) |
               ((uint32_t)in[4 * i + 3]);

    // 轮迭代
    int index;
    uint32_t tmp[3];
    for (int i = 0; i < 32; i += 4)
    {
        index = flag ? (31 - i) : i;
        tmp[0] = T(X[0], X[1], X[2], X[3], RK[index]);
        index = flag ? (30 - i) : (i + 1);
        tmp[1] = T(X[1], X[2], X[3], tmp[0], RK[index]);
        index = flag ? (29 - i) : (i + 2);
        tmp[2] = T(X[2], X[3], tmp[0], tmp[1], RK[index]);
        index = flag ? (28 - i) : (i + 3);
        X[3] = T(X[3], tmp[0], tmp[1], tmp[2], RK[index]);
        X[0] = tmp[0];
        X[1] = tmp[1];
        X[2] = tmp[2];
    }

    // 合并输出
    for (int i = 0; i < 4; i++)
    {
        out[4 * i] = (X[3 - i] >> 24) & 0xFF;
        out[4 * i + 1] = (X[3 - i] >> 16) & 0xFF;
        out[4 * i + 2] = (X[3 - i] >> 8) & 0xFF;
        out[4 * i + 3] = X[3 - i] & 0xFF;
    }
}

#if defined(SIMD)
void SM4SIMD(const uint32_t RK[32],
             const uint8_t *in0, const uint8_t *in1,
             const uint8_t *in2, const uint8_t *in3,
             uint8_t *out0, uint8_t *out1,
             uint8_t *out2, uint8_t *out3,
             int flag)
{
    __m128i X[4], B[4];
    // 处理输入
    B[0] = ByteRe(_mm_loadu_si128((const __m128i *)in0));
    B[1] = ByteRe(_mm_loadu_si128((const __m128i *)in1));
    B[2] = ByteRe(_mm_loadu_si128((const __m128i *)in2));
    B[3] = ByteRe(_mm_loadu_si128((const __m128i *)in3));
    Transpose(&B[0], &B[1], &B[2], &B[3]);
    X[0] = B[0];
    X[1] = B[1];
    X[2] = B[2];
    X[3] = B[3];

    // 轮迭代
    int index;
    __m128i tmp[3];
    for (int i = 0; i < 32; i += 4)
    {
        index = flag ? (31 - i) : i;
        tmp[0] = TSIMD(X[0], X[1], X[2], X[3], _mm_set1_epi32(RK[index]));
        index = flag ? (30 - i) : (i + 1);
        tmp[1] = TSIMD(X[1], X[2], X[3], tmp[0], _mm_set1_epi32(RK[index]));
        index = flag ? (29 - i) : (i + 2);
        tmp[2] = TSIMD(X[2], X[3], tmp[0], tmp[1], _mm_set1_epi32(RK[index]));
        index = flag ? (28 - i) : (i + 3);
        X[3] = TSIMD(X[3], tmp[0], tmp[1], tmp[2], _mm_set1_epi32(RK[index]));
        X[0] = tmp[0];
        X[1] = tmp[1];
        X[2] = tmp[2];
    }

    // 处理输出
    B[0] = X[3];
    B[1] = X[2];
    B[2] = X[1];
    B[3] = X[0];
    Transpose(&B[0], &B[1], &B[2], &B[3]);
    _mm_storeu_si128((__m128i *)out0, ByteRe(B[0]));
    _mm_storeu_si128((__m128i *)out1, ByteRe(B[1]));
    _mm_storeu_si128((__m128i *)out2, ByteRe(B[2]));
    _mm_storeu_si128((__m128i *)out3, ByteRe(B[3]));
}

static inline void XORSIMD(uint8_t c[16],
                           const uint8_t a[16], const uint8_t b[16])
{
    __m128i va = _mm_loadu_si128((const __m128i *)a);
    __m128i vb = _mm_loadu_si128((const __m128i *)b);
    __m128i vc = _mm_xor_si128(va, vb);
    _mm_storeu_si128((__m128i *)c, vc);
}
#endif