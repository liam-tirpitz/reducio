/*************************************************************************
 ************* C O N S T A N T S    A N D   T Y P E S  *******************
**************************************************************************/


/*************************************************************************
 ***********************  H E A D E R S  *********************************
 *************************************************************************/

/*  Define all the headers the program will recognize             */
/*  The actual sets of headers processed by each gress can differ */

header recirc_meta_h {
    bit<16>   round;
}

header empty {
}

header bridge_h {
    bit<32> density;
 }

header sensor_count_h {
    bit<16> count;
}

header sensor_match_h {
    bit<16>   round;
    bit<16>   cluster;
}

header label_h {
    bit<8>      phase;
    // bit<32>     packet_count;
}