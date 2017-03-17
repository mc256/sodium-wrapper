// keypair.h -- Public/Private Key Pair Wrapper
//
// Copyright (C) 2017 Farid Hajji <farid@hajji.name>. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _S_KEYPAIR_H_
#define _S_KEYPAIR_H_

#include "key.h"

namespace Sodium {

class KeyPair
{
  /**
   * The class Sodium::KeyPair represents a pair of Public Key /
   * Private Key used in various public key cryptography functions of
   * the libsodium library.
   *
   * The public key is stored in unprotected (data_t) memory, while
   * the private key, being sensitive, is stored in protected (key_t) 
   * memory, i.e. in an internal Key object.
   *
   * A KeyPair provides non-mutable data()/size() access to the bytes
   * of the public/private keys in a uniform fashion via the
   * pubkey() and privkey() accessors.
   *
   * A key pair can be constructed randomly, or deterministically by
   * providing a seed. Furthermore, given a private key previously
   * generated by KeyPair or the underlying libsodium functions,
   * the corresponding public key can be derived and a KeyPair
   * constructed.
   **/

 public:
  // common constants for typical key and seed sizes
  static constexpr std::size_t KEYSIZE_PUBKEY      = Key::KEYSIZE_PUBKEY;
  static constexpr std::size_t KEYSIZE_PRIVKEY     = Key::KEYSIZE_PRIVKEY;
  static constexpr std::size_t KEYSIZE_SEEDBYTES   = Key::KEYSIZE_SEEDBYTES;

  /**
   * Generate a new (random) key pair of public/private keys.
   *
   * The created KeyPair contains a public key with KEYSIZE_PUBKEY bytes,
   * and a private key with KEYSIZE_PRIVKEY bytes. Both keys are related
   * and must be used together.
   *
   * Underlying libsodium function: crypto_box_keypair().
   *
   * the private key is stored in an internal Key object in protected
   * key_t memory (readonly). It will be wiped clean when the KeyPair
   * goes out of scope or is destroyed.
   *
   * The public key is stored in an internal data_t object in
   * unprotected (readwrite) memory.
   **/
  
  KeyPair()
    : privkey_(KEYSIZE_PRIVKEY, false), pubkey_(KEYSIZE_PUBKEY, '\0') {
    crypto_box_keypair(pubkey_.data(), privkey_.keydata.data());
    privkey_.readonly();
  }

  /**
   * Deterministically generate a key pair of public/private keys.
   *
   * The created KeyPair depends on a seed which must have KEYSIZE_SEEDBYTES
   * bytes. The same public/private keys will be generated for the same
   * seeds. Providing a seed of wrong size will throw a std::runtime_error.
   *
   * Underlying libsodium function: crypto_box_seed_keypair().
   *
   * Otherwise, see KeyPair().
   **/
  
  KeyPair(const data_t &seed)
    : privkey_(KEYSIZE_PRIVKEY, false), pubkey_(KEYSIZE_PUBKEY, '\0') {
    if (seed.size() != KEYSIZE_SEEDBYTES)
      throw std::runtime_error {"Sodium::KeyPair::KeyPair(seed) wrong seed size"};
    crypto_box_seed_keypair(pubkey_.data(), privkey_.keydata.data(),
			    seed.data());
    privkey_.readonly();
  }

  /**
   * Given a previously calculated private Key whose privkey_size
   * bytes are stored starting at privkey_data, derive the
   * corresponding public key pubkey, and construct with privkey_data
   * and pubkey a new KeyPair. privkey_data MUST point to
   * KEYSIZE_PRIVKEY bytes as shown by privkey_size, of course, or
   * this constructor will throw a std::runtime_error. The bytes at
   * privkey_data must be accessible or readable, or the program will
   * terminate.
   *
   * Underlying libsodium function: crypto_scalarmult_base().
   *
   * Note that the bytes at privkey_data MUST have been generated by
   * calculation, i.e.  by calls to KeyPair() constructors or
   * underlying libsodium functions. Undefined behavior results if
   * this is not the case.
   *
   * Otherwise, see KeyPair().
   **/
  
  KeyPair(const unsigned char *privkey_data, const std::size_t privkey_size)
    : privkey_(KEYSIZE_PRIVKEY, false), pubkey_(KEYSIZE_PUBKEY, '\0') {
    if (privkey_size != KEYSIZE_PRIVKEY)
      throw std::runtime_error {"Sodium::KeyPair::KeyPair(privkey_data, privkey_size) wrong privkey_size"};
    privkey_.keydata.assign(privkey_data, privkey_data+privkey_size); 
    
    // public key can be reconstructed from private key
    // previously computed by crypto_box_[seed_]keypair()!
    crypto_scalarmult_base(pubkey_.data(), privkey_.data());

    privkey_.readonly();
  }

  /**
   * Give const access to the stored private key as a Key object.
   *
   * This can be used to access the bytes of the private key via a
   * non-mutable data()/size() interface like this:
   *   <SOME_KEYPAIR>.privkey().data(), <SOME_KEYPAIR>.privkey().size()
   **/

  const Key privkey() const { return privkey_; }

  /**
   * Give const access to the stored public key as a data_t object.
   *
   * This can be used to access the bytes of the public key via a
   * non-mutable data()/size() interface like this:
   *  <SOME_KEYPAIR>.pubkey().data(), <SOME_KEYPAIR>.pubkey().size()
   **/

  const data_t pubkey() const { return pubkey_; }
  
 private:
  data_t pubkey_;
  Key    privkey_;
};
  
} // namespace Sodium

extern bool operator== (const Sodium::KeyPair &kp1, const Sodium::KeyPair &kp2);
extern bool operator!= (const Sodium::KeyPair &kp1, const Sodium::KeyPair &kp2);

#endif // _S_KEYPAIR_H_
