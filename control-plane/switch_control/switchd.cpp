// Based on https://laboratory.comsys.rwth-aachen.de/theses-kunze/2020-ma-sokolowski-code

// #define _GLIBCXX_USE_CXX11_ABI 0

#include "switchd.hpp"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstdlib>
#include <functional>
#include <thread>
#include <iostream>



Switchd::Switchd(const char *p4_name, bool kernel_pkt_mode) {
  this->p4_name = p4_name;
  this->kernel_pkt_mode = kernel_pkt_mode;

  switchd_ctx = (bf_switchd_context_t *) malloc(sizeof(bf_switchd_context_t));

  if (switchd_ctx == NULL) {
    LOG_F(ERROR, "Cannot Allocate switchd context!");
    exit(1);
  }
  memset(switchd_ctx, 0, sizeof(bf_switchd_context_t));

  const char *env_sde_install = getenv("SDE_INSTALL");
  switchd_ctx->install_dir = strdup(env_sde_install);
  LOG_F(INFO, "Install Dir: %s\n", switchd_ctx->install_dir);

  char* sde_path = std::getenv("SDE");
  LOG_F(INFO, "The SDE path is: %s \n",sde_path);
  if (sde_path == nullptr) {
    LOG_F(ERROR, "$SDE variable is not set\n");
    exit(0);
  }


  switchd_ctx->conf_file = (char *)malloc(256);
    sprintf(switchd_ctx->conf_file,
          "%s/install/share/p4/targets/tofino/%s.conf",
          sde_path, p4_name);
  LOG_F(INFO, "Conf-file : %s\n", switchd_ctx->conf_file);
}

bf_status_t Switchd::start() {
  switchd_ctx->dev_sts_thread = true;
  switchd_ctx->dev_sts_port = 7777;


  switchd_ctx->init_mode = BF_DEV_INIT_COLD;


  // REMARK: Kernel pkt only works with tofnio hardware, not emulator
  switchd_ctx->kernel_pkt = this->kernel_pkt_mode;

  bf_status_t status;
  status = bf_switchd_lib_init(switchd_ctx);
  CHECK_F(status == BF_SUCCESS, "switchd lib init failed");

  memset(&device_target, 0, sizeof(device_target));
  device_target.dev_id = 0;
  device_target.pipe_id = ALL_PIPES;

  auto &devMgr = bfrt::BfRtDevMgr::getInstance();
  status = devMgr.bfRtInfoGet(device_target.dev_id, p4_name, &bfrtInfo);
  std::cout << status;
  CHECK_F(status == BF_SUCCESS, "Device Manager Instance failed");


  // Create a session object
  session = bfrt::BfRtSession::sessionCreate();
  CHECK_F(session != nullptr, "Cannot establish BfRt session.");


  return BF_SUCCESS;

}

void Switchd::joinContextThreads() {
  pthread_join(switchd_ctx->tmr_t_id, NULL);
  pthread_join(switchd_ctx->dma_t_id, NULL);
  pthread_join(switchd_ctx->int_t_id, NULL);
  pthread_join(switchd_ctx->pkt_t_id, NULL);
  pthread_join(switchd_ctx->port_fsm_t_id, NULL);
  pthread_join(switchd_ctx->drusim_t_id, NULL);
  pthread_join(switchd_ctx->accton_diag_t_id, NULL);
  for (size_t agent_idx = 0; agent_idx < BF_SWITCHD_MAX_AGENTS; agent_idx++) {
    if (switchd_ctx->agent_t_id[agent_idx] != 0) {
      pthread_join(switchd_ctx->agent_t_id[agent_idx], NULL);
    }
  }
}
