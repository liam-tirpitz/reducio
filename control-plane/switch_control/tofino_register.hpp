// Based on https://laboratory.comsys.rwth-aachen.de/theses-kunze/2020-ma-sokolowski-code

#pragma once
#include <loguru.hpp>

#include "switchd.hpp"

class TofinoRegister {
 private:
  Switchd* switchd;
  const BfRtTable* table;

  bf_rt_id_t reg_index_key_id;
  bf_rt_id_t data_id;

 public:
  TofinoRegister(std::string register_name, Switchd* switchd);
  uint64_t read(uint64_t index);
  void write(uint64_t index, uint64_t value);
};