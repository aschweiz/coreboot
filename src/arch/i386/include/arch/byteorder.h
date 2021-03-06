#ifndef _BYTEORDER_H
#define _BYTEORDER_H

#define __LITTLE_ENDIAN 1234

#include <swab.h>

#define cpu_to_le32(x) ((unsigned int)(x))
#define le32_to_cpu(x) ((unsigned int)(x))
#define cpu_to_le16(x) ((unsigned short)(x))
#define le16_to_cpu(x) ((unsigned short)(x))
#define cpu_to_be32(x) swab32((x))
#define be32_to_cpu(x) swab32((x))
#define cpu_to_be16(x) swab16((x))
#define be16_to_cpu(x) swab16((x))

#define ntohl(x) be32_to_cpu(x)
#define htonl(x) cpu_to_be32(x)

#endif /* _BYTEORDER_H */
