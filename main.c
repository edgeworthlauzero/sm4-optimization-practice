#include <sm4ctr.h>
#include <x86intrin.h>

#define ROUND 10000

// 打印十六进制数据
void printhex(const uint8_t *label, const uint8_t *data, int len)
{
    printf("%s ", label);
    for (int i = 0; i < len; i++)
        printf("%02x", data[i]);
    printf("\n");
}

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

#if defined(TABLE)
    printf("\nUse TABLE method\n");
    Precompute();
#elif defined(SIMD)
    printf("\nUse SIMD method\n");
    Precompute();
#endif

    // 生成密钥
    GenKey(key, rk);
    
    // 读取文件
    FILE *fp = fopen("text.txt", "rb");
    if (!fp)
        return 1;
    fseek(fp, 0, SEEK_END);
    long text_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *plaintext = (uint8_t *)malloc(text_len);
    uint8_t *ciphertext = (uint8_t *)malloc(text_len);
    uint8_t *decryptedtext = (uint8_t *)malloc(text_len);
    fread(plaintext, 1, text_len, fp);
    fclose(fp);

    // SM4-CTR加密解密
    printf("\nTest SM4-CTR\n");
    // 加密
    // 预热cache
    for (int i = 0; i < ROUND; i++)
        EncCTR(rk, iv, plaintext, ciphertext, text_len);
    // 开始计时
    start = __rdtsc();
    for (int i = 0; i < ROUND; i++)
        EncCTR(rk, iv, plaintext, ciphertext, text_len);
    end = __rdtsc();
    uint64_t enc_cycles = (end - start) / ROUND;
    // 解密
    // 预热cache
    for (int i = 0; i < ROUND; i++)
        DecCTR(rk, iv, ciphertext, decryptedtext, text_len);
    // 开始计时
    start = __rdtsc();
    for (int i = 0; i < ROUND; i++)
        DecCTR(rk, iv, ciphertext, decryptedtext, text_len);
    end = __rdtsc();
    uint64_t dec_cycles = (end - start) / ROUND;
    // 验证正确性
    if (memcmp(plaintext, decryptedtext, text_len) == 0)
        printf("Test passed!\n");
    else
        printf("Test failed!\n");
    // 打印所用时钟周期数
    printf("Encryption average cycles: %lu\n", enc_cycles);
    printf("Decryption average cycles: %lu\n\n", dec_cycles);

    // 释放内存
    free(plaintext);
    free(ciphertext);
    free(decryptedtext);

    return 0;
}