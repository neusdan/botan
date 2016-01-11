/*
* TLS Session Key
* (C) 2004-2006,2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TLS_SESSION_KEYS_H__
#define BOTAN_TLS_SESSION_KEYS_H__

#include <botan/symkey.h>

namespace Botan {

namespace TLS {

/**
* TLS Session Keys
*/
class Session_Keys
   {
   public:
      SymmetricKey client_cipher_key() const { return m_c_cipher; }
      SymmetricKey server_cipher_key() const { return m_s_cipher; }

      SymmetricKey client_mac_key() const { return m_c_mac; }
      SymmetricKey server_mac_key() const { return m_s_mac; }

      InitializationVector client_iv() const { return m_c_iv; }
      InitializationVector server_iv() const { return m_s_iv; }

      const secure_vector<byte>& master_secret() const { return m_master_sec; }

      Session_Keys() {}

      Session_Keys(const class Handshake_State* state,
                   const secure_vector<byte>& pre_master,
                   bool resuming);

   private:
      secure_vector<byte> m_master_sec;
      SymmetricKey m_c_cipher, m_s_cipher, m_c_mac, m_s_mac;
      InitializationVector m_c_iv, m_s_iv;
   };

}

}

#endif
