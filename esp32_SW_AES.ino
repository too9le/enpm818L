#include <AESLib.h>

AESLib aesLib;

// AES-256 key (32 bytes)
const byte key[32] = {
  0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
  0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
  0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
  0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4
};

// Dummy IV (required by AESLib)
byte dummy_iv[16] = { 0 };

byte plaintext[32]  = "ESP32 Software AES-256 Test";
byte ciphertext[32];
byte decrypted[33];  // +1 for null terminator

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Software AES-256 Encryption Demo");
  Serial.println("--------------------------------------");

  pinMode(12, OUTPUT);   // choose any GPIO pin you want
  digitalWrite(12, LOW);

  // --- Key ---
  Serial.print("AES-256 Key (hex): ");
  for (int i = 0; i < 32; i++) Serial.printf("%02X", key[i]);
  Serial.println();

  Serial.print("Original message: ");
  Serial.println((char*)plaintext);

  // --- Encrypt ---
  memset(dummy_iv, 0, sizeof(dummy_iv));  // always reset IV before encrypt
  unsigned long startMicros = micros();
  aesLib.encrypt(plaintext, sizeof(plaintext), ciphertext, key, 256, dummy_iv);
  unsigned long endMicros = micros();
  float encTimeUs = (endMicros - startMicros);
  Serial.printf("Encryption time: %.2f µs\n", encTimeUs);

  Serial.print("Encrypted message (hex): ");
  for (int i = 0; i < 32; i++) Serial.printf("%02X", ciphertext[i]);
  Serial.println();

  // --- Decrypt ---
  memset(dummy_iv, 0, sizeof(dummy_iv));  // reset IV before decrypt
  aesLib.decrypt(ciphertext, sizeof(ciphertext), decrypted, key, 256, dummy_iv);

  // manually add null terminator
  decrypted[32] = '\0';

  Serial.print("Decrypted message: ");
  Serial.println((char*)decrypted);

  // --- Benchmark ---
  Serial.println("\nPerformance Benchmark");
  Serial.println("---------------------");
  benchmarkEncryption();
}

void loop() {}

// -------------------------------
// 1 KB AES-256 ECB Benchmark
// -------------------------------
void benchmarkEncryption() {
  const int dataSize = 1024;  // 1 KB
  byte* testData      = (byte*)malloc(dataSize);
  byte* encryptedData = (byte*)malloc(dataSize);

  for (int i = 0; i < dataSize; i++) testData[i] = i & 0xFF;

  memset(dummy_iv, 0, sizeof(dummy_iv));
  unsigned long startMicros = micros();
  digitalWrite(12, HIGH);
  for (int block_num = 0; block_num <1024; block_num++) {
    for (int i = 0; i < dataSize / 16; i++) {
      aesLib.encrypt(testData + (i * 16), 16, encryptedData + (i * 16), key, 256, dummy_iv);
    }
  }
  unsigned long endMicros = micros();
  digitalWrite(12, LOW);

  float timeMs = (endMicros - startMicros) / 1000.0;
  float throughputKBps = (dataSize / 1024.0) / (timeMs / 1000.0);

  Serial.printf("Time to encrypt 1 KB: %.3f ms\n", timeMs);
  Serial.printf("Encryption throughput: %.2f KB/s\n", throughputKBps);

  free(testData);
  free(encryptedData);
}
