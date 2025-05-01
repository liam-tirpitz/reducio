#pragma once
#include <loguru.hpp>

#include "switchd.hpp"

struct key_list_entry {
  std::unique_ptr<bfrt::BfRtTableKey> key;
};

struct action_def {
  bf_rt_id_t id;
  std::unique_ptr<bfrt::BfRtTableData> data_ref;
  std::map<std::string, bf_rt_id_t> data_fields;

  action_def(std::map<std::string, bf_rt_id_t> fields)
          : data_fields(fields)
  {}

  action_def() {};
};

struct table_def {
  const bfrt::BfRtTable* table;

  std::unique_ptr<bfrt::BfRtTableKey> key_ref;

  std::map<std::string, bf_rt_id_t> keys;
  std::map<std::string, action_def> actions;
};

class TofinoTables {
 private:
  Switchd* switchd;
  std::map<std::string, table_def> tables;

 public:
  TofinoTables(Switchd* switchd, bool sampling, bool clustering);
  void initializeTables(bool sampling, bool clustering);

  //Step1
  void addStage1PortWithOutputReset(uint16_t in_port, uint16_t out_port);
  void addStage1PortWithOutput(uint16_t in_port, uint16_t out_port);
  void addStage1PortWithDrop(uint16_t in_port);
  void clearConfig();
  void setConfig(uint32_t id, uint32_t delay, uint32_t primary_pos);
  void addCountZeroWithCount(uint32_t id, uint32_t value, uint32_t mask, bool count_zeros, uint32_t pos, uint32_t prio);
  void addPickup(int registerID, uint32_t port);
  void addLeave(int registerID, uint32_t id);
  void addCountUpOrPick(int registerID, uint32_t port, uint32_t port_mask, uint32_t id, uint32_t id_mask, bool isPick, uint32_t prio, uint16_t reg_id);
  void addPickCount(int registerID, uint32_t port, uint32_t port_mask, uint32_t id, uint32_t id_mask, uint16_t reg_id);
  void updateLabelForPipe(uint32_t label, bool is_Ingress_2);

  //Step2
  void updateLabel(uint32_t label);
  void updateRestartThreshold(uint32_t threshold);
  void setParams(uint16_t ring_size);
  void addAggregationLoopBack(uint16_t port_in, uint16_t port_out);
  void addThreshold(uint16_t id, uint32_t max, uint32_t min);
  void addDifference(uint16_t from, uint16_t to, uint16_t difference);

};