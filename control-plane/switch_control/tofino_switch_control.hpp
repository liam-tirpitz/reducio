// Based on https://laboratory.comsys.rwth-aachen.de/theses-kunze/2020-ma-sokolowski-code

#pragma once
#include <bf_switchd/bf_switchd.h>
#include <loguru.hpp>

extern "C" {
#include <traffic_mgr/traffic_mgr.h>
#include <bf_pm/bf_pm_intf.h>
}

#include "switchd.hpp"

#include "tofino_register.hpp"
#include "tofino_tables.hpp"
#include "cpu_port_capture.hpp"
#include <pthread.h>

class TofinoSwitchControl {
 public:
  Switchd* switchd;
  pthread_t readDataplane_thread;

  TofinoRegister* hold_data;

  TofinoRegister* ring_pointer;
  TofinoRegister* ringbuffer[5];

  TofinoRegister* punch_counters;
  TofinoRegister* last_value;
  TofinoRegister* counter;
  TofinoRegister* raw_pointer;
  TofinoRegister* raw_storage;


  TofinoTables* tables;

  CpuPortCapture* cpu_port_capture;

  TofinoSwitchControl(bool sampling, bool clustering, bool kernel_pkt_mode, std::string mask);

  void initializeDataplaneInterfaces();
  void setupDataplane();
  std::string cpu_port_interface1();
  std::string cpu_port_interface2();
  void setupPort(uint64_t num, bf_port_speed_t speed);
  void sessionCompleteOperations();
  void setupRestartCapture(cb_t cb);
  void setupUnknownCapture(cb_t cb);


 private:
  bool sampling;
  bool clustering;
  bool kernel_pkt_mode;
};