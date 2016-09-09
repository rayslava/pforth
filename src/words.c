#include "pforth.h"

/**
   Decorating operations with attributes. It will work properly for target
   embeddable platform only anyway.
 */
#ifdef EMBED_BUILD
#define _PRE_DEC
#define _POST_DEC __attribute__((naked))
#define _EMBED_DECORATE(FUNC) _PRE_DEC FUNC _POST_DEC;
#else
#define _PRE_DEC
#define _POST_DEC
#define _EMBED_DECORATE(FUNC)
#endif

/**
   Push the data to global data stack and move the #data_stack_top forward

   The data must be valid pointer with at least size bytes inside

   \param data data to push
   \param size size of data in bytes
 */
_EMBED_DECORATE(void _push(void* data, size_t size))
void _push(void* data, size_t size) {
  memcpy(data_stack_top, data, size);
  data_stack_top += size;
}

/**
   Drops the size bytes data_stack.

   Memory is not freed, just #data_stack_top is moved

   \param size size of data in bytes
 */
_EMBED_DECORATE(void _drop(size_t size))
void _drop(size_t size) {
  data_stack_top -= size;
}

/**
   Generator of the stack pop functions for desired TYPE

   Creates function named pop_TYPE with retval of TYPE.
   The result for int32_t will be
   \code
   int32_t pop_int32_t();
   \endcode

   The resulting function will use #_drop for moving the #data_stack_top

   \param TYPE the retval type for generated function
 */
#define POP_WORD(TYPE) \
  _EMBED_DECORATE(TYPE pop_ ## TYPE())                        \
  TYPE pop_ ## TYPE() {                                       \
    TYPE rv = 0;                                              \
    memcpy(&rv, data_stack_top - sizeof(TYPE), sizeof(TYPE)); \
    _drop(sizeof(TYPE));                                      \
    return rv;                                                \
  }

/**
   Generator of the stack push functions for desired TYPE

   Creates function named push_TYPE with argument of TYPE
   The result for int32_t will be
   \code
   void push_int32_t (int32_t);
   \endcode

   The resulting function will use #_push for data placement

   \param TYPE the retval type for generated function
 */
#define PUSH_WORD(TYPE) \
  _EMBED_DECORATE(void push_ ## TYPE(TYPE value)) \
  void push_ ## TYPE(TYPE value) {                \
    _push(&value, sizeof(TYPE));                  \
  }

/**
   Generator for native math words

   Creates function named NAME for operation OP with arguments of TYPE
   The result for \p int32_t, \p add, \p + will be
   \code
   void _add_int32_t() {
     int32_t a = pop_int32();
     int32_t b = pop_int32();
     push_int32_t (a >= b ? -1 : 0);
   }
   \endcode

   The resulting function will use push_TYPE for data placement

   \param TYPE data type for generated function
   \param NAME resulting function name prefix
   \param OP operation to compile into function
 */
#define _MATH_OP(TYPE, NAME, OP) \
  _EMBED_DECORATE(void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) ()) \
  void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) () {                \
    TYPE a = pop_ ## TYPE();     \
    TYPE b = pop_ ## TYPE();     \
    push_ ## TYPE(a OP b);       \
  }

#define _FIVE_MATH_OPS(TYPE) \
  _MATH_OP(TYPE, add, +)     \
  _MATH_OP(TYPE, sub, -)     \
  _MATH_OP(TYPE, mul, *)     \
  _MATH_OP(TYPE, div, /)     \
  _MATH_OP(TYPE, mod, %)

/**
   Generator for comparison words

   Creates function named NAME for operation OP with arguments of TYPE
   The result for \p int32_t, \p ge, \p >= will be
   \code
   void _ge_int32_t() {
     int32_t a = pop_int32();
     int32_t b = pop_int32();
     push_int32_t (a >= b ? -1 : 0);
   }
   \endcode

   The resulting function will use push_TYPE for data placement

   \param TYPE data type for generated function
   \param NAME resulting function name prefix
   \param OP operation to compile into function
 */
#define _COMPARE_OP(TYPE, NAME, OP) \
  _EMBED_DECORATE(void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) ())     \
  void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) () {                    \
    TYPE b = pop_ ## TYPE();        \
    TYPE a = pop_ ## TYPE();        \
    push_ ## TYPE(a OP b ? -1 : 0); \
  }

#define _COMPARISON_OPS(TYPE) \
  _COMPARE_OP(TYPE, gt, >)    \
  _COMPARE_OP(TYPE, lt, <)    \
  _COMPARE_OP(TYPE, ge, >=)   \
  _COMPARE_OP(TYPE, le, <=)   \
  _COMPARE_OP(TYPE, eq, ==)

#define _TRUE_OP(TYPE) \
  _EMBED_DECORATE(int M_CONC(_, M_CONC(true, M_CONC(_, TYPE))) ()) \
  int M_CONC(_, M_CONC(true, M_CONC(_, TYPE))) () {                \
    return pop_ ## TYPE() == -1 ? 1 : 0;                           \
  }

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

#define FORTH_TYPE int32_t

#define INT_TYPE_LIST FORTH_TYPE

MAP(PUSH_POP_WORDS,  INT_TYPE_LIST)
MAP(_FIVE_MATH_OPS,  INT_TYPE_LIST)
MAP(_COMPARISON_OPS, INT_TYPE_LIST)
MAP(_TRUE_OP,	     INT_TYPE_LIST)

#define TRUE M_CONC(_true_, FORTH_TYPE) ()
