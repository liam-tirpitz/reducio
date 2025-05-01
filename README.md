# Reducio: Data Aggregation and Stability Detection for Industrial Processes Using In-Network Computing
This repository contains the source code of Reducio, a Tofino-based Data Aggregation and Stability Detection system for Industrial Processes.


## Structure
This project consists of two processing steps. 
In Phase 1, raw force data is aggregated, such that only a single packet describes a punch, by its core features (max Force, min Force etc.).
In Phase 2, the previously aggregated data is used to detect the state of the fine blanking process (in-control, out-of-control, ramp-up).

### Tooling
This project relies on the [Tofino SDE 9.7](https://github.com/barefootnetworks/Open-Tofino).


### Configure Switch
The configuration file, [config.json](control-plane/switch_control/config.json), determines the configuration of the Control Plane.
Read the [documentation](control-plane/switch_control/README.md) for details.

### Start Switch
To activate the switch, the p4 Code needs to be compiled and the control plane needs to be started.
Follow the general instruction to compile the [P4 Code](data-plane/Step12/combinedcluster.p4)).
Compile the [Control Plane](control-plane/switch_control) using ``` cmake . ``` and ``` make ```.
Start the Control Plane using ``` ./tofino_switch_control ```
Note, you may also use ``` ./tofino_switch_control sampling``` or ``` ./tofino_switch_control clustering``` to start 
the respective programs on both pipes of the switch.
In the default configuration, the sampling code is started on the first pipe and the clustering code is started on the second pipe.


### Extract Results
The Control Plane writes its local clustering information to a local file.

Additionally, the aggregated packets received by the clustering program are prepended with a single byte.
This label identifies the current phase of the fine blanking process (restart=41, stable=42, unstable=43, ramp up =44).
You may capture it wit tcpdump.

