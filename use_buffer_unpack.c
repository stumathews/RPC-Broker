#include <msgpack.h>
#include <stdio.h>
#include <assert.h>
#include <stulibc.h>

void prepare(msgpack_sbuffer* sbuf) {
    msgpack_packer pk;

    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);
    /* 0st Object */
    
    // pack a map with 2 items
    msgpack_pack_map(&pk,2);
    msgpack_pack_str(&pk, 4);
    msgpack_pack_str_body(&pk, "Name", 4);
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "Stuart", 6);
    
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);
    msgpack_pack_int(&pk, 1);

    /* 1st object */
    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);
    /* 2nd object */
    msgpack_pack_str(&pk, 6);
    msgpack_pack_str_body(&pk, "second", 6);
    /* 3rd object */
    msgpack_pack_array(&pk, 2);
    msgpack_pack_int(&pk, 42);
    msgpack_pack_false(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "ExAmPlE", 7);
}

void unpack(char const* buf, size_t len) {
    
    /* buf is allocated by client. */
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    size_t off = 0;
    int i = 0;
    msgpack_unpacked_init(&result);

    // Go ahead unpack an object
    ret = msgpack_unpack_next(&result, buf, len, &off);

    // Go and get the rest of all was good
    while (ret == MSGPACK_UNPACK_SUCCESS) 
    {
        /* Use obj. */
        printf("Object no %d:\n", ++i);
        
        // We have the object
        msgpack_object obj = result.data;
        // print it:

        if( obj.type == MSGPACK_OBJECT_ARRAY )
        {
            //decode the array
            printf("Array Detected. Size of the array is: %d\n",obj.via.array.size );
            
            // array is in formt [int, boolean, string], iterate over it
            for( int i = 0; i < obj.via.array.size; i++)
            {
                printf("Element %d type is: %d\n", i, obj.via.array.ptr[i].type);
            }
        }
        else if( obj.type == MSGPACK_OBJECT_MAP )
        {
            //decode the map
            printf("Map Detected. Size of the map is: %d\n", obj.via.map.size );
            
            for( int i = 0; i < obj.via.map.size;i++)
            {
                printf("Element %d type is: %d\n",i, obj.via.map.ptr[i].val.type);
            }
        }

        /* Use obj. */
        printf("Preview: ");
        msgpack_object_print(stdout, obj);
        printf("\n\n");
        /* If you want to allocate something on the zone, you can use zone. */
        /* msgpack_zone* zone = result.zone; */
        /* The lifetime of the obj and the zone,  */

        ret = msgpack_unpack_next(&result, buf, len, &off);
    }
    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_CONTINUE) {
        printf("All msgpack_object in the buffer is consumed.\n");
    }
    else if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
        printf("The data in the buf is invalid format.\n");
    }
}

int main(void) {
    PRINT("Start\n");
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);

    prepare(&sbuf);
    unpack(sbuf.data, sbuf.size);

    msgpack_sbuffer_destroy(&sbuf);
    return 0;
}

/* Output */

/*
Object no 1:
[1, true, "example"]
Object no 2:
"second"
Object no 3:
[42, false]
All msgpack_object in the buffer is consumed.
*/
