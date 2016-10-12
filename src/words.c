#include "pforth.h"

#undef POP_WORD
#undef PUSH_WORD
#undef _MATH_OP
#undef _COMPARE_OP
#undef _TRUE_OP
#undef _EMIT

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

FORTH_TYPE _top() {
  FORTH_TYPE result;
  memcpy(&result, data_stack_top - sizeof(FORTH_TYPE), sizeof(FORTH_TYPE));
  return result;
}

FORTH_TYPE _depth() {
  return (data_stack_top - data_stack_bottom()) / sizeof(FORTH_TYPE);
}

/**
   Drops the size bytes data_stack.

   Memory is not freed, just #data_stack_top is moved

   \param size size of data in bytes
 */
void _drop(size_t size) {
  if (data_stack_top > data_stack_bottom())
    data_stack_top -= size;
  else
    DBG("The data stack is empty. %s\n", "DNIWE is here!");
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
  TYPE pop_ ## TYPE() {                                         \
    if (!(data_stack_top > data_stack_bottom())) {              \
      DBG("The data stack is empty. %s\n", "DNIWE is here!");   \
      return 0;                                                 \
    };                                                          \
    TYPE rv = 0;                                                \
    memcpy(&rv, data_stack_top - sizeof(TYPE), sizeof(TYPE));   \
    _drop(sizeof(TYPE));                                        \
    DBG("poping %d of %s\n", rv, #TYPE);                        \
    return rv;                                                  \
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
    TYPE b = pop_ ## TYPE();     \
    TYPE a = pop_ ## TYPE();     \
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
  _EMBED_DECORATE(void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) ());    \
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

#undef _TYPED_GENERIC_WORD
#undef _GENERIC_WORD

#define _TYPED_GENERIC_WORD(TYPE, NAME, BODY)        \
  void M_CONC(_, M_CONC(NAME, M_CONC(_, TYPE))) () { \
    BODY \
  }

#define _GENERIC_WORD(NAME, BODY) _TYPED_GENERIC_WORD(FORTH_TYPE, NAME, BODY)

#define _TYPED_POP_NUM(TYPE, NAME) \
  TYPE NAME = M_CONC(pop, M_CONC(_, TYPE))();

#define _POP_NUM(NAME) \
  _TYPED_POP_NUM(FORTH_TYPE, NAME)

#define _PUSH_NUM(value) \
  M_CONC(push_, FORTH_TYPE) (value);

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

#define _DEF_TYPE_OP(op) M_CONC(_, M_CONC(op, M_CONC(_, FORTH_TYPE)))

_GENERIC_WORD(emit,                    \
              _POP_NUM(number)         \
              char c = LO(number);     \
              DBG("EMIT %c\n", c);     \
              PRINT("%c", c))

_GENERIC_WORD(dot,                           \
              _POP_NUM(number)               \
              DBG("PRINT %d\n", number);     \
              PRINT("%d ", number))

_GENERIC_WORD(dup,                           \
              _POP_NUM(number)               \
              _PUSH_NUM(number)              \
              _PUSH_NUM(number)              \
              DBG("DUP %d\n", number))

_GENERIC_WORD(swap,                           \
              _POP_NUM(number1)               \
              _POP_NUM(number2)               \
              _PUSH_NUM(number1)             \
              _PUSH_NUM(number2)             \
              DBG("%d %d -> %d %d\n", number1, number2, number2, number1))

_GENERIC_WORD(rot,         \
              _POP_NUM(c)  \
              _POP_NUM(b)  \
              _POP_NUM(a)  \
              _PUSH_NUM(b) \
              _PUSH_NUM(c) \
              _PUSH_NUM(a) \
              DBG("%d %d %d -> %d %d %d\n",
                  a, b, c,
                  b, c, a))

_GENERIC_WORD(mrot,        \
              _POP_NUM(c)  \
              _POP_NUM(b)  \
              _POP_NUM(a)  \
              _PUSH_NUM(c) \
              _PUSH_NUM(a) \
              _PUSH_NUM(b) \
              DBG("%d %d %d -> %d %d %d\n",
                  a, b, c,
                  c, a, b))

_GENERIC_WORD(qdup,                           \
              if (_top()) {                   \
  _POP_NUM(number)               \
  _PUSH_NUM(number)              \
  _PUSH_NUM(number)              \
  DBG("?DUP %d\n", number)     \
})

_GENERIC_WORD(over,           \
              FORTH_TYPE val; \
              memcpy(&val,    \
                     data_stack_top - 2 * sizeof(FORTH_TYPE),  \
                     sizeof(FORTH_TYPE));                      \
              _PUSH_NUM(val)  \
              DBG("OVER %d\n", val))

_GENERIC_WORD(drop,                           \
              _drop(sizeof(FORTH_TYPE));      \
              DBG("%s\n", "DROP"))

_GENERIC_WORD(depth,                           \
              _PUSH_NUM(_depth())              \
              DBG("DEPTH: %d\n", _depth()))

_GENERIC_WORD(times_divide,        \
              FORTH_TYPE n3 = POP; \
              FORTH_TYPE n2 = POP; \
              FORTH_TYPE n1 = POP; \
              FORTH_TYPE r = n1 * n2; \
              _PUSH_NUM(r / n3);   \
              )

_GENERIC_WORD(times_divide_mod,    \
              FORTH_TYPE n3 = POP; \
              FORTH_TYPE n2 = POP; \
              FORTH_TYPE n1 = POP; \
              FORTH_TYPE r = n1 * n2; \
              _PUSH_NUM(r % n3);   \
              _PUSH_NUM(r / n3);   \
              )

_GENERIC_WORD(key,                 \
              _PUSH_NUM(fgetc(stdin)))

_GENERIC_WORD(max,                 \
              _POP_NUM(n2)         \
              _POP_NUM(n1)         \
              _PUSH_NUM(n1 > n2 ? n1 : n2))

_GENERIC_WORD(min,                 \
              _POP_NUM(n2)         \
              _POP_NUM(n1)         \
              _PUSH_NUM(n1 < n2 ? n1 : n2))

_GENERIC_WORD(pick,                \
              _POP_NUM(depth)      \
              FORTH_TYPE * ptr = (FORTH_TYPE *) data_stack_top - depth - 1; \
              if (ptr < (FORTH_TYPE *) data_stack_bottom())            \
                return;            \
              _PUSH_NUM(*ptr))

void _EMBED_DECORATE(get_variable_value()) {
  FORTH_TYPE* val;
  memcpy(&val, data_stack_top - sizeof(POINTER_TYPE), sizeof(POINTER_TYPE));
  _drop(sizeof(POINTER_TYPE));
  _PUSH_NUM(*val);
}

void _EMBED_DECORATE(set_variable_value()) {
  POINTER_TYPE addr;
  memcpy(&addr, data_stack_top - sizeof(POINTER_TYPE), sizeof(POINTER_TYPE));
  _drop(sizeof(POINTER_TYPE));
  _POP_NUM(val);
  memcpy((void *) addr, &val, sizeof(FORTH_TYPE));
}

void register_precompiled() {
#include "generators_run.h"
  register_native("EMIT",  &_DEF_TYPE_OP(emit));
  register_native(".",	   &_DEF_TYPE_OP(dot));
  register_native("DUP",   &_DEF_TYPE_OP(dup));
  register_native("?DUP",  &_DEF_TYPE_OP(qdup));
  register_native("SWAP",  &_DEF_TYPE_OP(swap));
  register_native("ROT",   &_DEF_TYPE_OP(rot));
  register_native("-ROT",  &_DEF_TYPE_OP(mrot));
  register_native("OVER",  &_DEF_TYPE_OP(over));
  register_native("DROP",  &_DEF_TYPE_OP(drop));
  register_native("DEPTH", &_DEF_TYPE_OP(depth));
  register_native("*/",	   &_DEF_TYPE_OP(times_divide));
  register_native("*/MOD", &_DEF_TYPE_OP(times_divide_mod));
  register_native("KEY",   &_DEF_TYPE_OP(key));
  register_native("MAX",   &_DEF_TYPE_OP(max));
  register_native("MIN",   &_DEF_TYPE_OP(min));
  register_native("PICK",  &_DEF_TYPE_OP(pick));
  register_native("@",	   &get_variable_value);
  register_native("!",	   &set_variable_value);

#include "core_fs.h"
  preprocess((char *) core_compressed_fs);
  eval(forth_dict, (char *) core_compressed_fs, NULL);
}

pforth_word_ptr create_variable(const char* name) {
  pforth_word_ptr var = dict_set(forth_dict, name, NULL);
  var->text_code = malloc(2);
  var->text_code[0] = 0x01;
  DBG("Created variable %s @%p", name, (void *) &var->location);
  return var;
}
