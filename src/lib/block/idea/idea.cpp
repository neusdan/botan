/*
* IDEA
* (C) 1999-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/idea.h>
#include <botan/loadstor.h>
#include <botan/internal/ct_utils.h>

namespace Botan {

namespace {

/*
* Multiplication modulo 65537
*/
inline u16bit mul(u16bit x, u16bit y)
   {
   const u32bit P = static_cast<u32bit>(x) * y;

   const u16bit Z_mask = static_cast<u16bit>(CT::expand_mask(P) & 0xFFFF);

   const u32bit P_hi = P >> 16;
   const u32bit P_lo = P & 0xFFFF;

   const u16bit r_1 = (P_lo - P_hi) + (P_lo < P_hi);
   const u16bit r_2 = 1 - x - y;

   return CT::select(Z_mask, r_1, r_2);
   }

/*
* Find multiplicative inverses modulo 65537
*
* 65537 is prime; thus Fermat's little theorem tells us that
* x^65537 == x modulo 65537, which means
* x^(65537-2) == x^-1 modulo 65537 since
* x^(65537-2) * x == 1 mod 65537
*
* Do the exponentiation with a basic square and multiply: all bits are
* of exponent are 1 so we always multiply
*/
u16bit mul_inv(u16bit x)
   {
   u16bit y = x;

   for(size_t i = 0; i != 15; ++i)
      {
      y = mul(y, y); // square
      y = mul(y, x);
      }

   return y;
   }

/**
* IDEA is involutional, depending only on the key schedule
*/
void idea_op(const byte in[], byte out[], size_t blocks, const u16bit K[52])
   {
   const size_t BLOCK_SIZE = 8;

   CT::poison(in, blocks * 8);
   CT::poison(out, blocks * 8);
   CT::poison(K, 52);

   for(size_t i = 0; i != blocks; ++i)
      {
      u16bit X1 = load_be<u16bit>(in + BLOCK_SIZE*i, 0);
      u16bit X2 = load_be<u16bit>(in + BLOCK_SIZE*i, 1);
      u16bit X3 = load_be<u16bit>(in + BLOCK_SIZE*i, 2);
      u16bit X4 = load_be<u16bit>(in + BLOCK_SIZE*i, 3);

      for(size_t j = 0; j != 8; ++j)
         {
         X1 = mul(X1, K[6*j+0]);
         X2 += K[6*j+1];
         X3 += K[6*j+2];
         X4 = mul(X4, K[6*j+3]);

         u16bit T0 = X3;
         X3 = mul(X3 ^ X1, K[6*j+4]);

         u16bit T1 = X2;
         X2 = mul((X2 ^ X4) + X3, K[6*j+5]);
         X3 += X2;

         X1 ^= X2;
         X4 ^= X3;
         X2 ^= T0;
         X3 ^= T1;
         }

      X1  = mul(X1, K[48]);
      X2 += K[50];
      X3 += K[49];
      X4  = mul(X4, K[51]);

      store_be(out + BLOCK_SIZE*i, X1, X3, X2, X4);
      }

   CT::unpoison(in, blocks * 8);
   CT::unpoison(out, blocks * 8);
   CT::unpoison(K, 52);
   }

}

/*
* IDEA Encryption
*/
void IDEA::encrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   idea_op(in, out, blocks, m_EK.data());
   }

/*
* IDEA Decryption
*/
void IDEA::decrypt_n(const byte in[], byte out[], size_t blocks) const
   {
   idea_op(in, out, blocks, m_DK.data());
   }

/*
* IDEA Key Schedule
*/
void IDEA::key_schedule(const byte key[], size_t)
   {
   m_EK.resize(52);
   m_DK.resize(52);

   CT::poison(key, 16);
   CT::poison(m_EK.data(), 52);
   CT::poison(m_DK.data(), 52);

   for(size_t i = 0; i != 8; ++i)
      m_EK[i] = load_be<u16bit>(key, i);

   for(size_t i = 1, j = 8, offset = 0; j != 52; i %= 8, ++i, ++j)
      {
      m_EK[i+7+offset] = static_cast<u16bit>((m_EK[(i     % 8) + offset] << 9) |
                                           (m_EK[((i+1) % 8) + offset] >> 7));
      offset += (i == 8) ? 8 : 0;
      }

   m_DK[51] = mul_inv(m_EK[3]);
   m_DK[50] = -m_EK[2];
   m_DK[49] = -m_EK[1];
   m_DK[48] = mul_inv(m_EK[0]);

   for(size_t i = 1, j = 4, counter = 47; i != 8; ++i, j += 6)
      {
      m_DK[counter--] = m_EK[j+1];
      m_DK[counter--] = m_EK[j];
      m_DK[counter--] = mul_inv(m_EK[j+5]);
      m_DK[counter--] = -m_EK[j+3];
      m_DK[counter--] = -m_EK[j+4];
      m_DK[counter--] = mul_inv(m_EK[j+2]);
      }

   m_DK[5] = m_EK[47];
   m_DK[4] = m_EK[46];
   m_DK[3] = mul_inv(m_EK[51]);
   m_DK[2] = -m_EK[50];
   m_DK[1] = -m_EK[49];
   m_DK[0] = mul_inv(m_EK[48]);

   CT::unpoison(key, 16);
   CT::unpoison(m_EK.data(), 52);
   CT::unpoison(m_DK.data(), 52);
   }

void IDEA::clear()
   {
   zap(m_EK);
   zap(m_DK);
   }

}
