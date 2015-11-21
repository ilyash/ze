// NGS objects
#ifndef OBJ_H
#define OBJ_H
#include <stdint.h>
#include <stddef.h>

typedef uint16_t GLOBAL_VAR_INDEX;
#define GLOBAL_VAR_INDEX_FMT "%d"
typedef uint8_t LOCAL_VAR_INDEX;
#define MAX_GLOBALS         (65535)
#define MAX_LOCALS            (255)
#define INITITAL_ARRAY_SIZE     (8)

// On problems with `uintptr_t` change here according to Ruby source in `include/ruby/ruby.h`
// uintptr_t format for printf - PRIXPTR - printf("Blah %" PRIXPTR "\n", VALUE.num);
typedef union value_union {
	uintptr_t num;
	void * ptr;
} VALUE;

typedef struct object_struct {
	VALUE type;
	VALUE val;
} OBJECT;

typedef struct var_len_object_struct {
	OBJECT base;
	size_t len;
	size_t allocated;
	size_t item_size;
} VAR_LEN_OBJECT;

typedef struct closure {
	OBJECT base;
	size_t ip;
	VALUE **upvars;
	int upvars_levels; // needed?
	LOCAL_VAR_INDEX n_local_vars; // number of local variables including arguments
	LOCAL_VAR_INDEX n_params_required;
	LOCAL_VAR_INDEX n_params_optional;
	VALUE *params;
} CLOSURE_OBJECT;

// malloc() / NGS_MALLOC() memory is 8 bytes aligned, should be at least 4 bytes aligned
// .....000 - *OBJECT
// .....001 - int number
// OLD:
// .....000 - *OBJECT
// .....001 - int number
// .....010 - false
// .....011 - true
// .....100 - null
// .....101 - built_in_type (WIP)
// .....110 - reserved
// .....111 - undef
// NEW:
// ...... 00 - *OBJECT
// ...... 01 - int number
// XXXXXX 10 - misc values
// ...... 11 - reserved
// (scalar values)
// 000000 10 - null
// 000001 10 - undefined (aka undef)
// 00001X 10 - boolean
// 000010 10 - false
// 000011 10 - true
// (types)
// 000100 10 - Null
// 000101 10 - Bool
// 000110 10 - Int
// 000111 10 - Str
// 001000 10 - Arr
// 001001 10 - Fun
// 001010 10 - Any
// 001011 10 - Seq
// 001100 10 - Type


#define TAG_BITS    (2)
#define TAG_AND     (3)
#define TAG_INT     (1)
#define TYPE_AND (0xFF)

enum IMMEDIATE_VALUES {
	V_NULL  = 2,
	V_UNDEF = 6,
	V_FALSE = 10,
	V_TRUE  = 14,
	T_NULL  = 18,
	T_BOOL  = 22,
	T_INT   = 26,
	T_STR   = 30,
	T_ARR   = 34,
	T_FUN   = 38,
	T_ANY   = 42,
	T_SEQ   = 46,
	T_TYPE  = 50,
};

// object
// .....000 - *TYPE
// .....001 - String
// .....010 - Native method
// .....011 - NGS method
// .....100 - Array
// .....101 - Hash

// TODO: handle situation when n is wider than size_t - TAG_BITS bits
#define IS_NULL(v)      (v.num == V_NULL)
#define IS_TRUE(v)      (v.num == V_TRUE)
#define IS_FALSE(v)     (v.num == V_FALSE)
#define IS_UNDEF(v)     (v.num == V_UNDEF)
#define IS_NOT_UNDEF(v) (v.num != V_UNDEF)
// Boolean 00001X10
#define IS_BOOL(v)      ((v.num & 0xFB) == 10)
#define IS_INT(v)       ((v.num & TAG_AND) == TAG_INT)

#define SET_INT(v,n)    (v).num = ((n) << TAG_BITS) | TAG_INT
#define MAKE_INT(n)     ((VALUE){.num=((n) << TAG_BITS) | TAG_INT})
#define MAKE_BOOL(b)    ((VALUE){.num=(b ? V_TRUE : V_FALSE)})
#define MAKE_OBJ(o)     ((VALUE){.ptr=(o)})
#define GET_INT(v)      ((v).num >> TAG_BITS)
#define SET_OBJ(v,o)    (v).ptr = o
#define SET_NULL(v)     (v).num = V_NULL
#define SET_FALSE(v)    (v).num = V_FALSE
#define SET_TRUE(v)     (v).num = V_TRUE
#define SET_BOOL(v, b)  (v).num = b ? V_TRUE : V_FALSE
#define SET_UNDEF(v)    (v).num = V_UNDEF

// TODO: some saner numbering maybe
#define OBJ_TYPE_STRING        (1)
#define OBJ_TYPE_NATIVE_METHOD (2)
#define OBJ_TYPE_CLOSURE       (3)
#define OBJ_TYPE_ARRAY         (4)

#define OBJ_LEN(v)                ((VAR_LEN_OBJECT *) v.ptr)->len
#define OBJ_ALLOCATED(v)          ((VAR_LEN_OBJECT *) v.ptr)->allocated
#define CLOSURE_OBJ_IP(v)         ((CLOSURE_OBJECT *) v.ptr)->ip
#define CLOSURE_OBJ_N_LOCALS(v)   ((CLOSURE_OBJECT *) v.ptr)->n_local_vars
#define OBJ_DATA_PTR(v)           (((OBJECT *)(v).ptr)->val.ptr)
#define OBJ_TYPE(v)               (((OBJECT *)(v).ptr)->type.num)
#define OBJ_TYPE_PTR(v)           (((OBJECT *)(v).ptr)->type.ptr)
#define IS_STRING(v)              (((v.num & TAG_AND) == 0) && OBJ_TYPE(v) == OBJ_TYPE_STRING)
#define IS_NATIVE_METHOD(v)       (((v.num & TAG_AND) == 0) && OBJ_TYPE(v) == OBJ_TYPE_NATIVE_METHOD)
#define IS_CLOSURE(v)             (((v.num & TAG_AND) == 0) && OBJ_TYPE(v) == OBJ_TYPE_CLOSURE)
#define IS_ARRAY(v)               (((v.num & TAG_AND) == 0) && OBJ_TYPE(v) == OBJ_TYPE_ARRAY)
#define IS_VLO(v)                 (IS_ARRAY(v) || IS_STRING(v))
#define ARRAY_ITEMS(v)            ((VALUE *)(OBJ_DATA_PTR(v)))

void dump(VALUE v);
void dump_titled(char *title, VALUE v);
#endif