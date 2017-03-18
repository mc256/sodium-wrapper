// auth.cpp -- Secret Key Authentication (MAC)
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

#include "auth.h"
#include "key.h"

#include <stdexcept>

using Sodium::data_t;
using Sodium::Auth;
using Sodium::Key;

data_t
Auth::auth (const data_t &plaintext,
	    const Key    &key)
{
  // get the sizes
  const std::size_t key_size = Auth::KEYSIZE_AUTH;
  const std::size_t mac_size = Auth::MACSIZE;
  
  // some sanity checks before we get started
  if (key.size() != key_size)
    throw std::runtime_error {"Sodium::Auth::auth() key wrong size"};

  // make space for MAC
  data_t mac(mac_size);
  
  // let's compute the MAC now!
  crypto_auth (mac.data(),
	       plaintext.data(), plaintext.size(),
	       key.data());

  // return the MAC bytes
  return mac;
}

bool
Auth::verify (const data_t &plaintext,
	      const data_t &mac,
	      const Key    &key)
{
  // get the sizes
  const std::size_t mac_size = mac.size();
  const std::size_t key_size = key.size();
  
  // some sanity checks before we get started
  if (mac_size != Auth::MACSIZE)
    throw std::runtime_error {"Sodium::Auth::verify() mac wrong size"};
  if (key_size != Auth::KEYSIZE_AUTH)
    throw std::runtime_error {"Sodium::Auth::verify() key wrong size"};

  // and now verify!
  return crypto_auth_verify (mac.data(),
			     plaintext.data(), plaintext.size(),
			     key.data()) == 0;
}
