// streamcryptor.h -- Symmetric blockwise stream encryption/decryption
//
// ISC License
// 
// Copyright (c) 2017 Farid Hajji <farid@hajji.name>
// 
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef _S_STREAMCRYPTOR_H_
#define _S_STREAMCRYPTOR_H_

#include <sodium.h>

#include "common.h"
#include "key.h"
#include "nonce.h"
#include "cryptoraead.h"

#include <stdexcept>
#include <istream>
#include <ostream>

namespace Sodium {

class StreamCryptor {
 public:

  /**
   * We encrypt with AEAD.
   **/
  constexpr static std::size_t KEYSIZE = Sodium::KEYSIZE_AEAD;
  
  /**
   * Each block of plaintext will be encrypted to a block of the same
   * size of ciphertext, combined with a MAC of size MACSIZE.  Note
   * that the total blocksize of the (MAC || ciphertext)s will be
   * MACSIZE + plaintext.size() for each block.
   **/
  constexpr static std::size_t MACSIZE = CryptorAEAD::MACSIZE;

  /**
   * A StreamCryptor will encrypt/decrypt streams blockwise using a
   * CryptorAEAD as the crypto engine.
   * 
   * Each block of size blocksize from the input stream is encrypted
   * or decrypted in turn, using Authenticated Encryption with Added
   * Data to detect tampering of the ciphertext. The added plain text
   * data is for each block an empty header. Each block is encrypted
   * with the same key, but with a monotonically incremented nonce.
   * 
   * The constructor saves a copy of the key of KEYSIZE bytes, and a
   * copy of a CryptorAEAD::nonce_type nonce for later use in its
   * internal state. Furthermore, it also saves the desired blocksize
   * that will be used for both encryption and decryption of the
   * streams.
   *
   * If the key size isn't correct, or the blocksize doesn't make sense,
   * the constructor throws a std::runtime_error.
   **/
  
 StreamCryptor(const CryptorAEAD::key_type   &key,
	       const CryptorAEAD::nonce_type &nonce,
	       const std::size_t blocksize) :
  key_ {key}, nonce_ {nonce}, header_ {}, blocksize_ {blocksize} {
    // some sanity checks, before we start
    if (blocksize < 1)
      throw std::runtime_error {"Sodium::StreamCryptor::StreamCryptor(): wrong blocksize"};
    key_.readonly();
  }

  /**
   * Encrypt data read from input stream istr in a blockwise fashion,
   * writing (MAC || ciphertext) blocks to the output stream ostr.
   *
   * The input stream istr is read blockwise in chunks of size blocksize,
   * where blocksize has been passed in the constructor. The final block
   * by have less than blocksize bytes.
   * 
   * The encyption is performed by the CryptorAEAD crypto engine, using
   * the saved key, and a running nonce that starts with the initial
   * nonce passed at construction time, and whose copy is incremented
   * for each chunk.
   *
   * As soon as a chunk has been encrypted, it is written to the
   * output stream ostr.
   *
   * Note that each written chunk contains both the ciphertext for the
   * original chunk read from istr, as well as the authenticated MAC
   * of size MACSIZE, computed from both the ciphertext, as well as
   * from an empty plaintext header.  This additional MAC occurs every
   * chunk, and helps the decrypt() function verify the integrity of
   * the chunk (and MAC), should it have been tampered with.
   *
   * The saved nonce is not affected by the incrementing of the
   * running nonce. It can thus be reused to decrypt() a stream
   * encrypt()ed by *this.
   *
   * If an error occurs while writing to ostr, throw a std::runtime_error.
   **/
  
  void encrypt(std::istream &istr, std::ostream &ostr);

  /**
   * Decrypt data read from input stream istr in a blockwise fashion,
   * writing the plaintext to the output stream ostr.
   *
   * The input stream istr is assumed to have been generated by encrypt()
   * using the same key, (initial) nonce, and blocksize. Otherweise,
   * decryption will fail and a std::runtime_error will the thrown.
   *
   * The input stream istr is read blockwise in chunks of size MACSIZE
   * + blocksize, because each chunk in the encrypted stream is
   * assumed to have been combined with an authenticating MAC, i.e. to
   * be of the form (MAC || ciphertext).  The final block may have
   * less than MACSIZE + blocksize bytes, but should have at least
   * MACSIZE bytes left.
   *
   * The decryption is attempted by the CryptorAEAD crypto engine, using
   * the saved key, and a running nonce that starts with the initial
   * nonce passed at construction time, and whose copy is incremented
   * with each chunk.
   *
   * As soon as a chunk has been decrypted, it is written to the output
   * stream ostr.
   * 
   * Decryption can fail if
   *   - the key was wrong
   *   - the initial nonce was wrong
   *   - the blocksize was wrong
   *   - the input stream wasn't encrypted with encrypt()
   *   - one or more (MAC || ciphertext) chunks have been tampered with
   * In that case, throw a std::runtime_error and stop writing to ostr.
   * No strong guarantee w.r.t. ostr.
   *
   * The saved nonce is unaffected by the incrementing of the running
   * nonce during decryption.
   **/
  
  void decrypt(std::istream &istr, std::ostream &ostr);
  
 private:
  CryptorAEAD::key_type   key_;
  CryptorAEAD::nonce_type nonce_;
  data_t                  header_;
  std::size_t             blocksize_;
  
  CryptorAEAD             sc_aead_;
};

} // namespace Sodium

#endif // _S_STREAMCRYPTOR_H_
