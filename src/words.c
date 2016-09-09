#include "pforth.h"

#undef POP_WORD
#undef PUSH_WORD
#undef _MATH_OP
#undef _COMPARE_OP
#undef _TRUE_OP

/**
   Push the data to global data stack and move the #data_stack_top forward

   The data must be valid pointer with at least size bytes inside

   \param data data to push
   \param size size of data in bytes
 */
void _push(void* data, size_t size) {
  memcpy(data_stack_top, data, size);
  data_stack_top += size;
}

/**
   Drops the size bytes data_stack.

   Memory is not freed, just #data_stack_top is moved

   \param size size of data in bytes
 */
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
  TYPE pop_ ## TYPE() {                                       \
    TYPE rv = 0;                                              \
    memcpy(&rv, data_stack_top - sizeof(TYPE), sizeof(TYPE)); \
    _drop(sizeof(TYPE));                                      \
    DBG("poping %d of %s\n", rv, #TYPE); \
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
  void push_ ## TYPE(TYPE value) {                \
    DBG("pushing %d of %s\n", value, #TYPE); \
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
  void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) () {                \
    TYPE a = pop_ ## TYPE();     \
    TYPE b = pop_ ## TYPE();     \
    DBG("MATH_OP %d %s %d\n", a, #OP, b); \
    push_ ## TYPE(a OP b);       \
  }

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
    DBG("COMP_OP %d %s %d\n", a, #OP, b); \
    push_ ## TYPE(a OP b ? -1 : 0); \
  }

#define _TRUE_OP(TYPE) \
  int M_CONC(_, M_CONC(true, M_CONC(_, TYPE))) () {                \
    return pop_ ## TYPE() == -1 ? 1 : 0;                           \
  }

#include "generators_run.h"

#undef _MATH_OP
#define _MATH_OP(TYPE, NAME, OP)    \
  register_native(#OP, &M_CONC(_, M_CONC(NAME, M_CONC(_, FORTH_TYPE))));

#undef _COMPARE_OP
#define _COMPARE_OP(TYPE, NAME, OP) \
  register_native(#OP, &M_CONC(_, M_CONC(NAME, M_CONC(_, FORTH_TYPE))));

#undef POP_WORD
#define POP_WORD(TYPE)

#undef PUSH_WORD
#define PUSH_WORD(TYPE)

#undef _TRUE_OP
#define _TRUE_OP(TYPE)

void register_precompiled() {
   #include "generators_run.h"
}
