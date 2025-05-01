#define MAX_HEADER_COUNT 5
/*
 Defines how Data is stored in Registers and what operations are possible for a register

 STORE_MODE(X,Y,Z)
 X LEAVE_PACKET_POSITION
 Y COLLECT_PACKET_POSITION
 Z REGISTER_ID
 Options : VALUE_TO_REG, STORE_SENSOR_IN_REG, STORE_MIN_SENSOR_IN_REG, STORE_MAX_SENSOR_IN_REG

 REGISTER_TYPE(X,Y)
 X REGID
 Y PACKET_POSITION
 Options : REG_WITH_ADD, REG_WITH_MIN, REG_WITH_MAX

 AGG_TYPE_X
 Is written to outgoing packets to indicate the type of aggragation that was performed
 Options : LAST, MAX, MIN, SUM, OTHER

 SENSOR_ID_OUT_X Y Defines which SensorID Y is assigned when reading from Register X
*/

#define AGG_TYPE_0 MAX
#define STORE_MODE_0 VALUE_TO_REG(0, 0, 0)
#define COUNT_MODE_0 HANDLE_COUNTER(0, 0, 0)
#define REGISTER_TYPE_0 REG_WITH_MAX(0, 0)
#define SENSOR_ID_OUT_0 0

#define AGG_TYPE_1 MAX
#define STORE_MODE_1 VALUE_TO_REG(0, 1, 1)
#define COUNT_MODE_1 HANDLE_COUNTER(0, 1, 1)
#define REGISTER_TYPE_1 REG_WITH_MAX(1, 0)
#define SENSOR_ID_OUT_1 1

#define AGG_TYPE_2 MAX
#define STORE_MODE_2 VALUE_TO_REG(0, 2 , 2)
#define COUNT_MODE_2 HANDLE_COUNTER(0, 2, 2)
#define REGISTER_TYPE_2 REG_WITH_MAX(2, 0)
#define SENSOR_ID_OUT_2 2

#define AGG_TYPE_3 MAX
#define STORE_MODE_3 VALUE_TO_REG(0, 3, 3)
#define COUNT_MODE_3 HANDLE_COUNTER(0, 3, 3)
#define REGISTER_TYPE_3 REG_WITH_MAX(3, 0)
#define SENSOR_ID_OUT_3 3

#define AGG_TYPE_4 SUM
#define STORE_MODE_4 VALUE_TO_REG(0, 4 , 4)
#define COUNT_MODE_4 HANDLE_COUNTER(0, 4, 4)
#define REGISTER_TYPE_4 REG_WITH_SUM(4, 0)
#define SENSOR_ID_OUT_4 4