/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  H E A D E R S  ************************/
#define RECIRC_PORT_CLUSTERING 196
#define CPU_PORT 192
#define OUT_INP4 141

struct my_ingress_headers2_t {
    bridge_h                        bridge;
    ethernet_h                      ethernet;
    ipv4_h                          ipv4;
    udp_h                           udp;
    label_h                         label;
    sensor_h[5]                     sensors;
}

struct pair {
    data_t   data;
    int<32>  count;
}

    /******  G L O B A L   I N G R E S S   M E T A D A T A  *********/

struct my_ingress_metadata2_t {
    MirrorId_t     mirror_session;
    bit<32>        pointer;
    data_t         data0;
    data_t         data1;
    data_t         data2;
    data_t         data3;
    data_t         data4;
    data_t         new_is_similar_to_previous0;
    data_t         new_is_similar_to_previous1;
    data_t         new_is_similar_to_previous2;
    data_t         new_is_similar_to_previous3;
    data_t         new_is_similar_to_previous4;
    int<32>        restart_detected;
    bit<16>        ring_size;
    bridge_h       recirc;
    bit<8>         dummy;
    bit<8>         phase;
    int<32>        restart_threshold;
    int<32>        temp_threshold;
    bit<32>        raw_pointer;
    bit<20>        comparison;
    bit<32>        current_packet_counter; 
    // bit<16>         failure_threshold0_low;
    // bit<16>         failure_threshold0_high;
    // int<16>         failure_threshold1_low;
    // int<16>         failure_threshold1_high;
    // int<16>         failure_threshold2_low;
    // int<16>         failure_threshold2_high;
    // int<16>         failure_threshold3_low;
    // int<16>         failure_threshold3_high;
    // int<16>         failure_threshold4_low;
    // int<16>         failure_threshold4_high;
    bit<8>         send_to_aggregation; 
}

    /***********************  P A R S E R  **************************/
parser IngressParser2(packet_in        pkt,
    /* User */
    out my_ingress_headers2_t          hdr,
    out my_ingress_metadata2_t         meta,
    /* Intrinsic */
    out ingress_intrinsic_metadata_t  ig_intr_md)
{
    /* This is a mandatory state, required by Tofino Architecture */
     state start {
        pkt.extract(ig_intr_md);
        pkt.advance(PORT_METADATA_SIZE);
        meta.mirror_session = CONTROLPLANE_MIRROR_UNKNOWN;
        meta.pointer = 0;

        meta.data0 = 0;
        meta.data1 = 0;
        meta.data2 = 0;
        meta.data3 = 0;
        meta.data4 = 0;

        meta.new_is_similar_to_previous0 = 0;
        meta.new_is_similar_to_previous1 = 0;
        meta.new_is_similar_to_previous2 = 0;
        meta.new_is_similar_to_previous3 = 0;
        meta.new_is_similar_to_previous4 = 0;

        meta.restart_detected = 0;
        meta.ring_size = 0;
        meta.recirc.density = 0;
        meta.dummy = 0;
        meta.phase = 0;
        meta.restart_threshold = 0;
        meta.temp_threshold = 0;
        meta.send_to_aggregation = 0;
        // meta.current_packet_counter = 0;

        transition select(ig_intr_md.ingress_port) {
            RECIRC_PORT_CLUSTERING      : parse_recirc;
            default : parse_ethernet;
        }
    }

    state parse_recirc {
        pkt.extract(meta.recirc);
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
        transition parse_udp;
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


#include "restart_detection.p4"
#include "failure_detection.p4"


control Ingress2(
    /* User */
    inout my_ingress_headers2_t                       hdr,
    inout my_ingress_metadata2_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_t               ig_intr_md,
    in    ingress_intrinsic_metadata_from_parser_t   ig_prsr_md,
    inout ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md,
    inout ingress_intrinsic_metadata_for_tm_t        ig_tm_md)
{
    RestartDetection() restart_detection;
    FailureDetection() failure_detection;


    action send(PortId_t port) {
        ig_tm_md.ucast_egress_port = port;
        ig_tm_md.bypass_egress     = 1;
        meta.send_to_aggregation = 1;
    }

    action drop() {
        ig_dprsr_md.drop_ctl = 1;
    }

    // Read parameters from table.
    action set_ring_size(bit<16> ring_size) {
        meta.ring_size = ring_size;
    }

    table set_params {
        key = {
            meta.dummy : exact;
        }
        actions = {
            set_ring_size;
        }
        const default_action = set_ring_size(500);
        size = 1;
    }

    
    table send_back_to_aggregation {
        key = {
            ig_intr_md.ingress_port : exact;
        }
        actions = {
            send;
        }
        size = 10;
    }


   Register<bit<32>, _>(1) ring_pointer;
   RegisterAction<_, _, bit<32>>(ring_pointer) pointer_increment = {
        void apply(inout bit<32> value, out bit<32> result) {
            if (meta.new_is_similar_to_previous0 == 0) {
                if (value < (bit<32>) meta.ring_size) {
                    value = value + 1;
                } else {
                    value = 0; 
                }
            }
            result = value;
        }
   };

   Register<data_t, _>(10) last_value;
   RegisterAction<_, _, data_t>(last_value) set_last_value = {
       void apply(inout data_t value, out data_t result) {
            if (meta.data0 == value) {
                result = 1;
            } else {
                result = 0;
            }
            value = meta.data0;
       }
   };

    Register<data_t, _>(65000) ringbuffer0;
    RegisterAction<data_t, _, data_t>(ringbuffer0) add_to_buffer0 = {
        void apply(inout data_t value, out data_t result){
            value = meta.data0;
            result = 0;
        }
    };

   Register<data_t, _>(65000) counter0;
   RegisterAction<_, _, data_t>(counter0) set_counter0 = {
        void apply(inout data_t value, out data_t result) {
            result = value;
            if (meta.new_is_similar_to_previous0 == 1) {
                value = value + 1;
            } else {
                value = 1;
            }       
        }
   };

    Register<data_t, _>(65000) ringbuffer1;
    RegisterAction<data_t, _, data_t>(ringbuffer1) add_to_buffer1 = {
        void apply(inout data_t value, out data_t result){
            value = meta.data1;
            result = 0;
        }
    };

   Register<data_t, _>(65000) counter1;
   RegisterAction<_, _, data_t>(counter1) set_counter1 = {
        void apply(inout data_t value, out data_t result) {
            result = value;
            if (meta.new_is_similar_to_previous1 == 1) {
                value = value + 1;
            } else {
                value = 1;
            }       
        }
   };

    Register<data_t, _>(65000) ringbuffer2;
    RegisterAction<data_t, _, data_t>(ringbuffer2) add_to_buffer2 = {
        void apply(inout data_t value, out data_t result){
            value = meta.data2;
            result = 0;
        }
    };

   Register<data_t, _>(65000) counter2;
   RegisterAction<_, _, data_t>(counter2) set_counter2 = {
        void apply(inout data_t value, out data_t result) {
            result = value;
            if (meta.new_is_similar_to_previous2 == 1) {
                value = value + 1;
            } else {
                value = 1;
            }       
        }
   };

    Register<data_t, _>(65000) ringbuffer3;
    RegisterAction<data_t, _, data_t>(ringbuffer3) add_to_buffer3 = {
        void apply(inout data_t value, out data_t result){
            value = meta.data3;
            result = 0;
        }
    };

   Register<data_t, _>(65000) counter3;
   RegisterAction<_, _, data_t>(counter3) set_counter3 = {
        void apply(inout data_t value, out data_t result) {
            result = value;
            if (meta.new_is_similar_to_previous3 == 1) {
                value = value + 1;
            } else {
                value = 1;
            }       
        }
   };

    Register<data_t, _>(65000) ringbuffer4;
    RegisterAction<data_t, _, data_t>(ringbuffer4) add_to_buffer4 = {
        void apply(inout data_t value, out data_t result){
            value = meta.data4;
            result = 0;
        }
    };

   Register<data_t, _>(65000) counter4;
   RegisterAction<_, _, data_t>(counter4) set_counter4 = {
        void apply(inout data_t value, out data_t result) {
            result = value;
            if (meta.new_is_similar_to_previous4 == 1) {
                value = value + 1;
            } else {
                value = 1;
            }       
        }
   };

   // Eval Things
   Register<bit<32>, _>(1) raw_pointer;
   RegisterAction<_, _, bit<32>>(raw_pointer) raw_pointer_increment = {
        void apply(inout bit<32> value, out bit<32> result) {
            value = value + 1;
            result = value;
        }
   };

    Register<data_t, _>(65000) raw_storage;
    RegisterAction<data_t, _, data_t>(raw_storage) add_to_raw = {
        void apply(inout data_t value, out data_t result){
            value = hdr.sensors[1].value;
            result = 0;
        }
    };



    action send_to_cp() {
        ig_dprsr_md.mirror_type = MIRROR_TYPE_I2E;
        meta.mirror_session = CONTROLPLANE_MIRROR_UNKNOWN;
    }


    action recirculate(bit<9> recirc_port) {
        hdr.bridge.setValid();
        ig_tm_md.ucast_egress_port = recirc_port;
        ig_dprsr_md.drop_ctl = 0;
    }

    action set_phase_label(bit<8> label) {
      hdr.label.phase = hdr.label.phase + label;
      hdr.label.setValid();
      ig_dprsr_md.drop_ctl = 0;
      ig_tm_md.ucast_egress_port = OUT_INP4;
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


    apply {
        set_params.apply();
        send_back_to_aggregation.apply();

        if (meta.send_to_aggregation == 0) {
            restart_detection.apply(hdr.sensors, meta);
            if (meta.restart_detected == 1) {

            } else {
                meta.data0 = (int<32>)((bit<32>)hdr.sensors[0].value & (bit<32>) MASK); 
                meta.data1 = (int<32>)((bit<32>)hdr.sensors[1].value & (bit<32>) MASK); 
                meta.data2 = (int<32>)((bit<32>)hdr.sensors[2].value & (bit<32>) MASK); 
                meta.data3 = (int<32>)((bit<32>)hdr.sensors[3].value & (bit<32>) MASK); 
                meta.data4 = (int<32>)((bit<32>)hdr.sensors[4].value & (bit<32>) MASK); 
                set_last_value.execute(0);
                meta.new_is_similar_to_previous0 = 0; //set_last_value.execute(0);
                meta.pointer =  pointer_increment.execute(0);
                add_to_buffer0.execute(meta.pointer);
                set_counter0.execute(meta.pointer);
                add_to_buffer1.execute(meta.pointer);
                set_counter1.execute(meta.pointer);
                add_to_buffer2.execute(meta.pointer);
                set_counter2.execute(meta.pointer);
                add_to_buffer3.execute(meta.pointer);
                set_counter3.execute(meta.pointer);
                add_to_buffer4.execute(meta.pointer);
                set_counter4.execute(meta.pointer);


                phase_label.apply();
                failure_detection.apply(hdr.sensors, meta, hdr.label, ig_dprsr_md.mirror_type);

            }  
            // Eval Things
            meta.raw_pointer = raw_pointer_increment.execute(0);
            add_to_raw.execute(meta.raw_pointer);
        }

    }
}

    /*********************  D E P A R S E R  ************************/

control IngressDeparser2(packet_out pkt,
    /* User */
    inout my_ingress_headers2_t                       hdr,
    in    my_ingress_metadata2_t                      meta,
    /* Intrinsic */
    in    ingress_intrinsic_metadata_for_deparser_t  ig_dprsr_md)
{
    Mirror() cpu_mirror;

    apply {
        if (ig_dprsr_md.mirror_type == MIRROR_TYPE_I2E) {
          cpu_mirror.emit<mirror_h>(meta.mirror_session, { hdr.label.phase });
        }
        pkt.emit(hdr);
    }
}
