#include "sampling.hpp"

Sampling::Sampling(TofinoSwitchControl* tsc) {
  this->tsc = tsc;
}


void Sampling::initSampling(json config) {
  this->primary_pos[0] = default_primary;
  this->primary_pos[1] = default_primary;

  tsc->tables->addStage1PortWithOutput((int)config["aggregation"]["recirc_port"], (int)config["aggregation"]["out_pipe1_port"]);

  tsc->tables->addStage1PortWithDrop(12);
  std::set<int> in_ports;
    for (auto& machine : config["aggregation"]["machines"].items()) 
    {
      nlohmann::json machine_object = machine.value();
      int cfg_machine_id = (int)machine_object.at("machine_id");
      for (auto& sensor : machine_object["sensors"].items())
      {
          nlohmann::json object = sensor.value();
          in_ports.insert((int)object.at("ingress_port"));
          if ((bool)object.at("is_phase_detection_sensor")) {
            LOG_F(INFO, "Sensor %d", (int)object.at("global_sensor_id"));
            tsc->tables->addCountZeroWithCount((int)object.at("global_sensor_id"), 0x00000000, 0xFFFFFFFF << (int)object.at("zero_noise_bits"), true, (int)object.at("global_sensor_id"), 0);
            tsc->tables->addCountZeroWithCount((int)object.at("global_sensor_id"), 0xFFFFFFFF, 0xFFFFFFFF << (int)object.at("zero_noise_bits"), true, (int)object.at("global_sensor_id"), 0);
            tsc->tables->addCountZeroWithCount((int)object.at("global_sensor_id"), 0x00000000, 0x00000000, false, (int)object.at("global_sensor_id"), 10);
          }

          LOG_F(INFO, "Sensor %d", (int)object.at("global_sensor_id"));
          tsc->tables->addPickup((int)object.at("global_sensor_id"), (int)config["aggregation"]["recirc_port"]);
          tsc->tables->addLeave((int)object.at("global_sensor_id"), (int)object.at("global_sensor_id"));
          tsc->tables->addCountUpOrPick((int)object.at("global_sensor_id"), (int)object.at("ingress_port"), 0xFF, (int)object.at("global_sensor_id"), 0xFF, false, 2, 0);
          tsc->tables->addCountUpOrPick((int)object.at("global_sensor_id"), (int)config["aggregation"]["recirc_port"], 0xFF, (int)object.at("global_sensor_id"), 0x00, true, 0, 0);
      }
        this->setConfig(config, cfg_machine_id);
    }
    setOuts(in_ports, (int)config["aggregation"]["data_collection_port"]);
}

void Sampling::setOuts(std::set<int> in_ports, int out_port) {
  tsc->tables->addStage1PortWithOutput(52, out_port);
  tsc->tables->addStage1PortWithOutput(60, out_port);

}

void Sampling::setOut(int port) {
  tsc->tables->addStage1PortWithOutputReset(53, port);
  tsc->tables->addStage1PortWithOutput(68, 60);
}



nlohmann::json Sampling::getMachineObject(json config, int machine_id) {
  for (auto& machine : config["aggregation"]["machines"].items())
  {
    nlohmann::json machine_object = machine.value();
    int cfg_machine_id = (int)machine_object.at("machine_id");
    if (machine_id == cfg_machine_id) {
      return machine_object;
    }
  }
  return 0;
}

void Sampling::setConfig(json config, int machineID) {
  nlohmann::json machine_object = getMachineObject(config, machineID);
  for (auto& sensor : machine_object["sensors"].items()) {
    nlohmann::json sensor_object = sensor.value();
    tsc->tables->setConfig((int)sensor_object.at("global_sensor_id"), (int)sensor_object.at("phase_change_delay"), this->primary_pos[machineID]);
  }
}

void Sampling::resetCounters() {
  LOG_F(WARNING, "Reset Counters");
  tsc->punch_counters->write(0,0);
  tsc->punch_counters->write(1,0);
  tsc->punch_counters->write(2,0);
  tsc->punch_counters->write(3,0);
  tsc->punch_counters->write(4,0);
}


std::vector<int> Sampling::getPhaseDetectionSensorIDs(json config, int machineID) {
  nlohmann::json machine_object = getMachineObject(config, machineID);
  std::vector<int> ids;
  for (auto& sensor : machine_object["sensors"].items()) {
    nlohmann::json sensor_object = sensor.value();
    if ((int)sensor_object.at("is_phase_detection_sensor")) {
      ids.push_back((int)sensor_object.at("global_sensor_id"));
      LOG_F(INFO, "Things %d", (int)sensor_object.at("global_sensor_id"));
    }
  }
  return ids;
}


json Sampling::readCounters(json config, int machineID) {
  nlohmann::json machine_object = getMachineObject(config, machineID);
  std::vector<int> ids = getPhaseDetectionSensorIDs(config, machineID);
  int length = (int) ids.size();
  uint32_t counters[length];
  uint32_t id_array[length];

  int primary_pos_local = -1;
  auto countersjson = json::array();


  double sum = 0;
  for (int i = 0; i < length; i++) {
    counters[i] = tsc->punch_counters->read(ids.back());
    id_array[i] = ids.back();
    if (id_array[i] == this->primary_pos[machineID]) {
      primary_pos_local = i;
    }
    ids.pop_back();
    sum += counters[i];
    LOG_F(INFO, "Counter %d, Value %d, MachineID %d", id_array[i], counters[i], machineID);
    json counter = {
            {"counter", id_array[i]},
            {"value", counters[i]},
    };
    countersjson.push_back(counter);

  }

  double distance[length];
  double min_val = -1;
  int min_id = -1;
  double max_val = -1;
  int max_id = -1;

  double min_dist_val = -1;
  int min_dist_id = -1;


  if (length >= 3) {
    // Identify absolute maximum and minimum counters to ignore them
    for (int i = 0; i < length; i++) {
      if (counters[i] > max_val) {
          max_val = counters[i];
          max_id = i;
      }
      if (counters[i] < min_val || min_val == -1) {
          min_val = counters[i];
          min_id = i;
      }
    }

    // If multiple sensors have the minimum or maximum value -> assume it is not an outlier
    int max_count = 0;
    int min_count = 0;
    for (int i = 0; i < length; i++) {
      if (max_val == counters[i]) {
        max_count += 1;
      }
      if (min_val == counters[i]) {
        min_count += 1;
      }
    }

    if (min_count > 1) {
      min_val = -1;
      min_id = -1;
    }
    if (max_count > 1) {
      max_val = -1;
      min_val = -1;
    }
  }

  bool primary_change = false;

  auto outliers = json::array();

  // Sensors that are definetly outliers, keep track of their behavior
  for (int i = 0; i < length; i++) {
    if (min_id == i || max_id == i) {
       this->outlier_series[i] += 1;
       this->not_outlier_series[i] = 0;
    } else {
       this->not_outlier_series[i] +=1;
       this->outlier_series[i] = 0;
    }
    json outlier = {
            {"id", id_array[i]},
            {"is_max", max_id == i},
            {"is_min", min_id == i},
            {"outlier_series", this->outlier_series[i]},
            {"not_outlier_series", this->not_outlier_series[i]},
    };
    outliers.push_back(outlier);

    if ((bool)config["aggregation"]["sensor_failover"] && id_array[i] == default_primary && primary_pos[0] != default_primary && not_outlier_series[i] > 3) {
      LOG_F(INFO, "Change primary sensor source, fall back to default");
      this->primary_pos[0] = id_array[i];
      this->tsc->tables->clearConfig();
      this->resetCounters();
      this->setConfig(config, machineID);
      primary_change = true;
    }
  }

  // Calculate average of all counter that are not the absolute minimum or maximum
  int considered_counters = 0;
  for (int i = 0; i < length; i++) {
    if (min_id != i && max_id != i) {
      sum += counters[i];
      considered_counters += 1;
    }
  }


  double avg = sum / considered_counters;

  // Find counter closest to average of those not discarded
  for (int i = 0; i < length; i++) {
    distance[i] = abs(avg - counters[i]);
    if (min_id != i && max_id != i) {
      if (distance[i] < min_dist_val || min_dist_val == -1) {
        min_dist_val = distance[i];
        min_dist_id = i;
      }
    }
  }


  LOG_F(INFO, "Avg %f", avg);
  LOG_F(INFO, "PrimaryPos %d, MinID %d, Distance Min_id %f, Distance to PrimaryPos %f", this->primary_pos[machineID], min_dist_id, distance[min_dist_id], distance[primary_pos_local]);
  LOG_F(INFO, "Counter %d: ID %d", counters[primary_pos_local], primary_pos_local);

  if ((bool)config["aggregation"]["sensor_failover"] && primary_change == false && primary_pos_local != -1) {

    if (abs(distance[min_dist_id] - distance[primary_pos_local]) > 3) { 
      this->primary_pos[0] = id_array[min_dist_id];
      LOG_F(INFO, "Change primary sensor source");
      this->tsc->tables->clearConfig();
      this->resetCounters();
      this->setConfig(config, machineID);
      primary_change = true;
    } else if (counters[primary_pos_local] > 5) {
      LOG_F(INFO, "Counter window exceeded");
      this->resetCounters();
    }
  }

  json agg_state = {
        {"counters", countersjson},
        {"primary_changed", primary_change},
        {"primary_sensor", this->primary_pos[0]},
        {"machine_id", machineID},
        {"outliers", outliers},
  };

  return agg_state;

}

json Sampling::readAllCounters(json config) {
  auto machine_counters = json::array();

  for (auto& machine : config["aggregation"]["machines"].items())
  {
    nlohmann::json machine_object = machine.value();
    int cfg_machine_id = (int)machine_object.at("machine_id");
    LOG_F(INFO, "Read MachineID %d", cfg_machine_id);
    json machine_json = this->readCounters(config, cfg_machine_id);
    machine_counters.push_back(machine_json);
  }
  return machine_counters;
}