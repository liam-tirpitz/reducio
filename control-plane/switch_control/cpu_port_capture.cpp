/**
 * Based on https://laboratory.comsys.rwth-aachen.de/theses-kunze/2020-ma-sokolowski-code
 * packet decomposition concept inspired by https://www.tcpdump.org/pcap.html
 * raw socket and binding inspired by pcap
 **/
#include "cpu_port_capture.hpp"

#include <linux/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <loguru.hpp>

uint32_t reverse_bytes_32(uint32_t value) {
  return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
  (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}

uint16_t reverse_bytes_16(uint16_t value) {
  return (value & 0x00FFU) << 8 | (value & 0xFF00U) >> 8;
}


void *capture_thread_wrapper(void *arg) {
  CpuPortCapture *capture = (CpuPortCapture *)arg;
  capture->capture_thread_func();
  return NULL;
}

CpuPortCapture::CpuPortCapture(std::string interface_name) {
  socket_handle = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  CHECK_F(socket_handle > 0, "Failed to create socket");

  struct ifreq ifr;

  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, interface_name.c_str());

  CHECK_F(ioctl(socket_handle, SIOCGIFINDEX, &ifr) != -1,
          "could not get interface index for %s", interface_name.c_str());

  int interface_index = ifr.ifr_ifindex;
  CHECK_F(interface_index > 0,
          "Acquired presumably invalid interface index: %d", interface_index);

  struct sockaddr_ll sll;
  int ret, err;
  socklen_t errlen = sizeof(err);

  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = interface_index;
  sll.sll_protocol = 0;

  ret = bind(socket_handle, (struct sockaddr *)&sll, sizeof(sll));
  CHECK_F(ret != -1, "error binding socket to interface");

  CHECK_F(getsockopt(socket_handle, SOL_SOCKET, SO_ERROR, &err, &errlen) != -1,
          "error checking for network errors");

  CHECK_F(err != ENETDOWN, "Cpu interface is down");
  CHECK_F(err < 1, "some bind error");

  struct packet_mreq mreq;
  memset(&mreq, 0, sizeof(mreq));
  mreq.mr_ifindex = interface_index;
  mreq.mr_type = PACKET_MR_PROMISC;
  mreq.mr_alen = 6;

  CHECK_F(setsockopt(socket_handle, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
                     (void *)&mreq, (socklen_t)sizeof(mreq)) >= 0,
          "could not activate promiscous mode");

  ret = pthread_create(&pcap_thread, NULL, capture_thread_wrapper, this);
  CHECK_F(ret == 0, "Failed starting thread: pcap");
}

void CpuPortCapture::setCallback(cb_t cb, void *arg) {
  callback_arg = arg;
  callback = cb;
}

// header structs, the buffer is just mapped into these in order to extract the
// fields
struct sniff_ethernet {
  u_char ether_dhost[ETHER_ADDR_LEN];
  u_char ether_shost[ETHER_ADDR_LEN];
  u_short ether_type;
};

struct sniff_ip {
  u_char ip_vhl;
  u_char ip_tos;
  u_short ip_len;
  u_short ip_id;
  u_short ip_off;
  u_char ip_ttl;
  u_char ip_p;
  u_short ip_sum;
  u_int32_t ip_src;
  u_int32_t ip_dst;
};

struct sniff_udp {
  u_short src_port;
  u_short dst_port;
  u_short length;
  u_short checksum;
};

#define IP_HL(ip) (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip) (((ip)->ip_vhl) >> 4)

void CpuPortCapture::capture_thread_func() {
  LOG_F(INFO, "Starting cpu capture thread");

  unsigned char *buffer = (unsigned char *)malloc(65536);  // to receive data

  while (1) {
    memset(buffer, 0, 65536);

    int len = recv(socket_handle, buffer, 65536, 0);
    if (len == 0) {
      LOG_F(ERROR, "cpu capture socket closed");
      return;
    }

    if (len < 0) {
      LOG_F(ERROR,
            "error when receiving from cpu capture socket %d,"
            "exiting capture loop",
            strerror(len));
      return;
    }

    const struct sniff_ethernet *ethernet;
    const struct sniff_ip *ip;
    const struct sniff_udp *udp;
    struct sniff_label *label;
    struct sniff_sensor *sensor0;
    struct sniff_sensor *sensor1;
    struct sniff_sensor *sensor2;
    struct sniff_sensor *sensor3;
    struct sniff_sensor *sensor4;
    struct sniff_sensor *sensor5;


    const int size_udp = 8;
    const int size_sensor = 9;
    const int size_label = 1;

    label = (struct sniff_label *) (buffer);
    ethernet = (struct sniff_ethernet *)(buffer + size_label);


    uint16_t ether_type = ethernet->ether_type;
    ether_type = (ether_type & 0x00FFU) << 8 | (ether_type & 0xFF00U) >> 8;
    if (ether_type != 0x0800) {
      LOG_F(WARNING, "capture system received non ipv4 header, ethertype: %08x",
            ether_type);
      continue;
    }

    ip = (struct sniff_ip *)(buffer + size_label + sizeof(sniff_ethernet));

    u_int size_ip;
    size_ip = IP_HL(ip) * 4;
    if (size_ip < 20) {
      LOG_F(WARNING,
            "capture system received invalid IP header length: %u bytes",
            size_ip);
      continue;
    }

    udp = (struct sniff_udp *)(buffer + + size_label + sizeof(sniff_ethernet) + size_ip);

    sensor0 = (struct sniff_sensor *) (buffer + + size_label + sizeof(sniff_ethernet) + size_ip + size_udp);
    sensor0->sensor_id = reverse_bytes_16(sensor0->sensor_id);
    sensor0->value = reverse_bytes_32(sensor0->value);
    sensor0->counter = reverse_bytes_16(sensor0->counter);

    sensor1 = (struct sniff_sensor *) (buffer + sizeof(sniff_ethernet) + size_ip + size_udp + size_label + 1 * size_sensor);
    sensor1->sensor_id = reverse_bytes_16(sensor1->sensor_id);
    sensor1->value = reverse_bytes_32(sensor1->value);
    sensor1->counter = reverse_bytes_16(sensor1->counter);

    sensor2 = (struct sniff_sensor *) (buffer + sizeof(sniff_ethernet) + size_ip + size_udp + size_label + 2 * size_sensor);
    sensor2->sensor_id = reverse_bytes_16(sensor2->sensor_id);
    sensor2->value = reverse_bytes_32(sensor2->value);
    sensor2->counter = reverse_bytes_16(sensor2->counter);

    sensor3 = (struct sniff_sensor *) (buffer + sizeof(sniff_ethernet) + size_ip + size_udp + size_label + 3 * size_sensor);
    sensor3->sensor_id = reverse_bytes_16(sensor3->sensor_id);
    sensor3->value = reverse_bytes_32(sensor3->value);
    sensor3->counter = reverse_bytes_16(sensor3->counter);

    sensor4 = (struct sniff_sensor *) (buffer + sizeof(sniff_ethernet) + size_ip + size_udp + size_label + 4 * size_sensor);
    sensor4->sensor_id = reverse_bytes_16(sensor4->sensor_id);
    sensor4->value = reverse_bytes_32(sensor4->value);
    sensor4->counter = reverse_bytes_16(sensor4->counter);

    sensor5 = (struct sniff_sensor *) (buffer + sizeof(sniff_ethernet) + size_ip + size_udp + size_label + 5 * size_sensor);
    sensor5->sensor_id = reverse_bytes_16(sensor5->sensor_id);
    sensor5->value = reverse_bytes_32(sensor5->value);
    sensor5->counter = reverse_bytes_16(sensor5->counter);


    if (callback != NULL) {
      callback(label, sensor0, sensor1, sensor2, sensor3, sensor4, sensor5, callback_arg);
    }
  }
}