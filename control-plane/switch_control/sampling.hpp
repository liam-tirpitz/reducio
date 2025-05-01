#pragma once
#include "include/json.hpp"
#include <vector>
#include "tofino_switch_control.hpp"
using json = nlohmann::json;

class Sampling {
  private:
    TofinoSwitchControl* tsc;
    int ingress_port;
    int primary_pos[3];
    int default_primary = 1;
    int outlier_series[5];
    int not_outlier_series[5];


  public:
    Sampling(TofinoSwitchControl* tsc);
    void initSampling(json config);
    void setOut(int port);
    void setOuts(std::set<int> in_ports, int out_port);
    void setConfig(json config, int machineID);
    void resetCounters();
    std::vector<int> getPhaseDetectionSensorIDs(json config, int machineID);
    nlohmann::json getMachineObject(json config, int machine_id);
  
    json readCounters(json config, int machineID);
    json readAllCounters(json config);

};

