/* ###################################################################

MIT License

Copyright (c) 2023 TRP - Thimo Pischel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

######################################################################

HOW TO USE:

* Agnar's vectorclass Lib
this Project requires agnar's vectorclass simd lib file from:
https://github.com/vectorclass/version2.git
if needed change the #include "vectorclass.h" to the correct one (target is the
vectorclass.h from agnar)

* PREDFINE VECEX_OVERRIDE
required to activate the override for std::vector<number> operator overloading
( std::vector<float> a , b;  auto c = a + b; )

* Functions for your use
just hide the namespace internal. everything is left is safe to use

-> vecex::helper::element_count_min(const std::vector<T>& ...)
    handy to calculate fast the amount of elements that are safe to calculate:
        a.resize(25);  }
        b.resize(26);  } => element_count_min(a,b,c) => 20
        c.resize(20);  }

-> add / sub / mul / div - (_in)
    Arg1 -> std::vector
    Arg2 -> std::vector or number
    Arg3 -> only with _in, std::vector for the result
    return -> only without _in, a new std::vector with the result

-> compute
    Arg1 -> std::array with pointer to the first element of each collection
            .data() from a vector
    Arg2 -> element_count (the element count needs to be smallest of all,
            otherwise it will access outside the range.
            Tipp: use vecex::helper::element_count_min( [all_your_vecs]&...) )
    Arg3 -> lambda& for the computation []( auto& ctx ){ your_computation }.
            For more details read the next Paragraph.


    computation in lambda:

        * Parameter
        The parameter needs to be an auto& and is the context of the
        calculation. The auto is required, because the unrolling of the SIMD
        will generate different type of contexts.

        * Getting the value >> ctx.load(index_in_data_set) <<
        The context allows you to load the current value. The function requires
        the Index in the data_set you set in vecex::comput(data_set,
        element_count, lambda).

        * Storing the value >> ctx.store(value, index_in_data_set) <<
        !!! Remeber without storing the data_set will stay untouched !!!
        The context allows you to store values in the data_set you want.


    current supported Value Operations:

        Operation   |   supported types
        ------------|--------------------------------------
        +, -, *     | all types
        /           | float, double || WIP: INTEGER
                    |
        .pow(x)     | WIP
        .sin()      |
        .cos()      |
        .tan()      |

    Example:

        std::vector<float> a, b, c;
        a.resize(100);
        b.resize(100);
        c.resize(100);
        d.resize(100);
        //... fill a and b;

        vecex::compute(
            std::array {a.data(), b.data(), c.data(), c.data()},
            vecex::helper::element_count_min(a,b,c,d),
            []( auto& ctx ) {
                //loading from a.data()
                auto a = ctx.load(0);
                //loading from b.data()
                auto b = ctx.load(1);

                // simple caclulation
                auto a_b = a + b;
                // storing the result in c.data()
                ctx.store( a_b, 2 );

                // simple calculation but overriding in a (local)
                // NOT storing in a.data()
                a = a_b + 5;
                // storing in d.data()
                ctx.store( a, 3);
            });




###################################################################### */

#pragma once

// #define VECEX_OVERRIDE

#include "vectorclass.h"
#include <vector>
#include <limits>

namespace vecex {
//
// for libusers completly irrelevant. just hide internal and use the
// functionality below internal
//
namespace internal {
namespace translation_types {

// type
template<class T, size_t SIZE>
struct simd_vec_type {};

template<class T, size_t SIZE>
using simd_vec_type_t = typename simd_vec_type<T, SIZE>::type;

template<class T>
struct simd_vec_sizes {
    static const size_t min = 99999999;
    static const size_t max = 0;
};

template<class T, size_t size>
struct simd_vec_size_is_in_lower_bound {
    static const bool value = simd_vec_sizes<T>::min <= size;
};

// Floating point
template<>
struct simd_vec_type<float, 4> {
    typedef Vec4f type;
};
template<>
struct simd_vec_type<float, 8> {
    typedef Vec8f type;
};
template<>
struct simd_vec_type<float, 16> {
    typedef Vec16f type;
};
template<>
struct simd_vec_sizes<float> {
    static const size_t min = 4;
    static const size_t max = 16;
};

// Double
template<>
struct simd_vec_type<double, 4> {
    typedef Vec4d type;
};
template<>
struct simd_vec_type<double, 8> {
    typedef Vec8d type;
};
template<>
struct simd_vec_sizes<double> {
    static const size_t min = 4;
    static const size_t max = 8;
};

// long
template<>
struct simd_vec_type<long, 4> {
    typedef Vec4q type;
};
template<>
struct simd_vec_type<long, 8> {
    typedef Vec8q type;
};
template<>
struct simd_vec_sizes<long> {
    static const size_t min = 4;
    static const size_t max = 8;
};

// unsigned long
template<>
struct simd_vec_type<unsigned long, 4> {
    typedef Vec4q type;
};
template<>
struct simd_vec_type<unsigned long, 8> {
    typedef Vec8q type;
};
template<>
struct simd_vec_sizes<unsigned long> {
    static const size_t min = 4;
    static const size_t max = 8;
};

// int
template<>
struct simd_vec_type<int, 4> {
    typedef Vec4i type;
};
template<>
struct simd_vec_type<int, 8> {
    typedef Vec8i type;
};
template<>
struct simd_vec_type<int, 16> {
    typedef Vec16i type;
};
template<>
struct simd_vec_sizes<int> {
    static const size_t min = 4;
    static const size_t max = 16;
};

// unsigned int
template<>
struct simd_vec_type<unsigned int, 4> {
    typedef Vec4ui type;
};
template<>
struct simd_vec_type<unsigned int, 8> {
    typedef Vec8ui type;
};
template<>
struct simd_vec_type<unsigned int, 16> {
    typedef Vec16ui type;
};
template<>
struct simd_vec_sizes<unsigned int> {
    static const size_t min = 4;
    static const size_t max = 16;
};

// short
template<>
struct simd_vec_type<short, 8> {
    typedef Vec8s type;
};
template<>
struct simd_vec_type<short, 16> {
    typedef Vec16s type;
};
template<>
struct simd_vec_type<short, 32> {
    typedef Vec32s type;
};
template<>
struct simd_vec_sizes<short> {
    static const size_t min = 8;
    static const size_t max = 32;
};

// unsigned short
template<>
struct simd_vec_type<unsigned short, 8> {
    typedef Vec8us type;
};
template<>
struct simd_vec_type<unsigned short, 16> {
    typedef Vec16us type;
};
template<>
struct simd_vec_type<unsigned short, 32> {
    typedef Vec32us type;
};
template<>
struct simd_vec_sizes<unsigned short> {
    static const size_t min = 8;
    static const size_t max = 32;
};

// short
template<>
struct simd_vec_type<char, 16> {
    typedef Vec16c type;
};
template<>
struct simd_vec_type<char, 32> {
    typedef Vec32c type;
};
template<>
struct simd_vec_type<char, 64> {
    typedef Vec64c type;
};
template<>
struct simd_vec_sizes<char> {
    static const size_t min = 16;
    static const size_t max = 64;
};

// unsigned short
template<>
struct simd_vec_type<unsigned char, 16> {
    typedef Vec16uc type;
};
template<>
struct simd_vec_type<unsigned char, 32> {
    typedef Vec32uc type;
};
template<>
struct simd_vec_type<unsigned char, 64> {
    typedef Vec64uc type;
};
template<>
struct simd_vec_sizes<unsigned char> {
    static const size_t min = 16;
    static const size_t max = 64;
};

};    // namespace translation_types

namespace compute {

template<typename CalcType, size_t unroll_size, bool simd>
struct Value {
    typedef Value<CalcType, unroll_size, simd> _Value;
    typedef translation_types::simd_vec_type_t<CalcType, unroll_size>
               _SIMD_Type;
    _SIMD_Type value;

    static inline _Value
    from_number( const CalcType& number ) {
        return { _SIMD_Type( number ) };
    }

    static inline _Value
    load( const CalcType* ptr ) {
        _Value result;
        result.value.load( ptr );
        return result;
    }
    inline void
    store( CalcType* ptr ) const {
        this->value.store( ptr );
    }

    // with possible simd
    inline _Value
    operator+( const _Value& rh ) {
        return _Value { value = this->value + rh.value };
    }
    inline _Value
    operator-( const _Value& rh ) {
        return _Value { value = this->value - rh.value };
    }
    inline _Value
    operator*( const _Value& rh ) {
        return _Value { value = this->value * rh.value };
    }
    inline _Value
    operator/( const _Value& rh ) {
        return _Value { value = this->value / rh.value };
    }

    // with skalar value
    inline _Value
    operator+( const CalcType& rh ) {
        return _Value { value = this->value + rh };
    }
    inline _Value
    operator-( const CalcType& rh ) {
        return _Value { value = this->value - rh };
    }
    inline _Value
    operator*( const CalcType& rh ) {
        return _Value { value = this->value * rh };
    }
    inline _Value
    operator/( const CalcType& rh ) {
        return _Value { value = this->value / rh };
    }
};

template<typename CalcType, size_t unroll_size>
struct Value<CalcType, unroll_size, false> {
    typedef Value<CalcType, unroll_size, false> _Value;
    CalcType                                    value;

    static inline _Value
    from_number( const CalcType& number ) {
        return { number };
    }

    static inline _Value
    load( const CalcType* ptr ) {
        _Value result;
        result.value = *( ptr );
        return result;
    }
    inline void
    store( CalcType* ptr ) const {
        *( ptr ) = this->value;
    }

    // with possible simd
    inline _Value
    operator+( const _Value& rh ) {
        return _Value { value = this->value + rh.value };
    }
    inline _Value
    operator-( const _Value& rh ) {
        return _Value { value = this->value - rh.value };
    }
    inline _Value
    operator*( const _Value& rh ) {
        return _Value { value = this->value * rh.value };
    }
    inline _Value
    operator/( const _Value& rh ) {
        return _Value { value = this->value / rh.value };
    }

    // with skalar
    inline _Value
    operator+( const CalcType& rh ) {
        return _Value { value = this->value + rh };
    }
    inline _Value
    operator-( const CalcType& rh ) {
        return _Value { value = this->value - rh };
    }
    inline _Value
    operator*( const CalcType& rh ) {
        return _Value { value = this->value * rh };
    }
    inline _Value
    operator/( const CalcType& rh ) {
        return _Value { value = this->value / rh };
    }
};

template<class CalcType, size_t extern_size>
struct State {
    const std::array<CalcType*, extern_size> data_sets;
    size_t                                   offset;
    const size_t                             element_count;
};

template<class CalcType, size_t extern_size, size_t unroll_size>
struct Context {
    typedef Value<
            CalcType,
            unroll_size,
            translation_types::simd_vec_size_is_in_lower_bound<
                    CalcType,
                    unroll_size>::value>
            _Value;

    State<CalcType, extern_size> const* state;

    inline _Value
    load( const size_t index ) {
        return _Value::load(
                (CalcType*)( (size_t)( this->state->data_sets[index] )
                             + this->state->offset ) );
    };

    inline void
    store( _Value& to_store, const size_t index ) {
        to_store.store( (CalcType*)( (size_t)( this->state->data_sets[index] )
                                     + this->state->offset ) );
    }

    inline void
    store( const CalcType& to_store, const size_t index ) {
        auto tmp = _Value::from_number( to_store );
        store( tmp, index );
    }

    Context( State<CalcType, extern_size>* state ) {
        this->state = state;
    }
};

template<class CalcType, size_t extern_size, size_t unroll_size, bool>
struct unroll_operation {
    template<class Function>
    static inline void
    f( State<CalcType, extern_size>& state, Function& func ) {
        Context<CalcType, extern_size, unroll_size> ctx( &state );
        const size_t BLOCK_SIZE = unroll_size * sizeof( CalcType );
        const size_t MAX_OFFSET
                = state.element_count * sizeof( CalcType ) - BLOCK_SIZE;

        for ( ; state.offset <= MAX_OFFSET; state.offset += BLOCK_SIZE ) {
            func( ctx );
        }

        unroll_operation<
                CalcType,
                extern_size,
                unroll_size / 2,
                translation_types::simd_vec_size_is_in_lower_bound<
                        CalcType,
                        unroll_size / 2>::value>::f( state, func );
    }
};

template<class CalcType, size_t extern_size, size_t unroll_size>
struct unroll_operation<CalcType, extern_size, unroll_size, false> {
    template<class Function>
    static inline void
    f( State<CalcType, extern_size>& state, Function& func ) {
        Context<CalcType, extern_size, unroll_size> ctx( &state );

        const size_t max_offset = state.element_count * sizeof( CalcType );

        for ( ; state.offset < max_offset;
              state.offset += sizeof( CalcType ) ) {
            func( ctx );
        }
    }
};

template<class CalcType, size_t external_size, class Function>
void
run( std::array<CalcType*, external_size> data_sets,
     const size_t                         element_count,
     Function                             func ) {
    State<CalcType, external_size> state { .data_sets = data_sets,
                                           .offset = 0,
                                           .element_count = element_count };

    unroll_operation<
            CalcType,
            external_size,
            translation_types::simd_vec_sizes<CalcType>::max,
            translation_types::simd_vec_size_is_in_lower_bound<
                    CalcType,
                    translation_types::simd_vec_sizes<CalcType>::max>::value>::
            f( state, func );
}

};    // namespace compute

};    // namespace internal

namespace helper {

template<class head>
size_t
element_count_min( const std::vector<head>& h ) {
    return h.size();
}

template<class head, class... tail>
size_t
element_count_min( const std::vector<head>& h, const tail&... t ) {
    const size_t min = element_count_min( t... );
    const size_t current_size = h.size();
    return min < current_size ? min : current_size;
}
}    // namespace helper

template<class CalcType, size_t external_size, class Function>
void
compute( std::array<CalcType*, external_size> data_sets,
         const size_t                         element_count,
         Function                             func ) {
    internal::compute::State<CalcType, external_size> state {
        .data_sets = data_sets,
        .offset = 0,
        .element_count = element_count
    };

    internal::compute::unroll_operation<
            CalcType,
            external_size,
            internal::translation_types::simd_vec_sizes<CalcType>::max,
            internal::translation_types::simd_vec_size_is_in_lower_bound<
                    CalcType,
                    internal::translation_types::simd_vec_sizes<
                            CalcType>::max>::value>::f( state, func );
}

template<class CalcType>
std::vector<CalcType>
add( std::vector<CalcType>& a, std::vector<CalcType>& b ) {
    const size_t          element_count = helper::element_count_min( a, b );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i + b_i;
                ctx.store( result_i, 2 );
            } );
    return result;
}

template<class CalcType>
std::vector<CalcType>
add( std::vector<CalcType>& a, const CalcType& b ) {
    const size_t          element_count = helper::element_count_min( a );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [b]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i + b;
                ctx.store( result_i, 1 );
            } );
    return result;
}

template<class CalcType>
void
add_in( std::vector<CalcType>& a,
        std::vector<CalcType>& b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, b, result );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i + b_i;
                ctx.store( result_i, 2 );
            } );
}

template<class CalcType>
void
add_in( std::vector<CalcType>& a,
        const CalcType&        b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, result );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [&]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i + b;
                ctx.store( result_i, 1 );
            } );
}

template<class CalcType>
std::vector<CalcType>
sub( std::vector<CalcType>& a, std::vector<CalcType>& b ) {
    const size_t          element_count = helper::element_count_min( a, b );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i - b_i;
                ctx.store( result_i, 2 );
            } );
    return result;
}

template<class CalcType>
std::vector<CalcType>
sub( std::vector<CalcType>& a, const CalcType& b ) {
    const size_t          element_count = helper::element_count_min( a );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [b]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i - b;
                ctx.store( result_i, 1 );
            } );
    return result;
}

template<class CalcType>
void
sub_in( std::vector<CalcType>& a,
        std::vector<CalcType>& b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, b, result );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i - b_i;
                ctx.store( result_i, 2 );
            } );
}

template<class CalcType>
void
sub_in( std::vector<CalcType>& a,
        const CalcType&        b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, result );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [&]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i - b;
                ctx.store( result_i, 1 );
            } );
}

template<class CalcType>
std::vector<CalcType>
mul( std::vector<CalcType>& a, std::vector<CalcType>& b ) {
    const size_t          element_count = helper::element_count_min( a, b );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i * b_i;
                ctx.store( result_i, 2 );
            } );
    return result;
}

template<class CalcType>
std::vector<CalcType>
mul( std::vector<CalcType>& a, const CalcType& b ) {
    const size_t          element_count = helper::element_count_min( a );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [b]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i * b;
                ctx.store( result_i, 1 );
            } );
    return result;
}

template<class CalcType>
void
mul_in( std::vector<CalcType>& a,
        std::vector<CalcType>& b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, b, result );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i * b_i;
                ctx.store( result_i, 2 );
            } );
}

template<class CalcType>
void
mul_in( std::vector<CalcType>& a,
        const CalcType&        b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, result );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [&]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i * b;
                ctx.store( result_i, 1 );
            } );
}

template<class CalcType>
std::vector<CalcType>
div( std::vector<CalcType>& a, std::vector<CalcType>& b ) {
    const size_t          element_count = helper::element_count_min( a, b );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i / b_i;
                ctx.store( result_i, 2 );
            } );
    return result;
}

template<class CalcType>
std::vector<CalcType>
div( std::vector<CalcType>& a, const CalcType& b ) {
    const size_t          element_count = helper::element_count_min( a );
    std::vector<CalcType> result( element_count );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [b]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i / b;
                ctx.store( result_i, 1 );
            } );
    return result;
}

template<class CalcType>
void
div_in( std::vector<CalcType>& a,
        std::vector<CalcType>& b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, b, result );
    internal::compute::run(
            std::array { a.data(), b.data(), result.data() },
            element_count,
            []( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto b_i = ctx.load( 1 );
                auto result_i = a_i / b_i;
                ctx.store( result_i, 2 );
            } );
}

template<class CalcType>
void
div_in( std::vector<CalcType>& a,
        const CalcType&        b,
        std::vector<CalcType>& result ) {
    const size_t element_count = helper::element_count_min( a, result );
    internal::compute::run(
            std::array { a.data(), result.data() },
            element_count,
            [&]( auto& ctx ) {
                auto a_i = ctx.load( 0 );
                auto result_i = a_i / b;
                ctx.store( result_i, 1 );
            } );
}

}    // namespace vecex

#ifdef VECEX_OVERRIDE
template<class CalcType>
std::vector<CalcType>
operator+( std::vector<CalcType>& lhs, std::vector<CalcType>& rhs ) {
    return vecex::add( lhs, rhs );
}

template<class CalcType>
std::vector<CalcType>
operator+( std::vector<CalcType>& lhs, const CalcType& rhs ) {
    return vecex::add( lhs, rhs );
}

template<class CalcType>
std::vector<CalcType>
operator-( std::vector<CalcType>& lhs, std::vector<CalcType>& rhs ) {
    return vecex::sub( lhs, rhs );
}

template<class CalcType>
std::vector<CalcType>
operator-( std::vector<CalcType>& lhs, const CalcType& rhs ) {
    return vecex::sub( lhs, rhs );
}

template<class CalcType>
std::vector<CalcType>
operator*( std::vector<CalcType>& lhs, std::vector<CalcType>& rhs ) {
    return vecex::mul( lhs, rhs );
}

template<class CalcType>
std::vector<CalcType>
operator*( std::vector<CalcType>& lhs, const CalcType& rhs ) {
    return vecex::mul( lhs, rhs );
}

template<class CalcType>
std::vector<CalcType>
operator/( std::vector<CalcType>& lhs, std::vector<CalcType>& rhs ) {
    return vecex::div( lhs, rhs );
}

template<class CalcType>
std::vector<CalcType>
operator/( std::vector<CalcType>& lhs, const CalcType& rhs ) {
    return vecex::div( lhs, rhs );
}
#endif
