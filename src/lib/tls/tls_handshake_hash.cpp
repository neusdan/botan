/*
* TLS Handshake Hash
* (C) 2004-2006,2011,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/tls_handshake_hash.h>
#include <botan/tls_exceptn.h>
#include <botan/hash.h>

namespace Botan {

namespace TLS {

/**
* Return a TLS Handshake Hash
*/
secure_vector<byte> Handshake_Hash::final(Protocol_Version version,
                                          const std::string& mac_algo) const
   {
   auto choose_hash = [=]() {
      if(!version.supports_ciphersuite_specific_prf())
         return "Parallel(MD5,SHA-160)";;

      if(mac_algo == "MD5" || mac_algo == "SHA-1")
         return "SHA-256";
      return mac_algo.c_str();
   };

   std::unique_ptr<HashFunction> hash(HashFunction::create(choose_hash()));
   hash->update(m_data);
   return hash->final();
   }

}

}
