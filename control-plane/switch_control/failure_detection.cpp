#include "failure_detection.hpp"
#include <loguru.hpp>
#include "cpu_port_capture.hpp"

FailureDetection::FailureDetection(TofinoSwitchControl* tsc) {
  LOG_F(INFO, "Init FailureDetection");
  this->tsc = tsc;
}

 


void FailureDetection::setThresholds(json config) {
  if ((bool)config["aggregation"]["thresholds_enabled"] == true) {
    for (auto& machine : config["aggregation"]["machines"].items())
      {
        nlohmann::json machine_object = machine.value();
        for (auto& sensor : machine_object["sensors"].items()) {
          nlohmann::json sensor_object = sensor.value();
          int id = (int)sensor_object.at("global_sensor_id");
          int max = (int)sensor_object.at("max");
          int min = (int)sensor_object.at("min");
          tsc->tables->addThreshold(id, max, min);
        }
      }
  } else {
      LOG_F(INFO, "Threshold Comparisons disabled");
  }
}

void FailureDetection::setDifferences(json config) {
  if ((bool)config["aggregation"]["differences_enabled"] == true) {
    for (auto& machine : config["aggregation"]["machines"].items())
    {
      nlohmann::json machine_object = machine.value();
      for (auto& difference : machine_object["differences"].items()) {
        nlohmann::json diff_object = difference.value();
        int from = (int)diff_object.at("from");
        int to = (int)diff_object.at("to");
        int diff = (int)diff_object.at("difference");
      }
    }
  } else {
    LOG_F(INFO, "Difference Comparisons disabled");
  }
}

int checkSingleSensorEvent(json config, sniff_sensor* sensor) {
  for (auto& machine : config["aggregation"]["machines"].items())
  {
    nlohmann::json machine_object = machine.value();
    for (auto& config_sensor : machine_object["sensors"].items()) {
      nlohmann::json sensor_object = config_sensor.value();
      int id = (int)sensor_object.at("global_sensor_id");
      int max = (int)sensor_object.at("max");
      int min = (int)sensor_object.at("min");
      if (id == sensor->sensor_id) {
        if (max < sensor->value || min > sensor->value) {
          LOG_F(WARNING, "Sensor %d Threshold Violation %d:%d:%d", id, min, sensor->value, max);
          return sensor->value;
        }
      }
      
    }
  }
  return -1;
}


int checkRelationEvent(json config, sniff_sensor* sensor0, sniff_sensor* sensor1) {
  for (auto& machine : config["aggregation"]["machines"].items())
  {
    nlohmann::json machine_object = machine.value();
    for (auto& difference : machine_object["differences"].items()) {
      nlohmann::json diff_object = difference.value();
      int from = (int)diff_object.at("from");
      int to = (int)diff_object.at("to");
      int diff_conf = (int)diff_object.at("difference");
      if ((from == sensor0->sensor_id && to == sensor1->sensor_id) || (from == sensor1->sensor_id && to == sensor0->sensor_id))  {

        int diff_actual = 0;
        if (sensor0->value > sensor1->value) {
          diff_actual = sensor0->value - sensor1->value;
        } else {
          diff_actual = sensor1->value - sensor0->value;
        }

        if (diff_actual > diff_conf) {
          return 1;
        }
      }

    }

  }
  return -1;
}


void FailureDetection::handleOutofBoundsEvent(json config, sniff_sensor* sensor0, sniff_sensor* sensor1, sniff_sensor* sensor2, sniff_sensor* sensor3, sniff_sensor* sensor4, sniff_sensor* sensor5) {

  int eventSensor0 = checkSingleSensorEvent(config, sensor0);
  int eventSensor1 = checkSingleSensorEvent(config, sensor1);
  int eventSensor2 = checkSingleSensorEvent(config, sensor2);
  int eventSensor3 = checkSingleSensorEvent(config, sensor3);
  int eventSensor4 = checkSingleSensorEvent(config, sensor4);
  int eventSensor5 = checkSingleSensorEvent(config, sensor5);

  
  int relation01 = checkRelationEvent(config, sensor0, sensor1);

  LOG_F(WARNING, "SingleEvents %d:%d:%d:%d:%d", eventSensor0, eventSensor1, eventSensor2, eventSensor3, eventSensor4, eventSensor5);
  LOG_F(INFO, "Relation %d", relation01);


}




