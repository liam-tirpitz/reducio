// Based on https://laboratory.comsys.rwth-aachen.de/theses-kunze/2020-ma-sokolowski-code

#pragma once

#include <bf_rt/bf_rt_common.h>
#include <loguru.hpp>
#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_init.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_data.hpp>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_operations.hpp>
extern "C" {
#include <bf_switchd/bf_switchd.h>
}

#define ALL_PIPES 0xffff
#define PIPE_ID 1

using namespace bfrt;

class Switchd {
 public:
  bf_switchd_context_t* switchd_ctx;

  bf_rt_target_t device_target;
  const bfrt::BfRtInfo* bfrtInfo;
  std::shared_ptr<bfrt::BfRtSession> session;

  const char* p4_name;

  Switchd(const char* p4_name, bool kernel_pkt_mode);
  bf_status_t start();
  void joinContextThreads();

 private:
  bool kernel_pkt_mode;
};
