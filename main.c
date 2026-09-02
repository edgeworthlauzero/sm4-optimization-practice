#include <sm4ctr.h>
#include <sm4cbc.h>
#include <x86intrin.h>

#define ROUND 10000

int main()
{
    // 测试数据
    uint8_t key[16] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};

    uint8_t iv[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    uint32_t rk[32];
    uint64_t start, end;
    uint64_t enc_cycles, dec_cycles;

#if defined(TABLE)
        printf("\nUse TABLE method\n");
    Precompute();
#elif defined(SIMD)
        printf("\nUse SIMD method\n");
    Precompute();
#endif

    // 生成密钥
    GenKey(key, rk);

    // 打开文件
    FILE *fp = fopen("text.txt", "rb");
    if (!fp)
        return 1;

    // 获取文件长度
    fseek(fp, 0, SEEK_END);
    long text_len = ftell(fp);          // 文件长度
    int pad_len = 16 - (text_len % 16); // 填充长度
    fseek(fp, 0, SEEK_SET);

    // 明文
    uint8_t *plaintext = (uint8_t *)malloc(text_len);
    // 密文
    uint8_t *ciphertext_ctr = (uint8_t *)malloc(text_len);
    uint8_t *ciphertext_cbc = (uint8_t *)malloc(text_len + pad_len);
    // 解密后明文
    uint8_t *recoverytext_ctr = (uint8_t *)malloc(text_len);
    uint8_t *recoverytext_cbc = (uint8_t *)malloc(text_len);
    
    // 读取文件内容作为明文
    fread(plaintext, 1, text_len, fp);

    // 关闭文件
    fclose(fp);

    // SM4-CTR加密解密
    printf("\nTest SM4-CTR\n");
    // 加密
    // 预热cache
    for (int i = 0; i < ROUND; i++)
        EncCTR(rk, iv, plaintext, ciphertext_ctr, text_len);
    // 开始计时
    start = __rdtsc();
    for (int i = 0; i < ROUND; i++)
        EncCTR(rk, iv, plaintext, ciphertext_ctr, text_len);
    end = __rdtsc();
    enc_cycles = (end - start) / ROUND;
    // 解密
    // 预热cache
    for (int i = 0; i < ROUND; i++)
        DecCTR(rk, iv, ciphertext_ctr, recoverytext_ctr, text_len);
    // 开始计时
    start = __rdtsc();
    for (int i = 0; i < ROUND; i++)
        DecCTR(rk, iv, ciphertext_ctr, recoverytext_ctr, text_len);
    end = __rdtsc();
    dec_cycles = (end - start) / ROUND;
    // 验证正确性
    if (memcmp(plaintext, recoverytext_ctr, text_len) == 0)
        printf("Test passed!\n");
    else
        printf("Test failed!\n");
    // 打印所用时钟周期数
    printf("Encryption average cycles: %lu\n", enc_cycles);
    printf("Decryption average cycles: %lu\n", dec_cycles);

    // SM4-CBC加密解密
    printf("\nTest SM4-CBC\n");
    // 加密
    // 预热cache
    for (int i = 0; i < ROUND; i++)
        EncCBC(rk, iv, plaintext, ciphertext_cbc, text_len);
    // 开始计时
    start = __rdtsc();
    for (int i = 0; i < ROUND; i++)
        EncCBC(rk, iv, plaintext, ciphertext_cbc, text_len);
    end = __rdtsc();
    enc_cycles = (end - start) / ROUND;
    // 解密
    // 预热cache
    for (int i = 0; i < ROUND; i++)
        DecCBC(rk, iv, ciphertext_cbc, recoverytext_cbc, text_len);
    // 开始计时
    start = __rdtsc();
    for (int i = 0; i < ROUND; i++)
        DecCBC(rk, iv, ciphertext_cbc, recoverytext_cbc, text_len);
    end = __rdtsc();
    dec_cycles = (end - start) / ROUND;
    // 验证正确性
    if (memcmp(plaintext, recoverytext_cbc, text_len) == 0)
        printf("Test passed!\n");
    else
        printf("Test failed!\n");
    // 打印所用时钟周期数
    printf("Encryption average cycles: %lu\n", enc_cycles);
    printf("Decryption average cycles: %lu\n", dec_cycles);

    // 交叉验证正确性
    if (memcmp(recoverytext_ctr, recoverytext_cbc, text_len) == 0)
        printf("\nTest between CTR and CBC passed!\n\n");
    else
        printf("\nTest between CTR and CBC failed!\n\n");

    // 释放内存
    free(plaintext);
    free(ciphertext_ctr);
    free(ciphertext_cbc);
    free(recoverytext_ctr);
    free(recoverytext_cbc);

    return 0;
}