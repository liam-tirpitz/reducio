control Aggregations(
    in    bit<9>                        port,
    inout sensor_h[MAX_HEADER_COUNT]    sensors,
    inout bit<3>                        drop_ctl,
    inout bit<3>                        consider_ae,
    inout bit<9>                        egress_port)
{
    REGISTER_TYPE_0
    REGISTER_TYPE_1
    REGISTER_TYPE_2
    REGISTER_TYPE_3
    REGISTER_TYPE_4


    STORE_MODE_0
    STORE_MODE_1
    STORE_MODE_2
    STORE_MODE_3
    STORE_MODE_4

    Register<bit<32>, _>(3) counter;
    RegisterAction<bit<32>,_,bit<3>>(counter)
     increment_counter = {
        void apply(inout bit<32> value, out bit<3> trigger) {
            if (value >= 100) {
                value = 0;
                trigger = 1;
            } else {
                value = value + 1;
                trigger = 0;
            }
        }
    };


    Register<int<32>, _>(10) data_storage00;                    
    RegisterAction<int<32>, _, data_t>(data_storage00)         
    pickup_data00 = {                                              
        void apply(inout int<32> register_data, out data_t result) { 
            result = register_data;                           
            register_data = 0;                                                   
        }                                                           
    };                                                              
    RegisterAction<int<32>, _, data_t>(data_storage00)         
    leave_data00  = {                                          
        void apply(inout int<32> reg) {                         
            if (sensors[0].value > 0)  {                      
                if (sensors[0].value > reg)  {                
                    reg = sensors[0].value;                   
                }                                               
            }                                                   
        }                                                       
    };


    Register<int<32>, _>(10) data_storage11;                    
    RegisterAction<int<32>, _, data_t>(data_storage11)         
    pickup_data11 = {                                              
        void apply(inout int<32> register_data, out data_t result) { 
            result = register_data;                           
            register_data = 0;                                                   
        }                                                           
    };                                                              
    RegisterAction<int<32>, _, data_t>(data_storage11)         
    leave_data11  = {                                          
        void apply(inout int<32> reg) {                         
            if (sensors[0].value > 0)  {                      
                if (sensors[0].value > reg)  {                
                    reg = sensors[0].value;                   
                }                                               
            }                                                   
        }                                                       
    };

    Register<int<32>, _>(10) data_storage22;                    
    RegisterAction<int<32>, _, data_t>(data_storage22)         
    pickup_data22 = {                                              
        void apply(inout int<32> register_data, out data_t result) { 
            result = register_data;                           
            register_data = 0;                                                   
        }                                                           
    };                                                              
    RegisterAction<int<32>, _, data_t>(data_storage22)         
    leave_data22  = {                                          
        void apply(inout int<32> reg) {                         
            if (sensors[0].value > 0)  {                      
                if (sensors[0].value > reg)  {                
                    reg = sensors[0].value;                   
                }                                               
            }                                                   
        }                                                       
    };

    Register<int<32>, _>(10) data_storage44;                    
    RegisterAction<int<32>, _, data_t>(data_storage44)         
    pickup_data44 = {                                              
        void apply(inout int<32> register_data, out data_t result) { 
            result = register_data;                           
            register_data = 0;                                                   
        }                                                           
    };                                                              
    RegisterAction<int<32>, _, data_t>(data_storage44)         
    leave_data44  = {                                          
        void apply(inout int<32> reg) {                         
            if (sensors[0].value > 0)  {                      
                if (sensors[0].value > reg)  {                
                    reg = sensors[0].value;                   
                }                                               
            }                                                   
        }                                                       
    };



    apply {         


        leave_or_pickup0.apply();
        leave_or_pickup1.apply();
        leave_or_pickup2.apply();
        leave_or_pickup3.apply();
        leave_or_pickup4.apply();


        if (port == 68) {
                sensors[0].id = 0;                    
                sensors[0].value = pickup_data00.execute(0);   
                sensors[1].id = 1;                    
                sensors[1].value = pickup_data11.execute(0);   
                sensors[2].id = 2;                    
                sensors[2].value = pickup_data22.execute(0);   
                sensors[4].id = 4;                    
                sensors[4].value = pickup_data44.execute(0);   

                egress_port = 60;
        } else {
            if (sensors[0].id == 0) {
                leave_data00.execute(0);
            } else if (sensors[0].id == 1 ) {
                leave_data11.execute(0);
            } else if (sensors[0].id == 2 ) {
                leave_data22.execute(0);
            } else if (sensors[0].id == 4 ) {
                leave_data44.execute(0);
            }
        }

    }
}