// streamverifierpk.h -- Public-key signing streaming interface
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

#ifndef _S_STREAMVERIFIERPK_H_
#define _S_STREAMVERIFIERPK_H_

#include <sodium.h>

#include "common.h"
#include "key.h"
#include "keypairsign.h"

#include <stdexcept>
#include <istream>
#include <ostream>

namespace Sodium {

class StreamVerifierPK {
 public:

  static constexpr std::size_t KEYSIZE_PUBKEY = Sodium::KEYSIZE_PUBKEY_SIGN;
  static constexpr std::size_t SIGNATURE_SIZE = crypto_sign_BYTES;

  /**
   * A StreamVerifierPK will verify signatures on streams of
   * potentially unlimited length using the
   * crypto_sign_{init,update,final_verify}() libsodium API.
   *
   * The stream will be read in a blockwise fashion with blocks
   * of size at most blocksize bytes.
   * 
   * The constructor takes a public _signing_ Key of size
   * KEYSIZE_PUBKEY bytes. It will throw a std::runtime_error if the
   * key size isn't corect.
   **/

  StreamVerifierPK(const data_t      &pubkey,
		   const std::size_t blocksize) :
    pubkey_ {pubkey}, blocksize_ {blocksize} {
      if (pubkey.size() != KEYSIZE_PUBKEY)
	throw std::runtime_error {"Sodium::StreamVerifierPK() wrong key size"};
      if (blocksize < 1)
	throw std::runtime_error {"Sodium::StreamVerifierPK() wrong blocksize"};

      crypto_sign_init(&state_);
  }

  /**
   * A StreamVerifierPK will verify signatures on streams of
   * potentially unlimited length using the
   * crypto_sign_{init,update,final_verify}() libsodium API.
   *
   * The stream will be read in a blockwise fashion with blocks
   * of size at most blocksize bytes.
   * 
   * The constructor takes a KeyPairSign and uses the pubkey part of
   * it to verify the messages.
   **/
  StreamVerifierPK(const KeyPairSign &keypair,
		   const std::size_t blocksize) :
    pubkey_ {keypair.pubkey()}, blocksize_ {blocksize} {
      if (blocksize < 1)
	throw std::runtime_error {"Sodium::StreamVerifierPK() wrong blocksize"};
      
      crypto_sign_init(&state_);
  }
  
  /**
   * Verify the data provided by the std::istream istr, using the
   * public signing key provided by the constructor to compute a
   * signature. As soon as the stream reaches eof(), true or false is
   * returned, depending on whether the computed signature matches or
   * doesn't match the provided signature.
   *
   * The internal state_ is reset after verify() returns.
   * It is thus possible to call verify() multiple times.
   *
   * The stream is read() blockwise, using blocks of size up to
   * blocksize_ bytes.
   *
   * verify() will throw a std::runtime_error if the istr fails.
   **/
  
  bool verify(std::istream &istr,
	      const data_t &signature);
  
 private:
  data_t            pubkey_;
  crypto_sign_state state_;
  std::size_t       blocksize_;
};

} // namespace Sodium

#endif // _S_STREAMVERIFIERPK_H_
