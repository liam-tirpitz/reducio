struct phase_detection_data {
    bit<32> count;
    bit<32> current_phase; // 0 inactive, 1 punch
}

control PhaseDetection(
    inout sensor_h[MAX_HEADER_COUNT]    sensors,
    inout my_ingress_metadata_t         meta)
{
    // Register counting zero-noise packets and holding the current phase (inactive or punch)
    Register<phase_detection_data, _>(10) phase_state;

    // React to reading a zero noise value.
    RegisterAction<phase_detection_data, _, bit<1>>(phase_state) handle_zero = {
        void apply(inout phase_detection_data value, out bit<1> phase_change) {
            if (value.current_phase > 0 && value.count > meta.phase_change_delay) { //currently active, interpret counter as zero counter // Only cleare punch end, after phase_change_delay many zero packets
                    value.current_phase = 0;
                    phase_change = 1;
                    value.count = 0;
                     // If there are currently zeros, but we were previously active, we are at the end of a punch, send collection!
            } else { //currently inactive, interpret counter as nonzero counter
                phase_change = 0; // If there are currently zeros and there were zeros before, we are still inactive
                value.count = value.count + 1;
            }
        }
    };

    // React to reading a non-zero value.
    RegisterAction<phase_detection_data, _, bit<1>>(phase_state) handle_nonzero = {
        void apply(inout phase_detection_data value, out bit<1> phase_change) {
            value.current_phase = 1;
            value.count = 0;
        }
    };

    // Track current phase based on read sensor value and registers.
    action count_zeros_main(sensor_id_t pos) {
        meta.change_direction_main = 1;
        meta.phase_change_main = handle_zero.execute(pos);
        meta.pos = pos;
    }
    action count_nonzeros_main(sensor_id_t pos) {
        meta.change_direction_main = 0;
        meta.phase_change_main = handle_nonzero.execute(pos);
        meta.pos = pos;
    }


    table count_zero_table {
        key = {
            sensors[0].id           : exact; 
            sensors[0].value        : ternary;
        }
        actions = {count_zeros_main; count_nonzeros_main;}
        size = 20;
    }

    Register<bit<32>, _>(20) count_detected_punches;
    RegisterAction<bit<32>, _, bit<32>>(count_detected_punches) count_punch = {
        void apply(inout bit<32> value, out bit<32> result) {
            value = value + 1;
            result = value;
        }
    };
    RegisterAction<bit<32>, _, bit<32>>(count_detected_punches) read_punch = {
        void apply(inout bit<32> value, out bit<32> result) {
            result = value;
        }
    };

    apply {
        count_zero_table.apply();
        if (meta.phase_change_main == 1 && meta.change_direction_main == 1) {
            meta.count_punches = count_punch.execute(meta.pos);
        } else {
            meta.count_punches = read_punch.execute(meta.pos);

        }
    }
}