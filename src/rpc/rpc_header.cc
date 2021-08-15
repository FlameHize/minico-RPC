#include "../../include/rpc/rpc_header.h"

static const uint16_t DefaultMagic = 0x7777;

void set_rpc_header(void* header,int msg_len)
{
    ((RpcHeader*) header)->magic = DefaultMagic;
    ((RpcHeader*) header)->len = htonl(msg_len);
}