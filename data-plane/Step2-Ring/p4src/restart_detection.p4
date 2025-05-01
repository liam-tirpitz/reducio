control RestartDetection(
    inout sensor_h[5]           sensors,
    inout my_ingress_metadata2_t meta
)
{
    // Read RestartThreshold from table.
    action set_restart_thresh(int<32> threshold) {
        meta.restart_threshold = threshold;
    }
    table set_restart_threshold {
        key = {
            meta.dummy : exact;
        }
        actions = {
            set_restart_thresh;
        }
        const default_action = set_restart_thresh(5000);
        size = 5;
    }

    Register<int<32>, _>(1) last_max;
    RegisterAction<_, _, int<32>>(last_max) compare_to_last = {
        void apply(inout int<32> value, out int<32> result) {
            if (value > 0) {
                 if (value > meta.temp_threshold) {
                     result = 1;
                 } else {
                  result = 0;
                 }
            } else {
                result = 0;
            }
            value = sensors[1].value;
        }
    };

    apply {
        set_restart_threshold.apply();
        meta.temp_threshold = sensors[1].value + meta.restart_threshold;
        meta.restart_detected = 0; //compare_to_last.execute(0);
    }
}