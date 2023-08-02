#include <sstream>
#include <utility>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "inc/md5.h"

using namespace std::string_literals;

SCENARIO("md5 hash should be computed", "[md5]") {
    GIVEN("an input and an expected result") {
        auto [input, result] = GENERATE(
            // MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
            std::pair{ ""s, "d41d8cd98f00b204e9800998ecf8427e"s },
            // MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
            std::pair{ "a"s, "0cc175b9c0f1b6a831c399e269772661"s },
            // MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
            std::pair{ "abc"s, "900150983cd24fb0d6963f7d28e17f72"s },
            // MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
            std::pair{ "message digest"s, "f96b697d7cb7938d525a2f31aaf161d0"s },
            // MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
            std::pair{ "abcdefghijklmnopqrstuvwxyz"s, "c3fcd3d76192e4007dfb496cca67e13b"s },
            // MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") = d174ab98d277d9f5a5611c2c9f419d9f
            std::pair{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"s, "d174ab98d277d9f5a5611c2c9f419d9f"s },
            // MD5 ("12345678901234567890123456789012345678901234567890123456789012345678901234567890") = 57edf4a22be3c955ac49da2e2107b67a
            std::pair{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890"s, "57edf4a22be3c955ac49da2e2107b67a"s });
        
        WHEN("the input is used") {
            std::istringstream din{ input };
            THEN("the result should be correct") {
                REQUIRE(hash(din) == result);
            }
        }
    }
}






