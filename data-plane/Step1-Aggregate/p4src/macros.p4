#define OTHER 0
#define LAST 1
#define MAX 2
#define MIN 3
#define SUM 4


#define REG(x)                                                  \
Register<int<32>, _>(10) data_storage##x;                    \
RegisterAction<int<32>, _, data_t>(data_storage##x)         \
pickup_data##x = {                                              \
    void apply(inout int<32> register_data, out data_t result) { \
        result = register_data;                           \
        register_data = 0;                                                   \
    }                                                           \
};                                                              \

#define WITH_NORMAL_STORE(x, y)                                    \
RegisterAction<int<32>, _, data_t>(data_storage##x)         \
leave_data##x  = {                                              \
    void apply(inout int<32> reg) {                         \
        if (sensors[##y].value != 0) { \
            reg = sensors[##y].value;                           \
        }\
    }                                                           \
};

#define WITH_MAX(x, y)                                      \
RegisterAction<int<32>, _, data_t>(data_storage##x)         \
leave_data##x  = {                                          \
    void apply(inout int<32> reg) {                         \
        if (sensors[##y].value > 0)  {                      \
            if (sensors[##y].value > reg)  {                \
                reg = sensors[##y].value;                   \
            }                                               \
        }                                                   \
    }                                                       \
};

#define WITH_MIN(x, y)                                    \
RegisterAction<int<32>, _, data_t>(data_storage##x)         \
leave_data##x  = {                                          \
    void apply(inout int<32> reg) {                         \
        if (sensors[##y].value < reg) {          \
            reg = sensors[##y].value; \
        }\
    }                                                           \
};

#define WITH_SUM(x, y)                                             \
RegisterAction<int<32>, _, data_t>(data_storage##x)         \
leave_data##x  = {                                                \
    void apply(inout int<32> reg) {                         \
        reg = reg  |+| sensors[##y].value;            \
    }                                                           \
};

#define CREG(x)                                                 \
Register<bit<16>, _>(10) counter_storage##x;                 \
RegisterAction<bit<16>, _, bit<16>>(counter_storage##x)      \
pickup_count##x = {                                             \
    void apply(inout bit<16> register_data, out bit<16> result) { \
        result = register_data;                         \
        register_data = 0;                                                   \
    }                                                           \
};                                                              \
RegisterAction<bit<16>, _, bit<16>>(counter_storage##x)      \
leave_count##x  = {                                             \
    void apply(inout bit<16> reg) {                         \
        reg = reg |+| 1;                        \
    }                                                           \
};

// Can't remove exit beacuse placement
#define VALUE_TO_REG(x, y, z)             \
action pickup##z(sensor_id_t output_id, bit<9> output_port, bit<16> reg_id) { \
    sensors[y].id = output_id;                    \
    sensors[y].value = pickup_data##z.execute(reg_id);   \
    sensors[y].aggregation_type = AGG_TYPE_##y;     \
    egress_port = output_port;\
}\
action leave##z(bit<16> reg_id) {\
    leave_data##z.execute(reg_id); \
    drop_ctl = 1;  \
}\
table leave_or_pickup##z {\
    key = {port : ternary; sensors[##x].id : ternary;}\
    actions = {pickup##z; leave##z; NoAction;}\
    const default_action = NoAction;\
    size = 100;\
}

#define HANDLE_COUNTER(x, y, z)             \
action pick##z(bit<16> reg_id) {                           \
    sensors[y].counter = pickup_count##z.execute(reg_id);\
}\
action countup##z(bit<16> reg_id) {\
    leave_count##z.execute(reg_id); \
}\
table leave_or_pickup_count##z {\
    key = {port : ternary; sensors[##x].id : ternary;}\
    actions = {pick##z; countup##z; NoAction;}\
    const default_action = NoAction;\
    size = 100;\
}

#define REG_WITH_ADD(x,y)     REG(x) WITH_SUM(x,y)
#define REG_WITH_MIN(x,y)     REG(x) WITH_MIN(x,y)
#define REG_WITH_MAX(x,y)     REG(x) WITH_MAX(x,y)
#define REG_WITH_SUM(x,y)     REG(x) WITH_SUM(x,y)
#define REG_LAST(x,y)         REG(x) WITH_NORMAL_STORE(x,y)
