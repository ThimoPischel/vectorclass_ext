#include <iostream>
#include <vector>
#include <array>

#define ANKERL_NANOBENCH_IMPLEMENT
#include "nanobench.h"

#define VECEX_OVERRIDE
#include "vectorclass_ext.h"

#define TYPE float
#define SIZE 100000
#define MINIT 1000

void
bench1() {
    std::vector<TYPE> a;
    std::vector<TYPE> b;
    std::vector<TYPE> result;

    for ( int i = 0; i < SIZE; i++ ) {
        a.push_back( i );
        b.push_back( i );
        result.push_back( i );
    }
    b.push_back( 4 );
    ankerl::nanobench::Rng().shuffle( a );
    ankerl::nanobench::Rng().shuffle( b );


    ankerl::nanobench::doNotOptimizeAway( a );
    ankerl::nanobench::doNotOptimizeAway( b );

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Normal",
            [&]() {
                std::vector<TYPE> c( SIZE );
                c.resize( SIZE );
                for ( size_t i = 0; i < SIZE; i++ ) {
                    c[i] = a[i] + b[i];
                }
                ankerl::nanobench::doNotOptimizeAway( c );
            } );
    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Normal IN",
            [&]() {
                for ( size_t i = 0; i < SIZE; i++ ) {
                    result[i] = a[i] + b[i];
                }
                ankerl::nanobench::doNotOptimizeAway( result );
            } );

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Vector Override",
            [&]() {
                auto c = a + b;
                ankerl::nanobench::doNotOptimizeAway( c );
            } );
    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Vector IN",
            [&]() {
                vecex::add_in( a, b, result );
                ankerl::nanobench::doNotOptimizeAway( result );
            } );

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Compute",
            [&]() {
                vecex::compute(
                        std::array { a.data(), b.data(), result.data() },
                        SIZE,
                        []( auto& ctx ) {
                            auto a = ctx.load( 0 );
                            auto b = ctx.load( 1 );
                            auto c = a + b;
                            ctx.store( c, 2 );
                        } );
                ankerl::nanobench::doNotOptimizeAway( result );
            } );
}

void
bench_complex() {
    std::vector<TYPE> a;
    std::vector<TYPE> b;
    std::vector<TYPE> c;
    std::vector<TYPE> d;
    std::vector<TYPE> e;
    std::vector<TYPE> f;
    a.resize( SIZE );
    b.resize( SIZE );
    c.resize( SIZE );
    d.resize( SIZE );
    e.resize( SIZE );
    f.resize( SIZE );

    for ( int i = 0; i < SIZE; i++ ) {
        a[i] = i;
        b[i] = i;
        c[i] = i;
        d[i] = i;
        e[i] = i;
        f[i] = i;
    }

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Complex Normal IN",
            [&]() {
                for ( size_t i = 0; i < SIZE; i++ ) {
                    c[i] = a[i] + b[i];
                    d[i] = b[i] + c[i];
                    e[i] = c[i] * b[i];
                    f[i] = d[i] * b[i];
                    a[i] = f[i] - 3;
                    f[i] = d[i] - a[i];
                }
                ankerl::nanobench::doNotOptimizeAway( a );
                ankerl::nanobench::doNotOptimizeAway( b );
                ankerl::nanobench::doNotOptimizeAway( c );
                ankerl::nanobench::doNotOptimizeAway( d );
                ankerl::nanobench::doNotOptimizeAway( e );
                ankerl::nanobench::doNotOptimizeAway( f );
            } );

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Complex Vector Override",
            [&]() {
                c = a + b;
                d = b + c;
                e = c * b;
                f = d * b;
                a = f - TYPE( 3 );
                f = d - a;
                ankerl::nanobench::doNotOptimizeAway( a );
                ankerl::nanobench::doNotOptimizeAway( b );
                ankerl::nanobench::doNotOptimizeAway( c );
                ankerl::nanobench::doNotOptimizeAway( d );
                ankerl::nanobench::doNotOptimizeAway( e );
                ankerl::nanobench::doNotOptimizeAway( f );
            } );

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Complex Vector IN",
            [&]() {
                vecex::add_in( a, b, c );
                vecex::add_in( b, c, d );
                vecex::mul_in( c, b, e );
                vecex::mul_in( d, b, f );
                vecex::sub_in( f, TYPE( 3 ), a );
                ankerl::nanobench::doNotOptimizeAway( a );
                ankerl::nanobench::doNotOptimizeAway( b );
                ankerl::nanobench::doNotOptimizeAway( c );
                ankerl::nanobench::doNotOptimizeAway( d );
                ankerl::nanobench::doNotOptimizeAway( e );
                ankerl::nanobench::doNotOptimizeAway( f );
            } );

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Complex compute",
            [&]() {
                vecex::compute(
                        std::array { a.data(),
                                     b.data(),
                                     c.data(),
                                     d.data(),
                                     e.data(),
                                     f.data() },
                        SIZE,
                        []( auto& ctx ) {
                            auto a = ctx.load( 0 );
                            auto b = ctx.load( 1 );
                            auto c = a + b;
                            auto d = b + c;
                            auto e = c * b;
                            auto f = d * b;
                            auto an = f - TYPE( 3 );
                            f = d - an;
                            ctx.store( an, 0 );
                            ctx.store( c, 2 );
                            ctx.store( d, 3 );
                            ctx.store( e, 4 );
                            ctx.store( f, 5 );
                        } );
                ankerl::nanobench::doNotOptimizeAway( a );
                ankerl::nanobench::doNotOptimizeAway( b );
                ankerl::nanobench::doNotOptimizeAway( c );
                ankerl::nanobench::doNotOptimizeAway( d );
                ankerl::nanobench::doNotOptimizeAway( e );
                ankerl::nanobench::doNotOptimizeAway( f );
            } );

    ankerl::nanobench::Bench().minEpochIterations( MINIT ).run(
            "Complex compute only store e,f",
            [&]() {
                vecex::compute(
                        std::array { a.data(),
                                     b.data(),
                                     c.data(),
                                     d.data(),
                                     e.data(),
                                     f.data() },
                        SIZE,
                        []( auto& ctx ) {
                            auto a = ctx.load( 0 );
                            auto b = ctx.load( 1 );
                            auto c = a + b;
                            auto d = b + c;
                            auto e = c * b;
                            auto f = d * b;
                            auto an = f - TYPE( 3 );
                            f = d - an;
                            ctx.store( e, 4 );
                            ctx.store( f, 5 );
                        } );
                ankerl::nanobench::doNotOptimizeAway( a );
                ankerl::nanobench::doNotOptimizeAway( b );
                ankerl::nanobench::doNotOptimizeAway( c );
                ankerl::nanobench::doNotOptimizeAway( d );
                ankerl::nanobench::doNotOptimizeAway( e );
                ankerl::nanobench::doNotOptimizeAway( f );
            } );
}

void
testing() {
    std::vector<TYPE> a;
    std::vector<TYPE> b;
    std::vector<TYPE> c;
    std::vector<TYPE> d;
    std::vector<TYPE> e;
    std::vector<TYPE> f;
    a.resize( SIZE );
    b.resize( SIZE );
    c.resize( SIZE );
    d.resize( SIZE );
    e.resize( SIZE );
    f.resize( SIZE );

    for ( int i = 0; i < SIZE; i++ ) {
        a[i] = i;
        b[i] = i;
    }

    vecex::compute(
            std::array { a.data(),
                         b.data(),
                         c.data(),
                         d.data(),
                         e.data(),
                         f.data() },
            SIZE,
            []( auto& ctx ) {
                auto a = ctx.load( 0 );
                auto b = ctx.load( 1 );
                auto c = a + b;
                ctx.store( c, 2 );
                auto d = b + c;
                ctx.store( d, 3 );
                auto e = c * b;
                ctx.store( e, 4 );
                auto f = d * b;
                ctx.store( f, 5 );
                auto an = f - 3.0f;
                ctx.store( an, 0 );
                ctx.store( 3.0f, 0 );
            } );
    for ( int i = 0; i < 10; i++ ) {
        std::cout << a[i] << " " << b[i] << " " << c[i] << " " << d[i] << " "
                  << e[i] << " " << f[i] << " " << std::endl;
    }
}

int
main() {
    bench1();
    bench_complex();
    // testing();
    return 0;
}
