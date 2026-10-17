<div align="center">

# 🔐 Secure Chat

### End-to-End Encrypted Messaging — Built from Scratch in C

[![Language](https://img.shields.io/badge/Language-C-blue?style=flat-square&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Crypto](https://img.shields.io/badge/Crypto-ECC%20%2B%20AES--GCM-green?style=flat-square)](https://en.wikipedia.org/wiki/Elliptic-curve_cryptography)
[![Platform](https://img.shields.io/badge/Platform-Linux-orange?style=flat-square&logo=linux)](https://kernel.org)
[![Hardware](https://img.shields.io/badge/AES-NI-Hardware%20Accelerated-red?style=flat-square)](https://en.wikipedia.org/wiki/AES_instruction_set)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

*A complete, production-grade cryptographic stack — no crypto libraries, no shortcuts, no magic boxes.*

</div>

---

## What is this?

This is a two-party encrypted chat application over TCP, where **every layer of the cryptographic stack is implemented from scratch in C**. No OpenSSL EVP. No libsodium. No mbedTLS. The only external dependency is a single call to `RAND_bytes` for cryptographically secure random number generation.

The full stack, all hand-written:

- **256-bit finite field arithmetic** over a custom prime, using a radix-2²⁹ limb representation
- **Elliptic curve operations** in Jacobian coordinates with the Montgomery ladder
- **ECDH key exchange** for zero-transmission shared secret agreement
- **SHA-256** for key derivation from the shared secret
- **AES-128-GCM** using Intel AES-NI hardware intrinsics for authenticated encryption

---

## How the connection works

```
Client                                        Server
  │                                               │
  │  ─── TCP Connect to 127.0.0.1:8888 ────────► │
  │                                               │
  │  Generate ephemeral key pair (dA, QA)         │  Generate ephemeral key pair (dB, QB)
  │  QA = dA · G                                  │  QB = dB · G
  │                                               │
  │  ◄──────────── Send QB (128 hex chars) ─────  │
  │  ─────────────  Send QA (128 hex chars) ────► │
  │                                               │
  │  S = dA · QB                                  │  S = dB · QA  (same point, by ECDH)
  │  K = SHA-256(S.x ║ S.y)[0..15]               │  K = SHA-256(S.x ║ S.y)[0..15]
  │                                               │
  │  ◄══════════ AES-128-GCM messages ══════════► │
```

The shared secret `S` is never transmitted. Both sides arrive at the same 128-bit AES session key independently — the core property of Diffie-Hellman.

---

## Cryptographic Architecture

### Layer 1 — Finite Field Arithmetic (`gfP.h`)

All arithmetic happens in the prime field **𝔽ₚ** where:

```
p = 0xE92E40AD6F281C8A082AFDC49E1372659455BEC8CEEA043A614C835B7FE9EFF5
```

**Representation:** Each 256-bit field element is stored as an array of **9 limbs, each 29 bits wide** (radix 2²⁹). This keeps intermediate products within 58 bits, safely below the 64-bit limit — no 128-bit types, no compiler tricks.

**Implemented operations:**

| Function | What it does |
|---|---|
| `mult9` / `mult10` | Schoolbook long multiplication on 9- or 10-limb integers |
| `barrett` | Barrett reduction — replaces expensive division with two multiplications |
| `square` | Optimized squaring exploiting `(a+b)² = a² + 2ab + b²` symmetry |
| `mult` | Full multiplication with Barrett reduction |
| `add` / `mod_sub` | Modular addition and subtraction |
| `times2` / `times3` | Fast multiply-by-constant mod p |
| `mod_inv` | Modular inverse via Fermat's little theorem: `a^(p−2) mod p` |
| `modexp_rtl` / `modexp_ltr` | Binary exponentiation, both scan directions |

**Timing resistance:** Every conditional selection — including the post-reduction step in `add`, `mod_sub`, `times2`, and `times3` — uses **bitmask operations** instead of `if` branches to prevent data-dependent timing variations.

```c
// Example: constant-time conditional select (from mod_sub)
u64 mask = -sub(a, b, result);   // mask = 0xFFF...F if a < b, else 0x000...0
result[i] = (temp[i] & ~mask) ^ (result[i] & mask);
```

---

### Layer 2 — Elliptic Curve Operations (`ecc.h`)

The curve is **y² = x³ + ax + b** over 𝔽ₚ with:

```
a = 0xE92E40AD6F281C8A082AFDC49E1372659455BEC8CEEA043A614C835B7FE9EFF2
b = 1
```

The base point **G** is `(0, 1)` (lifted to Jacobian with Z = 1).

**Jacobian coordinates:** Points are stored as `(X : Y : Z)` where the affine point is `(X/Z², Y/Z³)`. This eliminates the need for a modular inverse in every intermediate step — inversions only happen once when converting back to affine.

**Scalar multiplication:** Uses the **Montgomery ladder**, which has a fixed execution pattern regardless of the scalar value:

```c
// Montgomery ladder core (from point_mult)
for (int i = 254; i >= 0; i--) {
    u8 sb = (scalar[8 - (i/29)] >> (i%29)) & 0x1;
    point_double(**result, inter);
    point_add(inter[0], P, inter + 1);
    *result = inter + sb;   // select R0 or R1 based on bit
}
```

Every iteration performs exactly one double and one addition — the bit value only determines *which* result is kept, not *whether* work is done.

| Function | Description |
|---|---|
| `point_double` | Doubling in Jacobian coordinates |
| `point_add` | Addition in Jacobian coordinates |
| `point_mult` | Scalar multiplication via the Montgomery ladder |
| `PtoA` | Projective → affine conversion (one modular inverse) |
| `key_validation` | Validates a point is on the curve with the correct group order |
| `are_equal` | Constant-time 256-bit integer equality check |
| `are_negative` | Checks if two points are additive inverses |

---

### Layer 3 — Key Exchange (`final.h`)

Connects the ECC primitives to the application. Handles all serialization between the 9-limb internal representation and the hex strings sent over the wire.

| Function | Description |
|---|---|
| `rand_str` | Generates a random 256-bit scalar via `RAND_bytes`, clamped to `< p` |
| `parse_to_int` | 64-char hex string → 9-limb radix-2²⁹ integer |
| `parse_to_hex` | 9-limb integer → 64-char hex string |
| `pub_point_generation` | Picks random `d`, computes `Q = d·G`, serializes Q to 128 hex chars |
| `shared_key_generation` | Computes `S = d·Q_peer`, hashes via SHA-256, returns first 16 bytes as AES key |

The public key is 128 hex characters — 64 for X and 64 for Y, each coordinate encoded as a big-endian 256-bit integer.

---

### Layer 4 — SHA-256 (`sha256.h`)

A self-contained, dependency-free SHA-256 with the standard streaming interface:

```
sha256_update  →  sha256_transform (64-round compression)  →  sha256_final
```

Used to stretch the ECDH shared point into a uniform 256-bit value, of which the first 128 bits become the AES session key. This ensures the key material has full entropy and no exploitable structure from the EC point representation.

---

### Layer 5 — AES-128-GCM (`gcm.h`)

Hardware-accelerated using **Intel AES-NI** and **CLMUL** intrinsics. Requires a CPU with AES-NI, SSE2, and PCLMULQDQ (any x86-64 processor made after ~2010).

**Encryption pipeline:**

```
Plaintext
    │
    ▼
AES-CTR encrypt  ──────────────────────────────────────────────►  Ciphertext
    │                                                                  │
    │                                                                  ▼
    │                AAD ──► GHASH over (AAD ║ Ciphertext ║ Lengths)
    │                                                                  │
    └──────────────────────────────────────────────────────────────── Authentication Tag
```

**GF(2¹²⁸) multiplication (`gfmul`)** uses `_mm_clmulepi64_si128` (carry-less multiply) — the hardware instruction designed specifically for GCM's polynomial arithmetic.

| Function | Description |
|---|---|
| `AES_128_Key_Expansion` | Expands 128-bit key into 11 round keys using AES-NI key schedule |
| `encrypt` | Encrypts a single 128-bit block with 10 rounds of AES-NI |
| `aes_ctr_encrypt` | CTR-mode stream cipher, handles partial final block correctly |
| `gfmul` | GF(2¹²⁸) multiply via `_mm_clmulepi64_si128` |
| `ghash` | GHASH universal hash for authentication |
| `aes_gcm_encrypt` | Encrypt + authenticate, returns hex-encoded ciphertext with tag |
| `aes_gcm_verify_and_decrypt` | Verify tag first, decrypt on success, abort on failure |

The AAD (Additional Authenticated Data) includes the **sender's IP address**. This means the recipient can cryptographically verify not just that the message is intact, but that it came from the expected source. Any attempt to replay or re-route a message will fail tag verification.

---

## Project Structure

```
.
├── gfP.h        ← 256-bit prime field arithmetic (radix-2²⁹ limbs, Barrett reduction)
├── ecc.h        ← EC point operations (Jacobian coords, Montgomery ladder scalar mult)
├── sha256.h     ← Self-contained SHA-256
├── gcm.h        ← AES-128-GCM with AES-NI + CLMUL hardware acceleration
├── final.h      ← ECDH key exchange, hex serialization, session key derivation
├── server.c     ← TCP server: ECDH handshake, pthread send/receive loop
├── client.c     ← TCP client: ECDH handshake, pthread send/receive loop
└── test.c       ← Development test harness for ECC operations
```

Each `.h` file is self-contained and independently testable. The dependency chain is:

```
gfP.h  ←  ecc.h  ←  final.h  ←  server.c / client.c
                       ↑
                    sha256.h    gcm.h
```

---

## Prerequisites

- **Linux** — uses POSIX sockets (`<arpa/inet.h>`) and `pthread`
- **GCC** — with x86-64 intrinsic support (standard on any modern install)
- **OpenSSL dev headers** — for `RAND_bytes` only

```bash
# Ubuntu / Debian
sudo apt update && sudo apt install gcc libssl-dev

# Fedora / RHEL
sudo dnf install gcc openssl-devel
```

---

## Build & Run

**1. Compile:**

```bash
# Server
gcc -maes -msse2 -mpclmul -o server server.c -lcrypto

# Client
gcc -maes -msse2 -mpclmul -o client client.c -lcrypto
```

**2. Start the server** (Terminal 1):

```bash
./server
# Server is listening on port 8888...
```

**3. Connect the client** (Terminal 2):

```bash
./client
# ====== Welcome ======
# (send 'bye' to leave)
```

**4. Chat.** Messages are encrypted before they leave your terminal. Type `bye` on either side to disconnect.

**Compiler flags:**

| Flag | Purpose |
|---|---|
| `-maes` | Unlock `_mm_aesenc_si128` and friends (AES-NI) |
| `-msse2` | Enable 128-bit SSE2 registers |
| `-mpclmul` | Enable `_mm_clmulepi64_si128` (carry-less multiply for GCM) |
| `-lcrypto` | Link OpenSSL for `RAND_bytes` |

**Test harness** (ECC verification):

```bash
gcc -maes -msse2 -mpclmul -o test test.c -lcrypto
./test
```

---

## Security Properties

| Property | How it's achieved |
|---|---|
| **Confidentiality** | AES-128 in CTR mode |
| **Integrity** | GCM authentication tag — any single-bit flip is detected |
| **Authenticity** | GHASH over ciphertext + AAD — tag covers sender IP |
| **Key Agreement** | ECDH over a 256-bit elliptic curve |
| **Forward Secrecy** | Fresh ephemeral key pair every session |
| **Timing Resistance** | Bitmask-based branching throughout field and curve arithmetic |
| **Randomness** | `RAND_bytes` (OpenSSL CSPRNG) for private key generation |

---

## Known Limitations

**Single client at a time.** The server calls `accept` once and enters the chat loop. Supporting multiple simultaneous clients would require a per-connection thread or event loop.

**Custom, non-standard curve.** The curve parameters were chosen for this project and have not been publicly scrutinized. A production system should use Curve25519 or NIST P-256.

**No mutual authentication.** ECDH does not prevent a man-in-the-middle from substituting their own public key during the handshake. A real deployment would need certificates or a pre-shared identity mechanism.

**Counter initialized to 0x01 per session.** This is safe because each session derives a fresh AES key, so the `(key, nonce)` pair is never reused. It would become a problem if the same key were used across sessions.

**Linux only.** Porting to Windows requires replacing POSIX sockets with Winsock2 and `pthread` with the Windows threading API.

---

## Implementation Notes

This project was written as a deep-dive into the mathematics underlying modern cryptography. The interesting parts:

- **Why radix 2²⁹?** Choosing 29-bit limbs is a precision tradeoff: a 256-bit number needs `⌈256/29⌉ = 9` limbs, and multiplying two 29-bit values gives at most 58 bits — fitting safely in a 64-bit `uint64_t`. Wider limbs would overflow.

- **Why Barrett reduction and not Montgomery?** Barrett is slightly simpler to implement correctly when you already have a schoolbook multiplier. Montgomery reduction would be faster in practice (avoids the quotient estimate) but requires re-representing all inputs, which adds code complexity.

- **Why the Montgomery ladder?** Most scalar multiplication algorithms (double-and-add, windowed NAF) leak the scalar through their branching pattern — a timing side-channel. The Montgomery ladder executes an identical sequence of operations for every bit, regardless of its value.

- **Why Jacobian coordinates?** Converting to affine requires a modular inverse, which costs ~256 squarings and multiplications. In Jacobian coordinates you can do hundreds of point additions before a single final conversion to affine, amortizing that cost across the entire scalar multiplication.

---

<div align="center">

Built from first principles — big integers, elliptic curves, SHA-256, and AES-GCM — all the way up to an encrypted chat session.

</div>
