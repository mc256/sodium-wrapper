// test_Cryptor.cpp -- Test Sodium::Cryptor
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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Sodium::Cryptor Test
#include <boost/test/included/unit_test.hpp>

#include <sodium.h>
#include "cryptor.h"
#include <string>

using Sodium::Cryptor;

using data_t = Sodium::data_t;

bool
test_of_correctness(const std::string &plaintext,
		    bool falsify_ciphertext=false,
		    bool falsify_mac=false,
		    bool falsify_key=false,
		    bool falsify_nonce=false)
{
  Cryptor             sc    {};
  Cryptor::key_type   key;
  Cryptor::key_type   key2;
  Cryptor::nonce_type nonce {};
  Cryptor::nonce_type nonce2 {};

  data_t plainblob {plaintext.cbegin(), plaintext.cend()};

  data_t ciphertext = sc.encrypt(plainblob, key, nonce);

  BOOST_CHECK(ciphertext.size() == Cryptor::MACSIZE + plainblob.size());
  
  if (! plaintext.empty() && falsify_ciphertext) {
    // ciphertext is of the form: (MAC || actual_ciphertext)
    ++ciphertext[Cryptor::MACSIZE]; // falsify ciphertext
  }
  
  if (falsify_mac) {
    // ciphertext is of the form: (MAC || actual_ciphertext)
    ++ciphertext[0]; // falsify MAC
  }

  try {
    data_t decrypted  = sc.decrypt(ciphertext,
				   (falsify_key ? key2 : key),
				   (falsify_nonce ? nonce2 : nonce));

    BOOST_CHECK(decrypted.size()  == plainblob.size());

    // decryption succeeded and plainblob == decrypted if and only if
    // we didn't falsify the ciphertext nor the MAC nor the key nor the nonce
    
    return !falsify_ciphertext &&
      !falsify_mac &&
      !falsify_key &&
      !falsify_nonce &&
      (plainblob == decrypted);
  }
  catch (std::exception &e) {
    // decryption failed. This is expected if and only if we falsified
    // the ciphertext OR we falsified the MAC
    // OR we falsified the key
    // OR we falsified the nonce

    return falsify_ciphertext || falsify_mac || falsify_key || falsify_nonce;
  }

  // NOTREACHED (hopefully)
  return false;
}

bool
test_of_correctness_detached(const std::string &plaintext,
			     bool falsify_ciphertext=false,
			     bool falsify_mac=false,
			     bool falsify_key=false,
			     bool falsify_nonce=false)
{
  Cryptor             sc    {};
  Cryptor::key_type   key;
  Cryptor::key_type   key2;
  Cryptor::nonce_type nonce {};
  Cryptor::nonce_type nonce2 {};

  data_t plainblob {plaintext.cbegin(), plaintext.cend()};
  data_t mac(Cryptor::MACSIZE);

  data_t ciphertext = sc.encrypt(plainblob, key, nonce, mac);

  BOOST_CHECK(ciphertext.size() == plainblob.size());
  
  if (! plaintext.empty() && falsify_ciphertext)
    ++ciphertext[0]; // falsify ciphertext

  if (falsify_mac)
    ++mac[0]; // falsify MAC

  try {
    data_t decrypted  = sc.decrypt(ciphertext,
				   mac,
				   (falsify_key   ? key2   : key),
				   (falsify_nonce ? nonce2 : nonce));

    BOOST_CHECK(decrypted.size()  == plainblob.size());

    // decryption succeeded and plainblob == decrypted if and only if
    // we didn't falsify the ciphertext nor the MAC nor the key nor the nonce
    
    return !falsify_ciphertext &&
      !falsify_mac &&
      !falsify_key &&
      !falsify_nonce &&
      (plainblob == decrypted);
  }
  catch (std::exception &e) {
    // decryption failed. This is expected if and only if we falsified
    // the ciphertext OR we falsified the MAC
    // OR falsified the key
    // OR falsified the nonce

    return falsify_ciphertext || falsify_mac || falsify_key || falsify_nonce;
  }

  // NOTREACHED (hopefully)
  return false;
}

struct SodiumFixture {
  SodiumFixture()  {
    BOOST_REQUIRE(sodium_init() != -1);
    BOOST_TEST_MESSAGE("SodiumFixture(): sodium_init() successful.");
  }
  ~SodiumFixture() {
    BOOST_TEST_MESSAGE("~SodiumFixture(): teardown -- no-op.");
  }
};

BOOST_FIXTURE_TEST_SUITE ( sodium_test_suite, SodiumFixture );

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_full_plaintext )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness(plaintext));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_empty_plaintext )
{
  std::string plaintext {};
  BOOST_CHECK(test_of_correctness(plaintext));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_full_plaintext_detached )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness_detached(plaintext));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_empty_plaintext_detached )
{
  std::string plaintext {};
  BOOST_CHECK(test_of_correctness_detached(plaintext));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_ciphertext )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness(plaintext, true, false, false, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_mac )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness(plaintext, false, true, false, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_key )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness(plaintext, false, false, true, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_nonce )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness(plaintext, false, false, false, true));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_mac_empty )
{
  std::string plaintext {};
  BOOST_CHECK(test_of_correctness(plaintext, false, true));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_ciphertext_and_mac )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness(plaintext, true, true, false, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_ciphertext_detached )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness_detached(plaintext, true, false, false, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_mac_detached )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness_detached(plaintext, false, true, false, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_key_detached )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness_detached(plaintext, false, false, true, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_nonce_detached )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness_detached(plaintext, false, false, false, true));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_mac_empty_detached )
{
  std::string plaintext {};
  BOOST_CHECK(test_of_correctness_detached(plaintext, false, true, false, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_key_empty_detached )
{
  std::string plaintext {};
  BOOST_CHECK(test_of_correctness_detached(plaintext, false, false, true, false));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_nonce_empty_detached )
{
  std::string plaintext {};
  BOOST_CHECK(test_of_correctness_detached(plaintext, false, false, false, true));
}

BOOST_AUTO_TEST_CASE( sodium_cryptor_test_falsify_ciphertext_and_mac_detached )
{
  std::string plaintext {"the quick brown fox jumps over the lazy dog"};
  BOOST_CHECK(test_of_correctness_detached(plaintext, true, true, false, false));
}

BOOST_AUTO_TEST_SUITE_END ();
