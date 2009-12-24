/*
* CFB Mode
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#include <botan/cfb.h>
#include <botan/parsing.h>
#include <botan/internal/xor_buf.h>
#include <algorithm>

namespace Botan {

/*
* CFB Encryption Constructor
*/
CFB_Encryption::CFB_Encryption(BlockCipher* ciph, u32bit fback_bits)
   {
   cipher = ciph;
   feedback = fback_bits ? fback_bits / 8: cipher->BLOCK_SIZE;

   buffer.resize(cipher->BLOCK_SIZE);
   state.resize(cipher->BLOCK_SIZE);
   position = 0;

   if(feedback == 0 || fback_bits % 8 != 0 || feedback > cipher->BLOCK_SIZE)
      throw Invalid_Argument("CFB_Encryption: Invalid feedback size " +
                             to_string(fback_bits));
   }

/*
* CFB Encryption Constructor
*/
CFB_Encryption::CFB_Encryption(BlockCipher* ciph,
                               const SymmetricKey& key,
                               const InitializationVector& iv,
                               u32bit fback_bits)
   {
   cipher = ciph;
   feedback = fback_bits ? fback_bits / 8: cipher->BLOCK_SIZE;

   buffer.resize(cipher->BLOCK_SIZE);
   state.resize(cipher->BLOCK_SIZE);
   position = 0;

   if(feedback == 0 || fback_bits % 8 != 0 || feedback > cipher->BLOCK_SIZE)
      throw Invalid_Argument("CFB_Encryption: Invalid feedback size " +
                             to_string(fback_bits));

   set_key(key);
   set_iv(iv);
   }

void CFB_Encryption::set_iv(const InitializationVector& iv)
   {
   if(iv.length() != state.size())
      throw Invalid_IV_Length(name(), iv.length());

   state = iv.bits_of();
   buffer.clear();
   position = 0;

   cipher->encrypt(state, buffer);
   }

/*
* Encrypt data in CFB mode
*/
void CFB_Encryption::write(const byte input[], u32bit length)
   {
   while(length)
      {
      u32bit xored = std::min(feedback - position, length);
      xor_buf(buffer + position, input, xored);
      send(buffer + position, xored);
      input += xored;
      length -= xored;
      position += xored;

      if(position == feedback)
         {
         for(u32bit j = 0; j != cipher->BLOCK_SIZE - feedback; ++j)
            state[j] = state[j + feedback];
         state.copy(cipher->BLOCK_SIZE - feedback, buffer, feedback);
         cipher->encrypt(state, buffer);
         position = 0;
         }
      }
   }

/*
* CFB Decryption Constructor
*/
CFB_Decryption::CFB_Decryption(BlockCipher* ciph, u32bit fback_bits)
   {
   cipher = ciph;
   feedback = fback_bits ? fback_bits / 8: cipher->BLOCK_SIZE;

   buffer.resize(cipher->BLOCK_SIZE);
   state.resize(cipher->BLOCK_SIZE);
   position = 0;

   if(feedback == 0 || fback_bits % 8 != 0 || feedback > cipher->BLOCK_SIZE)
      throw Invalid_Argument("CFB_Decryption: Invalid feedback size " +
                             to_string(fback_bits));
   }

/*
* CFB Decryption Constructor
*/
CFB_Decryption::CFB_Decryption(BlockCipher* ciph,
                               const SymmetricKey& key,
                               const InitializationVector& iv,
                               u32bit fback_bits)
   {
   cipher = ciph;
   feedback = fback_bits ? fback_bits / 8: cipher->BLOCK_SIZE;

   buffer.resize(cipher->BLOCK_SIZE);
   state.resize(cipher->BLOCK_SIZE);
   position = 0;

   if(feedback == 0 || fback_bits % 8 != 0 || feedback > cipher->BLOCK_SIZE)
      throw Invalid_Argument("CFB_Decryption: Invalid feedback size " +
                             to_string(fback_bits));

   set_key(key);
   set_iv(iv);
   }

void CFB_Decryption::set_iv(const InitializationVector& iv)
   {
   if(iv.length() != state.size())
      throw Invalid_IV_Length(name(), iv.length());

   state = iv.bits_of();
   buffer.clear();
   position = 0;

   cipher->encrypt(state, buffer);
   }

/*
* Decrypt data in CFB mode
*/
void CFB_Decryption::write(const byte input[], u32bit length)
   {
   while(length)
      {
      u32bit xored = std::min(feedback - position, length);
      xor_buf(buffer + position, input, xored);
      send(buffer + position, xored);
      buffer.copy(position, input, xored);
      input += xored;
      length -= xored;
      position += xored;
      if(position == feedback)
         {
         for(u32bit j = 0; j != cipher->BLOCK_SIZE - feedback; ++j)
            state[j] = state[j + feedback];
         state.copy(cipher->BLOCK_SIZE - feedback, buffer, feedback);
         cipher->encrypt(state, buffer);
         position = 0;
         }
      }
   }

}
