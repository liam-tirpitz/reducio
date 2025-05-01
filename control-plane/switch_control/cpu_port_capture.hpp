#pragma once

#include <pcap.h>
#include <string>
#include <arpa/inet.h>

#pragma pack(push, 1)

struct sniff_label {
  unsigned char label;
};


struct sniff_sensor {
  unsigned short sensor_id;
  unsigned int value;
  unsigned short counter;
  unsigned char aggregation_type;
};
#pragma pack(pop)

typedef void (*cb_t)(sniff_label *label, sniff_sensor *sensor0, sniff_sensor *sensor1, sniff_sensor *sensor2, sniff_sensor *sensor3, sniff_sensor *sensor4, sniff_sensor *sensor5, void* arg);

class CpuPortCapture {
 private:
  int socket_handle;

  pthread_t pcap_thread;
  pcap_t* pcap_handle;

 public:
  cb_t callback;
  void* callback_arg;

  CpuPortCapture(std::string interface_name);
  void setCallback(cb_t cb, void* arg);

  void capture_thread_func();
};