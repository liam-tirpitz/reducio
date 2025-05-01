control FailureDetection(
    inout sensor_h[5]           sensors,
    inout my_ingress_metadata2_t meta
)

{


    action set_params(
        bit<16> threshold0_low, bit<16> threshold0_high, 
        int<16> threshold1_low, int<16> threshold1_high,
        int<16> threshold2_low, int<16> threshold2_high,
        int<16> threshold3_low, int<16> threshold3_high,
        int<16> threshold4_low, int<16> threshold4_high,
        data_t difference) {
        meta.failure_threshold0_low = threshold0_low;
        meta.failure_threshold0_high = threshold0_high;
        meta.failure_threshold1_low = threshold1_low;
        meta.failure_threshold1_high = threshold1_high;
        meta.failure_threshold2_low = threshold2_low;
        meta.failure_threshold2_high = threshold2_high;
        meta.failure_threshold3_low = threshold3_low;
        meta.failure_threshold3_high = threshold3_high;
        meta.failure_threshold4_low = threshold4_low;
        meta.failure_threshold4_high = threshold4_high;
    }

    table set_failure_params {
        key = {
            sensors[0].id : exact;
        }
        actions = {
            set_params;
        }
        const default_action = set_params(1,1,1,1,1,1,1,1,1,1,1);
        size = 1;
    }

    action alert() {

    }

    apply {
        set_failure_params.apply();
        if ((bit<16>)(int<16>)meta.data0 > meta.failure_threshold0_high) {
            alert();
        } 
    }
}