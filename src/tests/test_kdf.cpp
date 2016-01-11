/*
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "tests.h"

#if defined(BOTAN_HAS_KDF_BASE)
  #include <botan/kdf.h>
#endif

namespace Botan_Tests {

namespace {

#if defined(BOTAN_HAS_KDF_BASE)
class KDF_KAT_Tests : public Text_Based_Test
   {
   public:
      KDF_KAT_Tests() : Text_Based_Test("kdf",
                                        {"OutputLen", "Salt", "Secret", "Output"},
                                        {"IKM","XTS"})
         {}

      Test::Result run_one_test(const std::string& kdf_name, const VarMap& vars) override
         {
         Test::Result result(kdf_name);

         std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(kdf_name));

         if(!kdf)
            {
            result.note_missing(kdf_name);
            return result;
            }

         const size_t outlen = get_req_sz(vars, "OutputLen");
         const std::vector<uint8_t> salt = get_opt_bin(vars, "Salt");
         const std::vector<uint8_t> secret = get_req_bin(vars, "Secret");
         const std::vector<uint8_t> expected = get_req_bin(vars, "Output");

         result.test_eq("derived key", kdf->derive_key(outlen, secret, salt), expected);

         return result;
         }

   };

BOTAN_REGISTER_TEST("kdf", KDF_KAT_Tests);

#endif

}

}
