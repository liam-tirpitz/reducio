#pragma once
#include "tofino_switch_control.hpp"
#include "include/json.hpp"
#include <stdio.h>
using json = nlohmann::json;

class FailureDetection {
  private:
    TofinoSwitchControl* tsc;

  public:
    FailureDetection(TofinoSwitchControl* tsc);
    void setThresholds(json config);
    void setDifferences(json config);
    void handleOutofBoundsEvent(json config, sniff_sensor* sensor0, sniff_sensor* sensor1, sniff_sensor* sensor2, sniff_sensor* sensor3, sniff_sensor* sensor4, sniff_sensor* sensor5);

};
