/*************************************************************************
 ************* C O N S T A N T S    A N D   T Y P E S  *******************
**************************************************************************/
const bit<16> ETHERTYPE_IPV4 = 0x0800;
typedef bit<3> mirror_type_t;
const mirror_type_t MIRROR_TYPE_I2E = 1;
const mirror_type_t MIRROR_TYPE_E2E = 2;
const MirrorId_t CONTROLPLANE_MIRROR_UNKNOWN  = 1;
const MirrorId_t CONTROLPLANE_MIRROR_RESTART  = 2;

/*************************************************************************
 ***********************  H E A D E R S  *********************************
 *************************************************************************/

/*  Define all the headers the program will recognize             */
/*  The actual sets of headers processed by each gress can differ */

/* Standard ethernet header */
header ethernet_h {
    bit<48>   dst_addr;
    bit<48>   src_addr;
    bit<16>   ether_type;
}

header ipv4_h {
    bit<4>   version;
    bit<4>   ihl;
    bit<8>   diffserv;
    bit<16>  total_len;
    bit<16>  identification;
    bit<3>   flags;
    bit<13>  frag_offset;
    bit<8>   ttl;
    bit<8>   protocol;
    bit<16>  hdr_checksum;
    bit<32>  src_addr;
    bit<32>  dst_addr;
}

header udp_h {
    bit<16>  src_port;
    bit<16>  dst_port;
    bit<16>  len;
    bit<16>  checksum;
}

typedef bit<16> sensor_id_t;
typedef int<32> data_t;

header mirror_h {
    bit<8>      label;
}



header sensor_h {
    sensor_id_t  id;
    data_t       value;
    bit<16>      counter;
    bit<7>       aggregation_type;
    bit<1>       has_next;
}