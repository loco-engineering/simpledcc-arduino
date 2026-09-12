#ifndef CBOR_HANDLER_H
#define CBOR_HANDLER_H

#include <stdint.h>
#include <stddef.h>

void process_cbor_message(const uint8_t *buffer, size_t length);

#endif // CBOR_HANDLER_H
