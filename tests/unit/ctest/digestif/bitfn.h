/*
 * Copyright (C) 2006-2009 Vincent Hanquez <vincent@snarc.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// clang-format off
#ifndef BITFN_H
#define BITFN_H

#include <stdint.h>
#include <string.h>

#if !defined(__cpluplus) && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L)
  #if defined(_MSC_VER)
    #define __INLINE __inline
  #elif defined(__GNUC__)
    #define __INLINE __inline__
  #else
    #define __INLINE
  #endif
#else
  #define __INLINE inline
#endif

static __INLINE void secure_zero_memory(void *v, size_t n)
{
  static void *(*const volatile memset_v)(void *, int, size_t) = &memset;
  memset_v(v, 0, n);
}

#ifndef NO_INLINE_ASM
/**********************************************************/
# if (defined(__i386__))
#  define ARCH_HAS_SWAP32
static __INLINE uint32_t bitfn_swap32(uint32_t a)
{
	__asm__ ("bswap %0" : "=r" (a) : "0" (a));
	return a;
}
/**********************************************************/
# elif (defined(__arm__))
#  define ARCH_HAS_SWAP32
static __INLINE uint32_t bitfn_swap32(uint32_t a)
{
	uint32_t tmp = a;
	__asm__ volatile ("eor %1, %0, %0, ror #16\n"
	                  "bic %1, %1, #0xff0000\n"
	                  "mov %0, %0, ror #8\n"
	                  "eor %0, %0, %1, lsr #8\n"
	                  : "=r" (a), "=r" (tmp) : "0" (a), "1" (tmp));
	return a;
}
/**********************************************************/
# elif defined(__x86_64__)
#  define ARCH_HAS_SWAP32
#  define ARCH_HAS_SWAP64
static __INLINE uint32_t bitfn_swap32(uint32_t a)
{
	__asm__ ("bswap %0" : "=r" (a) : "0" (a));
	return a;
}

static __INLINE uint64_t bitfn_swap64(uint64_t a)
{
	__asm__ ("bswap %0" : "=r" (a) : "0" (a));
	return a;
}

# endif
#endif /* NO_INLINE_ASM */
/**********************************************************/

#ifndef ARCH_HAS_ROL32
static __INLINE uint32_t rol32(uint32_t word, uint32_t shift)
{
	return (word << shift) | (word >> (32 - shift));
}
#endif

#ifndef ARCH_HAS_ROR32
static __INLINE uint32_t ror32(uint32_t word, uint32_t shift)
{
	return (word >> shift) | (word << (32 - shift));
}
#endif

#ifndef ARCH_HAS_ROL64
static __INLINE uint64_t rol64(uint64_t word, uint32_t shift)
{
	return (word << shift) | (word >> (64 - shift));
}
#endif

#ifndef ARCH_HAS_ROR64
static __INLINE uint64_t ror64(uint64_t word, uint32_t shift)
{
	return (word >> shift) | (word << (64 - shift));
}
#endif

#ifndef ARCH_HAS_SWAP32
static __INLINE uint32_t bitfn_swap32(uint32_t a)
{
	return (a << 24) | ((a & 0xff00) << 8) | ((a >> 8) & 0xff00) | (a >> 24);
}
#endif

#ifndef ARCH_HAS_ARRAY_SWAP32
static __INLINE void array_swap32(uint32_t *d, uint32_t *s, uint32_t nb)
{
	while (nb--)
		*d++ = bitfn_swap32(*s++);
}
#endif

#ifndef ARCH_HAS_SWAP64
static __INLINE uint64_t bitfn_swap64(uint64_t a)
{
	return ((uint64_t) bitfn_swap32((uint32_t) (a >> 32))) |
	       (((uint64_t) bitfn_swap32((uint32_t) a)) << 32);
}
#endif

#ifndef ARCH_HAS_ARRAY_SWAP64
static __INLINE void array_swap64(uint64_t *d, uint64_t *s, uint32_t nb)
{
	while (nb--)
		*d++ = bitfn_swap64(*s++);
}
#endif

#ifndef ARCH_HAS_MEMORY_ZERO
static __INLINE void memory_zero(void *ptr, uint32_t len)
{
	uint32_t *ptr32 = ptr;
	uint8_t *ptr8;
	int i;

	for (i = 0; (uint32_t) i < len / 4; i++)
		*ptr32++ = 0;
	if (len % 4) {
		ptr8 = (uint8_t *) ptr32;
		for (i = len % 4; i >= 0; i--)
			ptr8[i] = 0;
	}
}
#endif

#ifndef ARCH_HAS_ARRAY_COPY32
static __INLINE void array_copy32(uint32_t *d, uint32_t *s, uint32_t nb)
{
	while (nb--) *d++ = *s++;
}
#endif

#ifndef ARCH_HAS_ARRAY_COPY64
static __INLINE void array_copy64(uint64_t *d, uint64_t *s, uint32_t nb)
{
	while (nb--) *d++ = *s++;
}
#endif

static __INLINE uint64_t load64( const void *src )
{
#if defined(NATIVE_LITTLE_ENDIAN)
  uint64_t w;
  memcpy(&w, src, sizeof w);
  return w;
#else
  const uint8_t *p = ( const uint8_t * )src;
  return (( uint64_t )( p[0] ) <<  0) |
         (( uint64_t )( p[1] ) <<  8) |
         (( uint64_t )( p[2] ) << 16) |
         (( uint64_t )( p[3] ) << 24) |
         (( uint64_t )( p[4] ) << 32) |
         (( uint64_t )( p[5] ) << 40) |
         (( uint64_t )( p[6] ) << 48) |
         (( uint64_t )( p[7] ) << 56) ;
#endif
}

static __INLINE uint32_t load32( const void *src )
{
#if defined(NATIVE_LITTLE_ENDIAN)
  uint32_t w;
  memcpy(&w, src, sizeof w);
  return w;
#else
  const uint8_t *p = ( const uint8_t * )src;
  return (( uint32_t )( p[0] ) <<  0) |
         (( uint32_t )( p[1] ) <<  8) |
         (( uint32_t )( p[2] ) << 16) |
         (( uint32_t )( p[3] ) << 24) ;
#endif
}

static __INLINE void store32( void *dst, uint32_t w )
{
#if defined(NATIVE_LITTLE_ENDIAN)
  memcpy(dst, &w, sizeof w);
#else
  uint8_t *p = ( uint8_t * )dst;
  p[0] = (uint8_t)(w >>  0);
  p[1] = (uint8_t)(w >>  8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
#endif
}

static __INLINE void store64( void *dst, uint64_t w )
{
#if defined(NATIVE_LITTLE_ENDIAN)
  memcpy(dst, &w, sizeof w);
#else
  uint8_t *p = ( uint8_t * )dst;
  p[0] = (uint8_t)(w >>  0);
  p[1] = (uint8_t)(w >>  8);
  p[2] = (uint8_t)(w >> 16);
  p[3] = (uint8_t)(w >> 24);
  p[4] = (uint8_t)(w >> 32);
  p[5] = (uint8_t)(w >> 40);
  p[6] = (uint8_t)(w >> 48);
  p[7] = (uint8_t)(w >> 56);
#endif
}

#ifdef __MINGW32__
  # define LITTLE_ENDIAN 1234
  # define BYTE_ORDER    LITTLE_ENDIAN
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
  # include <sys/endian.h>
#elif defined(__OpenBSD__) || defined(__SVR4)
  # include <sys/types.h>
#elif defined(__APPLE__)
  # include <machine/endian.h>
#elif defined( BSD ) && ( BSD >= 199103 )
  # include <machine/endian.h>
#elif defined( __QNXNTO__ ) && defined( __LITTLEENDIAN__ )
  # define LITTLE_ENDIAN 1234
  # define BYTE_ORDER    LITTLE_ENDIAN
#elif defined( __QNXNTO__ ) && defined( __BIGENDIAN__ )
  # define BIG_ENDIAN 1234
  # define BYTE_ORDER    BIG_ENDIAN
#elif defined(_MSC_VER)
  # define LITTLE_ENDIAN	1234
  # define BYTE_ORDER	    LITTLE_ENDIAN
#else
  # include <endian.h>
#endif
/* big endian to cpu */
#if LITTLE_ENDIAN == BYTE_ORDER

# define be32_to_cpu(a) bitfn_swap32(a)
# define cpu_to_be32(a) bitfn_swap32(a)
# define le32_to_cpu(a) (a)
# define cpu_to_le32(a) (a)
# define be64_to_cpu(a) bitfn_swap64(a)
# define cpu_to_be64(a) bitfn_swap64(a)
# define le64_to_cpu(a) (a)
# define cpu_to_le64(a) (a)

# define cpu_to_le32_array(d, s, l) array_copy32(d, s, l)
# define le32_to_cpu_array(d, s, l) array_copy32(d, s, l)
# define cpu_to_be32_array(d, s, l) array_swap32(d, s, l)
# define be32_to_cpu_array(d, s, l) array_swap32(d, s, l)

# define cpu_to_le64_array(d, s, l) array_copy64(d, s, l)
# define le64_to_cpu_array(d, s, l) array_copy64(d, s, l)
# define cpu_to_be64_array(d, s, l) array_swap64(d, s, l)
# define be64_to_cpu_array(d, s, l) array_swap64(d, s, l)

# define ror32_be(a, s) rol32(a, s)
# define rol32_be(a, s) ror32(a, s)

# define ARCH_IS_LITTLE_ENDIAN

#elif BIG_ENDIAN == BYTE_ORDER

# define be32_to_cpu(a) (a)
# define cpu_to_be32(a) (a)
# define be64_to_cpu(a) (a)
# define cpu_to_be64(a) (a)
# define le64_to_cpu(a) bitfn_swap64(a)
# define cpu_to_le64(a) bitfn_swap64(a)
# define le32_to_cpu(a) bitfn_swap32(a)
# define cpu_to_le32(a) bitfn_swap32(a)

# define cpu_to_le32_array(d, s, l) array_swap32(d, s, l)
# define le32_to_cpu_array(d, s, l) array_swap32(d, s, l)
# define cpu_to_be32_array(d, s, l) array_copy32(d, s, l)
# define be32_to_cpu_array(d, s, l) array_copy32(d, s, l)

# define cpu_to_le64_array(d, s, l) array_swap64(d, s, l)
# define le64_to_cpu_array(d, s, l) array_swap64(d, s, l)
# define cpu_to_be64_array(d, s, l) array_copy64(d, s, l)
# define be64_to_cpu_array(d, s, l) array_copy64(d, s, l)

# define ror32_be(a, s) ror32(a, s)
# define rol32_be(a, s) rol32(a, s)

# define ARCH_IS_BIG_ENDIAN

#else
# error "endian not supported"
#endif

#endif /* !BITFN_H */
// clang-format on
