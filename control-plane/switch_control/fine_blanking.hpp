#pragma once
#include "tofino_switch_control.hpp"
#include "sampling.hpp"
#include <list>
#include <ctime>
#include <iostream>
#include "include/json.hpp"
#include <stdio.h>
using json = nlohmann::json;

class FineBlanking {
  private:
    TofinoSwitchControl* tsc;
    Sampling* samplingClass;
    int stable_std_threshold;
    uint32_t last_pointer[5];
    uint32_t last_raw_pointer;
    enum State { ramp_up, stable, unstable };
    State current_state;
    State states[5];
    json config;

    bool stableSinceRestart;
    int ring_size;
    int window;
    int gradient_window;
    bool gradient_detection_enabled;
    std::string filename;
    std::string filename_raw;
    std::chrono::system_clock::time_point start_time;
    void resetRing(unsigned int value, int sensor_index);
    void writeRawDataToFile(std::ofstream& file, std::chrono::system_clock::time_point current_time, int value, int pos);
    std::vector<int> getStabilityDetectionSensorIDs(json config, int machineID);

    typedef struct {
      int value;
      uint32_t position;
      int repeat;
    } DataElement;

    std::list<DataElement> local_ring[5];
    std::list<DataElement> new_data[5];

  public:
    FineBlanking(TofinoSwitchControl* tsc, int stable_std_threshold, uint32_t restart_threshold, int window, int ring_size, int gradient_window, bool enable_gradient_detection, std::chrono::system_clock::time_point start_time, json config);
    std::vector<int> getWindow(int size, int sensor_index);
    float rolling_std(std::vector<int> elements);
    double gradient(std::vector<int> elements);
    json determine_current_state();
    void updateFromRing(TofinoSwitchControl* tsc, int sensor_index);
    void print_current_ring();
    void reset(unsigned int value, int sensor_index);
    void setRestartThreshold(uint32_t threshold);
    void raw_data_to_file(std::chrono::system_clock::time_point current_time);
    void appendLineToFile(std::string filepath, std::string line);
    void initFile(std::string filepath, const char* params);
    bool is_current_state_rampup();
    json setState();
    nlohmann::json getMachineObject(json config, int machine_id);
    std::vector<int> getPhaseDetectionSensorIDs(json config, int machineID);

};
