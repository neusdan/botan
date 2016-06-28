/*
* KDF1 from ISO 18033
* (C) 2016 Philipp Weber
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_KDF1_18033_H__
#define BOTAN_KDF1_18033_H__

#include <botan/kdf.h>
#include <botan/hash.h>

namespace Botan {

/**
* KDF1, from IEEE 1363
*/
class BOTAN_DLL KDF1_18033 : public KDF
   {
   public:
      std::string name() const override { return "KDF1-18033(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF1_18033(m_hash->clone()); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len,
                 const byte label[], size_t label_len) const override;

      KDF1_18033(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}

#endif
