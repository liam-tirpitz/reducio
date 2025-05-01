/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  H E A D E R S  ************************/
#define RECIRC_PORT 68

header stroke_label {
    sensor_id_t                     id;
    bit<32>                         detection_count;
    bit<8>                          phase_label;
}

struct my_ingress_headers_t {
    ethernet_h                      ethernet;
    ipv4_h                          ipv4;
    udp_h                           udp;
    stroke_label                    label;
    sensor_h[MAX_HEADER_COUNT]      sensors;
    sensor_h[4]                     sensors_add;
}

    /******  G L O B A L   I N G R E S S   M E T A D A T A  *********/

struct my_ingress_metadata_t {
    bit<8>                          dummy;
    bit<32>                         phase_change_delay;
    bit<1>                          phase_change_main; // 0 no change, 1 phase change detected
    bit<1>                          change_direction_main; // 0 beginning of punch, 1 punch ended
    bit<3>                          consider_ae;
    sensor_id_t                     pos;
    sensor_id_t                     primary_pos;
    bit<32>                         count_punches;
    bit<32>                         packet_count;
}


    /***********************  P A R S E R  **************************/
parser IngressParser1(packet_in        pkt,
    /* User */
    out my_ingress_headers_t          hdr,
    out my_ingress_metadata_t         meta,
    /* Intrinsic */
    out ingress_intrinsic_metadata_t  ig_intr_md)
{
    /* This is a mandatory state, required by Tofino Architecture */
     state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE);
        meta.dummy = 0;
        meta.pos = 0;
        meta.phase_change_delay = 0;
        meta.phase_change_main = 0;
        meta.change_direction_main = 0;
        meta.consider_ae = 0;
        meta.primary_pos = 0;
        meta.packet_count = 0;
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.ether_type) {
            ETHERTYPE_IPV4:  parse_ipv4;
            default: accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.src_addr) {
            0x0a000002 : parse_udp;
            default: reject;
        }
    }

    state parse_udp {
        pkt.extract(hdr.udp);
        transition parse_sensor;
    }

    state parse_sensor {
        pkt.extract(hdr.sensors.next);
        transition select(hdr.sensors.last.has_next) {
            0x01 : parse_sensor;
            default: accept;
        }
    }
}


/***************** M A T C H - A C T I O N  *********************/

#include "aggregations.p4"
#include "phase_detection.p4"


control Ingress1(
    /* User */
    inout my_ingress_headers_t                       hdr,
    inout my_ingress_metadata_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_t               ig_intr_md,
    in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md)
{
    // Handle Aggregation Registers in seperate Control
    Aggregations() aggregations;
    PhaseDetection() phase_detection;

    // Mark packets to be dropped.
    action drop() {
        ig_dprsr_md.drop_ctl = 1;
    }

    // set output port
    action setPort(PortId_t port) {
        ig_dprsr_md.drop_ctl = 0;
        ig_tm_md.ucast_egress_port = port;
    }

    // Determine output port.
    table selectPort {
        key = {
            ig_intr_md.ingress_port : exact;
        }
        actions = {
            setPort;
            drop;
        }
        const default_action = drop();
        size = 5;
    }

    // Read PhaseChangeDelay from table.
    action set_conf(bit<32> delay, sensor_id_t primary_pos) {
        meta.phase_change_delay = delay;
        meta.primary_pos = primary_pos;
    }

    table set_config {
        key = {
            hdr.sensors[0].id : exact;
        }
        actions = {
            set_conf;
        }
        const default_action = set_conf(50, 0);
        size = 10;
    }

    action set_phase_label(bit<8> label) {
      hdr.label.phase_label = label;
    }

    table phase_label {
      key = {
              meta.dummy : exact;
      }
      actions = {
              set_phase_label;
      }
      size = 5;
    }


    Register<data_t, _>(10) global_packet_counter;
    RegisterAction<_, _, bit<32>>(global_packet_counter) inc_packet_counter = {
        void apply(inout bit<32> value, out bit<32> result) {
            result = value;
            value = value + 1;
        }
    };
    RegisterAction<_, _, bit<32>>(global_packet_counter) read_packet_counter = {
        void apply(inout bit<32> value, out bit<32> result) {
            result = value;
        }
    };



    apply {
        ig_tm_md.bypass_egress     = 1; // The whole program only happens in the Ingress Pipeline
        set_config.apply(); // Load PhaseChangeDelay Parameter
        phase_detection.apply(
            hdr.sensors,
            meta
        ); // Track current phase based on read sensor value and registers

        aggregations.apply(
            ig_intr_md.ingress_port,
            hdr.sensors,
            ig_dprsr_md.drop_ctl,
            meta.consider_ae,
            ig_tm_md.ucast_egress_port); // Handle data aggregation
        if (meta.phase_change_main == 1 && meta.change_direction_main == 1 && meta.pos == meta.primary_pos) { // If the end of a punch is detected, an aggregation packet is created.
            // The aggregation packet is created by setting empty sensor headers in the current packet and recirculating it. Mirroring would be the better option here.
            hdr.sensors[0].id = 0;
            hdr.sensors[0].has_next = 1;

            hdr.sensors_add[0].setValid();
            hdr.sensors_add[0].id = 1;
            hdr.sensors_add[0].value = 0;
            hdr.sensors_add[0].has_next = 1;

            hdr.sensors_add[1].setValid();
            hdr.sensors_add[1].id = 2;
            hdr.sensors_add[1].value = 0;
            hdr.sensors_add[1].has_next = 1;

            hdr.sensors_add[2].setValid();
            hdr.sensors_add[2].id = 3;
            hdr.sensors_add[2].value = 0;
            hdr.sensors_add[2].has_next = 1;

            hdr.sensors_add[3].setValid();
            hdr.sensors_add[3].id = 4;
            hdr.sensors_add[3].value = 0;
            hdr.sensors_add[3].has_next = 0;

            ig_tm_md.ucast_egress_port = RECIRC_PORT;
            ig_dprsr_md.drop_ctl = 0;
        } else {
            // If we do not want to recirculate the packet, we need to determine the output port.
            meta.packet_count = inc_packet_counter.execute(hdr.sensors[0].id); // TODO This counts packets for ID 0 an additional time for every recirculation and not at all for the other ones
            selectPort.apply();
            if (ig_tm_md.ucast_egress_port != 60) { // If we already decided to forward it to the stability detection, don't reroute
                hdr.label.setValid();
                hdr.label.id = meta.primary_pos;
                hdr.label.detection_count = meta.count_punches;
                phase_label.apply();
            } else {
                hdr.sensors[3].value = (int<32>)meta.packet_count;
            }
        }
    }
}

    /*********************  D E P A R S E R  ************************/

control IngressDeparser1(packet_out pkt,
    /* User */
    inout my_ingress_headers_t                       hdr,
    in    my_ingress_metadata_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md)
{
    apply {
        pkt.emit(hdr);
    }
}