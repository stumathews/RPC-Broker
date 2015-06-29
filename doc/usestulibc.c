#include <stulibc.h>
#include <msgpack.h>
#include <stdio.h>

void simple1()
{


}

void simple()
{
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_zone mempool;
    msgpack_object deserialized;

    /* msgpack::sbuffer is a simple buffer implementation. */
    msgpack_sbuffer_init(&sbuf);

    /* serialize values into the buffer using msgpack_sbuffer_write callback function. */
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    // add 4 items to an array
    msgpack_pack_array(&pk, 4);
    // pack an integer
    msgpack_pack_int(&pk, 1);
    // pack a true value
    msgpack_pack_true(&pk);
    // pack space for a 7 character string
    msgpack_pack_str(&pk, 7);
    // put the string in the space
    msgpack_pack_str_body(&pk, "example", 7);
    // pack an integer
    msgpack_pack_int(&pk, 1);

    /* deserialize the buffer into msgpack_object instance. */
    /* deserialized object is valid during the msgpack_zone instance alive. */
    msgpack_zone_init(&mempool, 2048);

    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);

    /* print the deserialized object. */
    msgpack_object_print(stdout, deserialized);
    puts("");

    msgpack_zone_destroy(&mempool);
    msgpack_sbuffer_destroy(&sbuf);
}

int main( int argc, char** argv)
{
    PRINT("Hello WORLD!\n");
    simple();
    simple1();
}
