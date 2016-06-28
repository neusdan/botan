/*
* Entropy Source Using Intel's rdrand instruction
* (C) 2012,2015 Jack Lloyd
* (C) 2015 Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/rdrand.h>
#include <botan/cpuid.h>
#include <botan/build.h>

#if !defined(BOTAN_USE_GCC_INLINE_ASM)
  #include <immintrin.h>
#endif

namespace Botan {

void Intel_Rdrand::poll(Entropy_Accumulator& accum) {
   if(!CPUID::has_rdrand())
      return;

   for(size_t p = 0; p != BOTAN_ENTROPY_INTEL_RNG_POLLS; ++p)
      {
      for(size_t i = 0; i != BOTAN_ENTROPY_RDRAND_RETRIES; ++i)
         {
         uint32_t r = 0;

#if defined(BOTAN_USE_GCC_INLINE_ASM)
         int cf = 0;

         // Encoding of rdrand %eax
         asm(".byte 0x0F, 0xC7, 0xF0; adcl $0,%1" :
             "=a" (r), "=r" (cf) : "0" (r), "1" (cf) : "cc");
#else
         int cf = _rdrand32_step(&r);
#endif
         if(1 == cf)
            {
            accum.add(r, BOTAN_ENTROPY_ESTIMATE_HARDWARE_RNG);
            break;
            }
         }
      }
   }

}
