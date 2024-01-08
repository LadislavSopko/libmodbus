#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <errno.h>
#include <stdlib.h>

#include <modbus.h>

void mb_rw_callback() {
    printf("Modbus RW callback called.\n");
}

void mb_w_callback(uint16_t cnt, modbus_register_changed_value const * changes) {
    printf("Modbus W callback called with: [%d] changes.\n", cnt);

    for(int i = 0; i < cnt; i++) {
        printf("Address: %d, value: %d -> %d\n", changes[i].address, changes[i].old_value, changes[i].new_value);
    }
}

int main(void)
{
    modbus_t* ctx;
    modbus_mapping_t* mb_mapping;

    ctx = modbus_new_udp("0.0.0.0", 1502);
    modbus_set_debug(ctx, TRUE);

    mb_mapping = modbus_mapping_new(500, 500, 500, 500);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
            modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    modbus_udp_listen(ctx, 1);

    for (;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc;

        rc = modbus_receive(ctx, query);
        if (rc != -1) {
            /* rc is the query size */
            modbus_reply_with_calback(ctx, query, rc, mb_mapping, mb_rw_callback, mb_w_callback);
        }
        else {
            /* Connection closed by the client or error */
            break;
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
}
