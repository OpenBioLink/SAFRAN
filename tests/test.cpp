// 010-TestCase.cpp
// And write tests in the same file:
#include <catch2/catch_test_macros.hpp>
#include "Util.hpp"
#include <vector>

TEST_CASE( "util in sorted", "[vector]" ) {

    std::vector<int>a {1,2,3,4,5,7,8,9,10};

    SECTION( "Binary search on vector with even number of elements" ) {

        REQUIRE( util::in_sorted(a, 1) == true );
        REQUIRE( util::in_sorted(a, 10) == true );
        REQUIRE( util::in_sorted(a, 5) == true );
        REQUIRE( util::in_sorted(a, -1) == false );
        REQUIRE( util::in_sorted(a, 0) == false );
        REQUIRE( util::in_sorted(a, 6) == false );
        REQUIRE( util::in_sorted(a, 11) == false );
    }
    SECTION( "Binary search on vector with odd number of elements" ) {
        a.push_back(11);
        REQUIRE( util::in_sorted(a, 1) == true );
        REQUIRE( util::in_sorted(a, 11) == true );
        REQUIRE( util::in_sorted(a, 5) == true );
        REQUIRE( util::in_sorted(a, -1) == false );
        REQUIRE( util::in_sorted(a, 0) == false );
        REQUIRE( util::in_sorted(a, 6) == false );
        REQUIRE( util::in_sorted(a, 12) == false );
    }
}