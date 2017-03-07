// cryptoraead.h -- Authenticated Encryption with Added Data
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

#ifndef _S_CRYPTORAEAD_H_
#define _S_CRYPTORAEAD_H_

#include "common.h"
#include "key.h"
#include "nonce.h"

namespace Sodium {

class CryptorAEAD
{
 public:
  static constexpr unsigned int NSZA    = Sodium::NONCESIZE_AEAD;
  static constexpr std::size_t  MACSIZE = crypto_aead_chacha20poly1305_ABYTES;
  
  /**
   * Encrypt plaintext using key and nonce. Compute a MAC from the ciphertext
   * and the attached plain header. Return a combination MAC+ciphertext.
   *
   * Any modification of the returned MAC+ciphertext, OR of the header, will
   * render decryption impossible. The intended application is to send
   * encrypted message bodies along with unencrypted message headers, but to
   * protect both the bodies and headers with the MAC. The nonce is public
   * and can be sent along the MAC+ciphertext. The key is private and MUST NOT
   * be sent over the channel.
   *
   * This function can be used repeately with the same key, but you MUST
   * then make sure never to reuse the same nonce. The easiest way to achieve
   * this is to increment nonce after or prior to each encrypt() invocation.
   * 
   * Limits: Up to 2^64 messages with the same key,
   *         Up to 2^70 bytes per message.
   *
   * The key   must be Sodium::Key::KEYSIZE_AEAD bytes long
   * The nonce must be Sodium::NONCESIZE_AEAD    bytes long
   *
   * The MAC+ciphertext size is 
   *    plaintext.size() + Sodium::CryptorAEAD::MACSIZE.
   **/

  data_t encrypt(const data_t      &header,
		 const data_t      &plaintext,
		 const Key         &key,
		 const Nonce<NSZA> &nonce);

  /**
   * Decrypt ciphertext_with_mac returned by Sodium::CryptorAEAD::encrypt()
   * along with plain header, using secret key, and public nonce.
   * 
   * If decryption succeeds, return plaintext.
   *
   * If the ciphertext, embedded MAC, or plain header have been tampered with,
   * or, in general, if the decryption doesn't succeed, throw a
   * std::runtime_error.
   * 
   * The key   must be Sodium::Key::KEYSIZE_AEAD bytes long
   * The nonce must be Sodium::NONCESIZE_AEAD    bytes long
   * 
   * The nonce can be public, the key must remain private. To successfully
   * decrypt a message, both the key and nonce must be the same as those
   * used when encrypting.
   **/

  data_t decrypt(const data_t      &header,
		 const data_t      &ciphertext_with_mac,
		 const Key         &key,
		 const Nonce<NSZA> &nonce);

};

} // namespace Sodium

#endif // _S_CRYPTORAEAD_H_