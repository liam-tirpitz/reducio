// Based on https://laboratory.comsys.rwth-aachen.de/theses-kunze/2020-ma-sokolowski-code

#include "tofino_switch_control.hpp"

TofinoSwitchControl::TofinoSwitchControl(bool sampling, bool clustering, bool kernel_pkt_mode, std::string mask) {
  this->sampling = sampling;
  this->clustering = clustering;
  this->kernel_pkt_mode = kernel_pkt_mode;
  std::string name;
  if (sampling && clustering) {
    name = "combinedcluster-" + mask;
  } else if (sampling) {
    name = "sampling";
  } else if (clustering) {
    name = "clustering";
  }

  switchd = new Switchd(name.c_str(), this->kernel_pkt_mode);
  switchd->start();
  LOG_F(INFO, "BFRT Switchd initialization finished");
}

void TofinoSwitchControl::setupUnknownCapture(cb_t cb) {
  std::string cpu_interface = cpu_port_interface1();
  cpu_port_capture = new CpuPortCapture(cpu_interface);

  cpu_port_capture->setCallback(cb, this);

  LOG_F(INFO, "switch %d is sniffing for unknown data points on %s", 0,
        cpu_interface.c_str());
}

void TofinoSwitchControl::setupRestartCapture(cb_t cb) {
  std::string cpu_interface = cpu_port_interface2();
  cpu_port_capture = new CpuPortCapture(cpu_interface);

  cpu_port_capture->setCallback(cb, this);

  LOG_F(INFO, "switch %d is sniffing for restarts on %s", 0,
        cpu_interface.c_str());
}

void TofinoSwitchControl::initializeDataplaneInterfaces() {
  tables = new TofinoTables(switchd, sampling, clustering);

  if (sampling) {
    hold_data = new TofinoRegister("Ingress1.aggregations.data_storage0", switchd);
    punch_counters = new TofinoRegister("Ingress1.phase_detection.count_detected_punches", switchd);

  }
  if (clustering) {
    ring_pointer = new TofinoRegister("Ingress2.ring_pointer", switchd);
    ringbuffer[0] = new TofinoRegister("Ingress2.ringbuffer0", switchd);
    ringbuffer[1] = new TofinoRegister("Ingress2.ringbuffer1", switchd);
    ringbuffer[2] = new TofinoRegister("Ingress2.ringbuffer2", switchd);
    ringbuffer[3] = new TofinoRegister("Ingress2.ringbuffer3", switchd);
    ringbuffer[4] = new TofinoRegister("Ingress2.ringbuffer4", switchd);


    last_value = new TofinoRegister("Ingress2.last_value", switchd);
    counter = new TofinoRegister("Ingress2.counter0", switchd);
    raw_pointer = new TofinoRegister("Ingress2.raw_pointer", switchd);
    raw_storage = new TofinoRegister("Ingress2.raw_storage", switchd);

  }

  LOG_F(INFO, "Initialized dataplane interfaces");
  sessionCompleteOperations();
}

void TofinoSwitchControl::setupPort(uint64_t num, bf_port_speed_t speed) {
  bf_status_t status;

  bf_pal_front_port_handle_t port_handle;
  status = bf_pm_port_dev_port_to_front_panel_port_get(
      switchd->device_target.dev_id, num, &port_handle);
  CHECK_F(status == BF_SUCCESS, "Failed to acquire port handle for port num %d",
          num);

  status = bf_pm_port_add(switchd->device_target.dev_id, &port_handle, speed,
                          BF_FEC_TYP_NONE);
  CHECK_F(status == BF_SUCCESS, "Failed to add port %d", num);

  status = bf_pm_port_enable(switchd->device_target.dev_id, &port_handle);
  CHECK_F(status == BF_SUCCESS, "Failed to enable port %d", num);
}

void TofinoSwitchControl::setupDataplane() {
  bf_status_t bf_status;
  bf_status = bf_tm_port_cpuport_set(switchd->device_target.dev_id, 192);
  CHECK_F(bf_status == BF_SUCCESS, "Failed to configure CPU port");
  LOG_F(INFO, "Activated CPU port, port number %d", 192);

  setupPort(52, BF_SPEED_25G);
  setupPort(53, BF_SPEED_25G);

  setupPort(140, BF_SPEED_25G);
  setupPort(141, BF_SPEED_25G);

  setupPort(60, BF_SPEED_100G);
  setupPort(184, BF_SPEED_100G);

  setupPort(24, BF_SPEED_100G);
  setupPort(8, BF_SPEED_100G);

  setupPort(64, BF_SPEED_10G);
  setupPort(66, BF_SPEED_10G);

  sessionCompleteOperations();
}

void TofinoSwitchControl::sessionCompleteOperations() {
  bf_status_t bf_status;
  bf_status = switchd->session->sessionCompleteOperations();
  assert(bf_status == BF_SUCCESS);
}

std::string TofinoSwitchControl::cpu_port_interface1() {
  return "ens1";
}

std::string TofinoSwitchControl::cpu_port_interface2() {
  return "eth1";
}