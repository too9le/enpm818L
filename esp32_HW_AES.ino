#include "mbedtls/aes.h"

// AES-256 key (32 bytes)
const unsigned char key[32] = {
  0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
  0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
  0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};

unsigned char plaintext[32] = "ESP32 Hardware AES-256 Test Msg";
unsigned char encrypted[32];
unsigned char decrypted[32];

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Hardware AES-256 Encryption Demo");
  Serial.println("--------------------------------------");
  pinMode(12, OUTPUT);   // choose any GPIO pin you want
  
  Serial.print("AES-256 Key (hex): ");
  for (int i = 0; i < 32; i++) Serial.printf("%02X", key[i]);
  Serial.println();

  Serial.print("Original message: ");
  Serial.println((char*)plaintext);
  
  encryptAES(plaintext, encrypted);

  Serial.print("Encrypted message (hex): ");
  for (int i = 0; i < 32; i++) Serial.printf("%02X", encrypted[i]);
  Serial.println();

  decryptAES(encrypted, decrypted);

  Serial.print("Decrypted message: ");
  Serial.println((char*)decrypted);

  Serial.println("\nPerformance Benchmark");
  Serial.println("---------------------");
  benchmarkEncryption();

}

void loop() {}

// --- AES-256 ECB Encryption ---
void encryptAES(unsigned char* input, unsigned char* output) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 256);

  unsigned long startMicros = micros();
  for (int i = 0; i < 2; i++) {
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input + (i * 16), output + (i * 16));
  }
  unsigned long endMicros = micros();

  float encTimeUs = (endMicros - startMicros);
  Serial.printf("Encryption time: %.2f µs\n", encTimeUs);

  mbedtls_aes_free(&aes);
}

// --- AES-256 ECB Decryption ---
void decryptAES(unsigned char* input, unsigned char* output) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, key, 256);
  for (int i = 0; i < 2; i++) {
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input + (i * 16), output + (i * 16));
  }
  mbedtls_aes_free(&aes);
}

// --- Performance Benchmark 1 KB ---
void benchmarkEncryption() {
  const int dataSize = 1024;
  unsigned char* testData = (unsigned char*)malloc(dataSize);
  unsigned char* encryptedData = (unsigned char*)malloc(dataSize);
  for (int i = 0; i < dataSize; i++) testData[i] = i & 0xFF;

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 256);

  digitalWrite(12, HIGH);
  unsigned long startMicros = micros();
  // for (int block_num = 0;  block_num < 1024; block_num++) {
    for (int i = 0; i < dataSize / 16; i++) {
      mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, testData + (i * 16), encryptedData + (i * 16));
    }
  // }
  unsigned long endMicros = micros();
  digitalWrite(12, LOW);

  float timeMs = (endMicros - startMicros) / 1000.0;
  float throughputKBps = (dataSize / 1024.0) / (timeMs / 1000.0);

  Serial.printf("Time to encrypt 1 KB: %.3f ms\n", timeMs);
  Serial.printf("Encryption throughput: %.2f KB/s\n", throughputKBps);

  free(testData);
  free(encryptedData);
  mbedtls_aes_free(&aes);
}
