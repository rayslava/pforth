#include "pforth.h"
#pragma once

/**
   Decorating operations with attributes. It will work properly for target
   embeddable platform only anyway.
 */
#ifdef EMBED_BUILD
#define _PRE_DEC
#define _POST_DEC __attribute__((naked))
#else
#define _PRE_DEC
#define _POST_DEC
#endif
#define _EMBED_DECORATE(FUNC) _PRE_DEC FUNC _POST_DEC

FORTH_TYPE _depth();

void _push(void* data, size_t size);

#define _PRECOMPILED_WORDS

_EMBED_DECORATE(void _push(void* data, size_t size));

_EMBED_DECORATE(void _drop(size_t size));

#define POP_WORD(TYPE) \
  _EMBED_DECORATE(TYPE pop_ ## TYPE());

#define PUSH_WORD(TYPE) \
  _EMBED_DECORATE(void push_ ## TYPE(TYPE value));

#define _MATH_OP(TYPE, NAME, OP) \
  _EMBED_DECORATE(void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) ());

#define _FIVE_MATH_OPS(TYPE) \
  _MATH_OP(TYPE, add, +)     \
  _MATH_OP(TYPE, sub, -)     \
  _MATH_OP(TYPE, mul, *)     \
  _MATH_OP(TYPE, div, /)     \
  _MATH_OP(TYPE, mod, %)

#define _COMPARE_OP(TYPE, NAME, OP) \
  _EMBED_DECORATE(void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) ());

#define _COMPARISON_OPS(TYPE) \
  _COMPARE_OP(TYPE, gt,	  >)   \
  _COMPARE_OP(TYPE, lt,	  <)   \
  _COMPARE_OP(TYPE, ge,	 >=)   \
  _COMPARE_OP(TYPE, le,	 <=)   \
  _COMPARE_OP(TYPE, eq,	 ==)   \
  _COMPARE_OP(TYPE, neq, !=)

#define _TRUE_OP(TYPE) \
  _EMBED_DECORATE(int M_CONC(_, M_CONC(true, M_CONC(_, TYPE))) ());

#define PUSH_POP_WORD(TYPE) PUSH_WORD(TYPE) POP_WORD(TYPE)
#define PUSH_POP_WORDS(...) PUSH_POP_WORD(__VA_ARGS__)

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define  EVAL(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define MAP_END(...)
#define MAP_OUT
#define MAP_GET_END() 0, MAP_END
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next) MAP_NEXT1(MAP_GET_END test, next)
#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1) (f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0) (f, peek, __VA_ARGS__)
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, (), 0))

#define M_CONC(A, B) M_CONC_(A, B)
#define M_CONC_(A, B) A ## B

#define _TYPED_GENERIC_WORD(TYPE, NAME) \
  _EMBED_DECORATE(void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) ());

#define _GENERIC_WORD(NAME) _TYPED_GENERIC_WORD(FORTH_TYPE, NAME)

#define POP M_CONC(pop_, FORTH_TYPE) ();

_GENERIC_WORD(emit)

_GENERIC_WORD(dot)

_GENERIC_WORD(dup)

_GENERIC_WORD(swap)

_GENERIC_WORD(rot)

_GENERIC_WORD(over)

_GENERIC_WORD(dot)

_GENERIC_WORD(depth)

_GENERIC_WORD(times_divide)

_GENERIC_WORD(times_divide_mod)

_GENERIC_WORD(key)

_GENERIC_WORD(max)

_GENERIC_WORD(min)

_GENERIC_WORD(pick)

#include "generators_run.h"

void register_precompiled();

pforth_word_ptr create_variable(const char* name);

/* Variable management */
