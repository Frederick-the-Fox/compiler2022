#include "koopaaux.h"
#include "assert.h"

size_t get_type_size(koopa_raw_type_t ty)
{
    if (ty->tag == KOOPA_RTT_UNIT)
        return 0;
    if (ty->tag == KOOPA_RTT_ARRAY)
        return ty->data.array.len * get_type_size(ty->data.array.base);
    return 4;
}

size_t get_pointer_size(koopa_raw_type_t ty)
{
    assert(ty->tag == KOOPA_RTT_POINTER);
    return get_type_size(ty->data.pointer.base);
}