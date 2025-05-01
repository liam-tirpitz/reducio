/* -*- P4_16 -*- */
#include <core.p4>
#include <tna.p4>

#include "headers_global.p4"

#include "../Step1-Aggregate/p4src/config.p4"
#include "../Step1-Aggregate/p4src/headers.p4"
#include "../Step1-Aggregate/p4src/macros.p4"
#include "../Step1-Aggregate/p4src/ingress.p4"
#include "../Step1-Aggregate/p4src/egress.p4"

#include "../Step2-Ring/p4src/headers.p4"
#include "../Step2-Ring/p4src/ingress.p4"
#include "../Step2-Ring/p4src/egress.p4"

/************ F I N A L   P A C K A G E ******************************/
Pipeline(
    IngressParser1(),
    Ingress1(),
    IngressDeparser1(),
    EgressParser1(),
    Egress1(),
    EgressDeparser1()
) pipe_1;

Pipeline(
    IngressParser2(),
    Ingress2(),
    IngressDeparser2(),
    EgressParser2(),
    Egress2(),
    EgressDeparser2()
) pipe_2;

Switch(pipe_1, pipe_2) main;
