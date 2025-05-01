// Based on https://laboratory.comsys.rwth-aachen.de/inp/teaching/rfc1920/p6-packet-generation/-/blob/master/Trace_Replay2/bfrt_cpp/src/trace_stream_loader.cpp

#include "tofino_tables.hpp"
#include <string>
#include <iostream>


TofinoTables::TofinoTables(Switchd* switchd, bool sampling, bool clustering) {
  this->switchd = switchd;
  initializeTables(sampling, clustering);
}

void TofinoTables::initializeTables(bool sampling, bool clustering) {
  if (sampling) {
    tables["Ingress1.selectPort"] = table_def{};
    tables["Ingress1.selectPort"].keys["ig_intr_md.ingress_port"] = 0;
    tables["Ingress1.selectPort"].actions["Ingress1.setPort"] = action_def({{ "port", 0 },});
    tables["Ingress1.selectPort"].actions["Ingress1.drop"] = action_def();

    tables["Ingress1.set_config"] = table_def{};
    tables["Ingress1.set_config"].keys["hdr.sensors$0.id"] = 0;
    tables["Ingress1.set_config"].actions["Ingress1.set_conf"] = action_def({{ "delay", 0 },{ "primary_pos", 0 },});

    tables["Ingress1.phase_detection.count_zero_table"] = table_def{};
    tables["Ingress1.phase_detection.count_zero_table"].keys["sensors$0.id"] = 0;
    tables["Ingress1.phase_detection.count_zero_table"].keys["sensors$0.value"] = 0;
    tables["Ingress1.phase_detection.count_zero_table"].keys["$MATCH_PRIORITY"] = 0;

    tables["Ingress1.phase_detection.count_zero_table"].actions["Ingress1.phase_detection.count_zeros_main"] = action_def({{ "pos", 0 },});
    tables["Ingress1.phase_detection.count_zero_table"].actions["Ingress1.phase_detection.count_nonzeros_main"] = action_def({{ "pos", 0 },});

    tables["Ingress1.aggregations.leave_or_pickup0"] = table_def{};
    tables["Ingress1.aggregations.leave_or_pickup0"].keys["port"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup0"].keys["sensors$0.id"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup0"].keys["$MATCH_PRIORITY"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup0"].actions["Ingress1.aggregations.pickup0"] = action_def({{ "output_id", 0 }, { "output_port", 0 },{ "reg_id", 0 },});
    tables["Ingress1.aggregations.leave_or_pickup0"].actions["Ingress1.aggregations.leave0"] = action_def({{ "reg_id", 0 },});

    tables["Ingress1.aggregations.leave_or_pickup1"] = table_def{};
    tables["Ingress1.aggregations.leave_or_pickup1"].keys["port"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup1"].keys["sensors$0.id"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup1"].keys["$MATCH_PRIORITY"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup1"].actions["Ingress1.aggregations.pickup1"] = action_def({{ "output_id", 0 }, { "output_port", 0 },{ "reg_id", 0 },});
    tables["Ingress1.aggregations.leave_or_pickup1"].actions["Ingress1.aggregations.leave1"] = action_def({{ "reg_id", 0 },});

    tables["Ingress1.aggregations.leave_or_pickup2"] = table_def{};
    tables["Ingress1.aggregations.leave_or_pickup2"].keys["port"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup2"].keys["sensors$0.id"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup2"].keys["$MATCH_PRIORITY"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup2"].actions["Ingress1.aggregations.pickup2"] = action_def({{ "output_id", 0 }, { "output_port", 0 },{ "reg_id", 0 },});
    tables["Ingress1.aggregations.leave_or_pickup2"].actions["Ingress1.aggregations.leave2"] = action_def({{ "reg_id", 0 },});

    tables["Ingress1.aggregations.leave_or_pickup3"] = table_def{};
    tables["Ingress1.aggregations.leave_or_pickup3"].keys["port"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup3"].keys["sensors$0.id"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup3"].keys["$MATCH_PRIORITY"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup3"].actions["Ingress1.aggregations.pickup3"] = action_def({{ "output_id", 0 }, { "output_port", 0 },{ "reg_id", 0 },});
    tables["Ingress1.aggregations.leave_or_pickup3"].actions["Ingress1.aggregations.leave3"] = action_def({{ "reg_id", 0 },});

    tables["Ingress1.aggregations.leave_or_pickup4"] = table_def{};
    tables["Ingress1.aggregations.leave_or_pickup4"].keys["port"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup4"].keys["sensors$0.id"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup4"].keys["$MATCH_PRIORITY"] = 0;
    tables["Ingress1.aggregations.leave_or_pickup4"].actions["Ingress1.aggregations.pickup4"] = action_def({{ "output_id", 0 }, { "output_port", 0 },{ "reg_id", 0 },});
    tables["Ingress1.aggregations.leave_or_pickup4"].actions["Ingress1.aggregations.leave4"] = action_def({{ "reg_id", 0 },});

    tables["Ingress1.phase_label"] = table_def{};
    tables["Ingress1.phase_label"].keys["meta.dummy"] = 0;
    tables["Ingress1.phase_label"].actions["Ingress1.set_phase_label"] = action_def({{ "label", 0 },});

  }

  if (clustering) {

    tables["Ingress2.phase_label"] = table_def{};
    tables["Ingress2.phase_label"].keys["meta.dummy"] = 0;
    tables["Ingress2.phase_label"].actions["Ingress2.set_phase_label"] = action_def({{ "label", 0 },});

    tables["Ingress2.send_back_to_aggregation"] = table_def{};
    tables["Ingress2.send_back_to_aggregation"].keys["ig_intr_md.ingress_port"] = 0;
    tables["Ingress2.send_back_to_aggregation"].actions["Ingress2.send"] = action_def({{ "port", 0 },});


    tables["Ingress2.restart_detection.set_restart_threshold"] = table_def{};
    tables["Ingress2.restart_detection.set_restart_threshold"].keys["meta.dummy"] = 0;
    tables["Ingress2.restart_detection.set_restart_threshold"].actions["Ingress2.restart_detection.set_restart_thresh"] = action_def({{ "threshold", 0 },});

    tables["Ingress2.failure_detection.check_0"] = table_def{};
    tables["Ingress2.failure_detection.check_0"].keys["value"] = 0;
    tables["Ingress2.failure_detection.check_0"].keys["sensors$4.id"] = 0;
    tables["Ingress2.failure_detection.check_0"].actions["Ingress2.failure_detection.in_range"] = action_def();
    tables["Ingress2.failure_detection.check_0"].actions["Ingress2.failure_detection.out_of_range"] = action_def();

    tables["Ingress2.failure_detection.compare_0"] = table_def{};
    tables["Ingress2.failure_detection.compare_0"].keys["difference"] = 0;
    tables["Ingress2.failure_detection.compare_0"].keys["sensors$0.id"] = 0;
    tables["Ingress2.failure_detection.compare_0"].actions["Ingress2.failure_detection.in_range"] = action_def();
    tables["Ingress2.failure_detection.compare_0"].actions["Ingress2.failure_detection.out_of_range"] = action_def();

  }

  for (auto& table : tables) {
    auto bf_status = switchd->bfrtInfo->bfrtTableFromNameGet(table.first, &table.second.table);
    assert(bf_status == BF_SUCCESS);

    bf_status = table.second.table->keyAllocate(&table.second.key_ref);
    assert(bf_status == BF_SUCCESS);

    // load key ids
    for (auto& key : table.second.keys) {
      bf_status = table.second.table->keyFieldIdGet(key.first, &key.second);
      assert(bf_status == BF_SUCCESS);
    }

    // load action ids
    for (auto& action : table.second.actions) {
      bf_status = table.second.table->actionIdGet(action.first, &action.second.id);
      assert(bf_status == BF_SUCCESS);

      bf_status = table.second.table->dataAllocate(action.second.id, &action.second.data_ref);
      assert(bf_status == BF_SUCCESS);

      // load action data ids
      for (auto& field : action.second.data_fields) {
        bf_status = table.second.table->dataFieldIdGet(field.first, action.second.id, &field.second);
        assert(bf_status == BF_SUCCESS);
      }
    }
  }
}

//Step1

void TofinoTables::addStage1PortWithOutputReset(uint16_t in_port, uint16_t out_port) {
  auto& table_ref = tables["Ingress1.selectPort"];
  auto bf_status = table_ref.table->tableClear(*switchd->session, switchd->device_target);
  assert(bf_status == BF_SUCCESS);

  key_list_entry result;

  bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["ig_intr_md.ingress_port"], static_cast<uint64_t>(in_port));
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions["Ingress1.setPort"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["port"], static_cast<uint64_t>(out_port));
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::addStage1PortWithOutput(uint16_t in_port, uint16_t out_port) {
  auto& table_ref = tables["Ingress1.selectPort"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["ig_intr_md.ingress_port"], static_cast<uint64_t>(in_port));
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions["Ingress1.setPort"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["port"], static_cast<uint64_t>(out_port));
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::addStage1PortWithDrop(uint16_t in_port) {
  auto& table_ref = tables["Ingress1.selectPort"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["ig_intr_md.ingress_port"], in_port);
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions["Ingress1.drop"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::clearConfig() {
  auto& table_ref = tables["Ingress1.set_config"];
  auto bf_status = table_ref.table->tableClear(*switchd->session, switchd->device_target);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::setConfig(uint32_t id, uint32_t delay, uint32_t primary_pos) {
  auto& table_ref = tables["Ingress1.set_config"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["hdr.sensors$0.id"], static_cast<uint64_t>(id));
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions["Ingress1.set_conf"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["delay"], static_cast<uint64_t>(delay));
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["primary_pos"], static_cast<uint64_t>(primary_pos));
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}


void TofinoTables::addCountZeroWithCount(uint32_t id, uint32_t value, uint32_t mask, bool count_zeros, uint32_t pos, uint32_t prio) {
  auto& table_ref = tables["Ingress1.phase_detection.count_zero_table"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);
  bf_status = result.key->setValue(table_ref.keys["sensors$0.id"], id);
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValueandMask(table_ref.keys["sensors$0.value"], value, mask);
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValue(table_ref.keys["$MATCH_PRIORITY"], prio);
  assert(bf_status == BF_SUCCESS);

  auto& action_ref_non = table_ref.actions["Ingress1.phase_detection.count_nonzeros_main"];
  auto& action_ref_zeros = table_ref.actions["Ingress1.phase_detection.count_zeros_main"];


  if (count_zeros) {
    bf_status = table_ref.table->dataReset(action_ref_zeros.id, action_ref_zeros.data_ref.get());
    assert(bf_status == BF_SUCCESS);
    bf_status = action_ref_zeros.data_ref->setValue(action_ref_zeros.data_fields["pos"], static_cast<uint64_t>(pos));
    assert(bf_status == BF_SUCCESS);
    bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref_zeros.data_ref);
    assert(bf_status == BF_SUCCESS);
  } else {
    bf_status = table_ref.table->dataReset(action_ref_non.id, action_ref_non.data_ref.get());
    assert(bf_status == BF_SUCCESS);
    bf_status = action_ref_non.data_ref->setValue(action_ref_non.data_fields["pos"], static_cast<uint64_t>(pos));
    assert(bf_status == BF_SUCCESS);
    bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref_non.data_ref);
    assert(bf_status == BF_SUCCESS);
     
  }
}

void TofinoTables::addPickup(int registerID, uint32_t port) {
  auto& table_ref = tables["Ingress1.aggregations.leave_or_pickup" + std::to_string(registerID)];
  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  table_ref.table->keyAllocate(&result.key);
  bf_status = result.key->setValueandMask(table_ref.keys["port"], port, 0xFF);
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValueandMask(table_ref.keys["sensors$0.id"], 0, 0x00);
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValue(table_ref.keys["$MATCH_PRIORITY"], 1);
  assert(bf_status == BF_SUCCESS);


  auto& action_ref = table_ref.actions["Ingress1.aggregations.pickup" + std::to_string(registerID)];
  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["output_id"], static_cast<uint64_t>(registerID));
  assert(bf_status == BF_SUCCESS);
  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["output_port"], static_cast<uint64_t>(0)); //TODO
  assert(bf_status == BF_SUCCESS);
  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["reg_id"], static_cast<uint64_t>(0)); 
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::addLeave(int registerID, uint32_t id) {
  std::string table = "Ingress1.aggregations.leave_or_pickup";
  table += std::to_string(registerID);
  auto& table_ref = tables[table];
  LOG_F(INFO, table.c_str());

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  table_ref.table->keyAllocate(&result.key);
  bf_status = result.key->setValueandMask(table_ref.keys["port"], 0, 0x00);
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValueandMask(table_ref.keys["sensors$0.id"], id, 0xFF);
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValue(table_ref.keys["$MATCH_PRIORITY"], 0);
  assert(bf_status == BF_SUCCESS);


  std::string action = "Ingress1.aggregations.leave";
  action += std::to_string(registerID);
  auto& action_ref = table_ref.actions[action];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);
  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["reg_id"], static_cast<uint64_t>(0)); 
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::addCountUpOrPick(int registerID, uint32_t port, uint32_t port_mask, uint32_t id, uint32_t id_mask, bool isPick, uint32_t prio, uint16_t reg_id) {
 
}

void TofinoTables::updateRestartThreshold(uint32_t threshold) {
  LOG_F(INFO, "Attempt to set RestartThreshold");
  auto& table_ref = tables["Ingress2.restart_detection.set_restart_threshold"];
  auto bf_status = table_ref.table->tableClear(*switchd->session, switchd->device_target);
  assert(bf_status == BF_SUCCESS);
  key_list_entry result;
  LOG_F(INFO, "Cleared Table");

  bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["meta.dummy"], 0);
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions["Ingress2.restart_detection.set_restart_thresh"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["threshold"], static_cast<uint64_t>(threshold));
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}


void TofinoTables::updateLabel(uint32_t label) {
  updateLabelForPipe(label, true);
  updateLabelForPipe(label, false);
}

void TofinoTables::updateLabelForPipe(uint32_t label, bool is_Ingress_2) {
  std::string table = "";
  std::string method = "";
  if (is_Ingress_2) {
    table = "Ingress2.phase_label";
    method = "Ingress2.set_phase_label";
  } else {
    table = "Ingress1.phase_label";
    method = "Ingress1.set_phase_label";
  }

  LOG_F(INFO, "Attempt LabelUpate");
  auto& table_ref = tables[table];
  auto bf_status = table_ref.table->tableClear(*switchd->session, switchd->device_target);
  assert(bf_status == BF_SUCCESS);
  key_list_entry result;

  bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["meta.dummy"], 0);
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions[method];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["label"], static_cast<uint64_t>(label));
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::setParams(uint16_t ring_size) {
  auto& table_ref = tables["Ingress2.set_params"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["meta.dummy"], 0);
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions["Ingress2.set_ring_size"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["ring_size"], static_cast<uint64_t>(ring_size));
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::addAggregationLoopBack(uint16_t port_in, uint16_t port_out) {
  auto& table_ref = tables["Ingress2.send_back_to_aggregation"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValue(table_ref.keys["ig_intr_md.ingress_port"], static_cast<uint64_t>(port_in));
  assert(bf_status == BF_SUCCESS);

  // load data
  auto& action_ref = table_ref.actions["Ingress2.send"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);

  bf_status = action_ref.data_ref->setValue(action_ref.data_fields["port"], static_cast<uint64_t>(port_out));
  assert(bf_status == BF_SUCCESS);

  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}

void TofinoTables::addThreshold(uint16_t id, uint32_t max, uint32_t min) {
  auto& table_ref = tables["Ingress2.failure_detection.check_0"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValueRange(table_ref.keys["value"], (min>>12), (max>>12));
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValue(table_ref.keys["sensors$4.id"], id);
  assert(bf_status == BF_SUCCESS);

  auto& action_ref = table_ref.actions["Ingress2.failure_detection.in_range"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);


  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}


void TofinoTables::addDifference(uint16_t from, uint16_t to, uint16_t difference) {
  auto& table_ref = tables["Ingress2.failure_detection.compare_0"];

  key_list_entry result;

  auto bf_status = table_ref.table->keyReset(table_ref.key_ref.get());
  assert(bf_status == BF_SUCCESS);

  // create new key object
  table_ref.table->keyAllocate(&result.key);

  bf_status = result.key->setValueRange(table_ref.keys["difference"], 0, difference); // TODO
  assert(bf_status == BF_SUCCESS);
  bf_status = result.key->setValue(table_ref.keys["sensors$0.id"], 0); // TODO
  assert(bf_status == BF_SUCCESS);

  auto& action_ref = table_ref.actions["Ingress2.failure_detection.in_range"];

  bf_status = table_ref.table->dataReset(action_ref.id, action_ref.data_ref.get());
  assert(bf_status == BF_SUCCESS);


  bf_status = table_ref.table->tableEntryAdd(*switchd->session, switchd->device_target, *result.key, *action_ref.data_ref);
  assert(bf_status == BF_SUCCESS);
}