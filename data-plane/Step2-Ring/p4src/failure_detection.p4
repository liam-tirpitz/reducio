control FailureDetection(
    inout sensor_h[5]           sensors,
    inout my_ingress_metadata2_t meta,
    inout label_h label,
    inout mirror_type_t mt
)

{


    action send_to_cp() {
        mt = MIRROR_TYPE_I2E;
        meta.mirror_session = CONTROLPLANE_MIRROR_UNKNOWN;
    }


    action in_range() {

    }

    action out_of_range() {
        label.phase = label.phase + 10;
        label.setValid();
        send_to_cp();
    }

    table compare_0 {
        key = {
             (bit<20>)(bit<32>)sensors[0].value -  (bit<20>)(bit<32>)sensors[1].value: range @name("difference");
            sensors[0].id : exact;
        }
        actions = {
            in_range();
            out_of_range();
        }
        size = 100;
    }




    table check_0{
        key = {
            meta.comparison : range @name("value");
            sensors[4].id : exact;
        }
        actions = {
            in_range();
            out_of_range();
        }
        // const default_action = out_of_range();
        size = 100;
    }

    apply {

        meta.comparison = (bit<20>)((bit<32>)sensors[4].value>>12);
        check_0.apply();
        // check_1.apply();
        compare_0.apply();


    }
}