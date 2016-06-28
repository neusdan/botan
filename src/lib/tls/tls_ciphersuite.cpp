/*
* TLS Cipher Suite
* (C) 2004-2010,2012,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/tls_ciphersuite.h>
#include <botan/parsing.h>
#include <botan/block_cipher.h>
#include <botan/stream_cipher.h>
#include <botan/hash.h>
#include <botan/mac.h>
#include <sstream>

namespace Botan {

namespace TLS {

namespace {

/*
* This way all work happens at the constuctor call, and we can
* rely on that happening only once in C++11.
*/
std::vector<Ciphersuite> gather_known_ciphersuites()
   {
   std::vector<Ciphersuite> ciphersuites;

   std::vector<u16bit> all_ids = Ciphersuite::all_known_ciphersuite_ids();

   for(auto id : all_ids)
      {
      Ciphersuite suite = Ciphersuite::by_id(id);

      if(suite.valid())
         ciphersuites.push_back(suite);
      }

   return ciphersuites;
   }

}

const std::vector<Ciphersuite>& Ciphersuite::all_known_ciphersuites()
   {
   static std::vector<Ciphersuite> all_ciphersuites(gather_known_ciphersuites());
   return all_ciphersuites;
   }

bool Ciphersuite::is_scsv(u16bit suite)
   {
   // TODO: derive from IANA file in script
   return (suite == 0x00FF || suite == 0x5600);
   }

bool Ciphersuite::psk_ciphersuite() const
   {
   return (kex_algo() == "PSK" ||
           kex_algo() == "DHE_PSK" ||
           kex_algo() == "ECDHE_PSK");
   }

bool Ciphersuite::ecc_ciphersuite() const
   {
   return (sig_algo() == "ECDSA" || kex_algo() == "ECDH" || kex_algo() == "ECDHE_PSK");
   }

namespace {

bool have_hash(const std::string& prf)
   {
   return (HashFunction::providers(prf).size() > 0);
   }

bool have_cipher(const std::string& cipher)
   {
   return (BlockCipher::providers(cipher).size() > 0) ||
      (StreamCipher::providers(cipher).size() > 0);
   }

}

bool Ciphersuite::valid() const
   {
   if(!m_cipher_keylen) // uninitialized object
      return false;

   if(!have_hash(prf_algo()))
      return false;

   if(mac_algo() == "AEAD")
      {
      if(cipher_algo() == "ChaCha20Poly1305")
         {
#if !defined(BOTAN_HAS_AEAD_CHACHA20_POLY1305)
         return false;
#endif
         }
      else
         {
         auto cipher_and_mode = split_on(cipher_algo(), '/');
         BOTAN_ASSERT(cipher_and_mode.size() == 2, "Expected format for AEAD algo");
         if(!have_cipher(cipher_and_mode[0]))
            return false;

         const auto mode = cipher_and_mode[1];

#if !defined(BOTAN_HAS_AEAD_CCM)
         if(mode == "CCM" || mode == "CCM-8")
            return false;
#endif

#if !defined(BOTAN_HAS_AEAD_GCM)
         if(mode == "GCM")
            return false;
#endif

#if !defined(BOTAN_HAS_AEAD_OCB)
         if(mode == "OCB(12)" || mode == "OCB")
            return false;
#endif
         }
      }
   else
      {
      // Old non-AEAD schemes
      if(!have_cipher(cipher_algo()))
         return false;
      if(!have_hash(mac_algo())) // HMAC
         return false;
      }

   if(kex_algo() == "SRP_SHA")
      {
#if !defined(BOTAN_HAS_SRP6)
      return false;
#endif
      }
   else if(kex_algo() == "ECDH" || kex_algo() == "ECDHE_PSK")
      {
#if !defined(BOTAN_HAS_ECDH)
      return false;
#endif
      }
   else if(kex_algo() == "DH" || kex_algo() == "DHE_PSK")
      {
#if !defined(BOTAN_HAS_DIFFIE_HELLMAN)
      return false;
#endif
      }

   if(sig_algo() == "DSA")
      {
#if !defined(BOTAN_HAS_DSA)
      return false;
#endif
      }
   else if(sig_algo() == "ECDSA")
      {
#if !defined(BOTAN_HAS_ECDSA)
      return false;
#endif
      }
   else if(sig_algo() == "RSA")
      {
#if !defined(BOTAN_HAS_RSA)
      return false;
#endif
      }

   return true;
   }

}

}

