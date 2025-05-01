/* -*- P4_16 -*- */
#include <core.p4>
#include <tna.p4>

#include "../../Step12/headers_global.p4"
#include "config.p4"
#include "headers.p4"
#include "macros.p4"
#include "ingress.p4"
#include "egress.p4"

/************ F I N A L   P A C K A G E ******************************/
Pipeline(
    IngressParser1(),
    Ingress1(),
    IngressDeparser1(),
    EgressParser1(),
    Egress1(),
    EgressDeparser1()
) pipe;

Switch(pipe) main;
