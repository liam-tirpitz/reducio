#include "fine_blanking.hpp"
#include <math.h>
#include <iostream>
#include <cmath>
#include <numeric>
#include <fstream>
#include <loguru.hpp>
#include <chrono>
#include <filesystem>

using namespace std::chrono;

FineBlanking::FineBlanking(TofinoSwitchControl* tsc, int stable_std_threshold, uint32_t restart_threshold, int window, int ring_size, int gradient_window, bool enable_gradient_detection, std::chrono::system_clock::time_point  start_time, json config) {
  LOG_F(INFO, "Init FineBlanking");
  this->filename = "combined.csv";
  this->filename_raw = "raw.csv";
  this->start_time = start_time;
  this->gradient_detection_enabled = enable_gradient_detection;
  this->tsc = tsc;
  this->stable_std_threshold = stable_std_threshold;
  this->ring_size = ring_size;
  this->window = window;
  this->gradient_window = gradient_window;
  current_state = ramp_up;
  this->initFile(this->filename, "sensor,grad,stdev,phase,time,epoch_time,ring");
  this->initFile(this->filename_raw, "time,epoch_time,value,position");
  this->config = config;

  this->setRestartThreshold(restart_threshold);
  if (enable_gradient_detection) {
    this->stableSinceRestart = false;
    tsc->tables->updateLabel(44);
    LOG_F(INFO, "Assume Ramp-up Start");
  } else {
    this->stableSinceRestart = true;
    tsc->tables->updateLabel(42);
    LOG_F(INFO, "Assume Stable start");

  }
  this->samplingClass = new Sampling(tsc);

  tsc->tables->addAggregationLoopBack(140, 184);
  tsc->tables->addAggregationLoopBack(141, 184);
}

 std::vector<int> FineBlanking::getWindow(int size, int sensor_index) {
    auto n = 0;
    std::vector<int> elements;
    for (std::list<DataElement>::reverse_iterator it = local_ring[sensor_index].rbegin(); it != local_ring[sensor_index].rend(); ++it)
    {
      int repeat = it->repeat;
      int value = it->value;
      n += repeat;
      int difference = n - size;
      if (difference > 0) {
        n = size;
        repeat = repeat - difference;
      }
      for (int i = 0; i < repeat; i++) {    
        elements.push_back(value);
      }
      if (n >= size) {
        break;
      }
    }
    return elements;
}

nlohmann::json FineBlanking::getMachineObject(json config, int machine_id) {
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

std::vector<int> FineBlanking::getPhaseDetectionSensorIDs(json config, int machineID) {
  nlohmann::json machine_object = getMachineObject(config, machineID);
  std::vector<int> ids;
  for (auto& sensor : machine_object["sensors"].items()) {
    nlohmann::json sensor_object = sensor.value();
    if ((int)sensor_object.at("is_stability_detection_sensor")) {
      ids.push_back((int)sensor_object.at("global_sensor_id"));
    }
  }
  return ids;
}



float FineBlanking::rolling_std(std::vector<int> elements) {
  auto n = elements.size();

  if (n == 0) return 0;
  double sum = std::accumulate(elements.begin(), elements.end(), 0.0);
  double mean = sum / n;
  std::vector<double> diff(n);
  std::transform(elements.begin(), elements.end(), diff.begin(), [mean](double x) { return x - mean; });
  double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
  double stdev = std::sqrt(sq_sum / n);
  LOG_F(INFO, "Mean %f, Std %f (over %ld values).  ", mean, stdev, n);
  return stdev;
}

// Based on https://www.codesansar.com/numerical-methods/linear-regression-method-using-cpp-output.htm
double FineBlanking::gradient(std::vector<int> elements)
{
  int n = elements.size();
  long sumX = 0;
  long sumY = 0;
  long sumX2 = 0;
  long sumXY = 0;
  double m = 0;
  double b = 0;

  for(int i = 0; i < n; i++)
  {
    int posx = (n - i);
    //LOG_F(INFO, "Add %d:%d", posx, elements[i]);
    sumX = sumX + posx;
    sumX2 = sumX2 + posx*posx;
    sumY = sumY + elements[i];
    sumXY = sumXY + posx*elements[i];
  }
  if (n > 0) {
    long divisor = n*sumX2-sumX*sumX;
    if (divisor != 0) {
      m = (n*sumXY-sumX*sumY) / divisor;
      b = (sumY - m*sumX)/n;
    }
  }
  return m;
}

void FineBlanking::resetRing(unsigned int value, int sensor_index) {
  LOG_F(WARNING, "Reset Ring");
  this->tsc->ring_pointer->write(0, 0);
  this->tsc->counter->write(0, 1);
  this->tsc->ringbuffer[sensor_index]->write(0, value);
  for (int i = 1; i < this->ring_size; i++) {    
        this->tsc->ringbuffer[sensor_index]->write(i, 0);
          this->tsc->counter->write(i, 0);

  }



}


void FineBlanking::updateFromRing(TofinoSwitchControl* tsc, int sensor_index) {
  uint32_t previous_pointer = this->last_pointer[sensor_index];
  LOG_F(WARNING, "%d: POINTER: %d", sensor_index, previous_pointer);
  this->last_pointer[sensor_index] =  tsc->ring_pointer->read(0);
  DataElement last = local_ring[sensor_index].back();
  std::list<DataElement> new_data;
  if (previous_pointer > this->last_pointer[sensor_index]) {
     LOG_F(INFO, "Add Modulo Magic here");
  }
  while (previous_pointer <= this->last_pointer[sensor_index]) {
    int value =  tsc->ringbuffer[sensor_index]->read(previous_pointer);
    int count =  tsc->counter->read(previous_pointer);

    DataElement data = {value, previous_pointer, count};
    if (data.position == last.position && local_ring[sensor_index].size() > 0) {
       local_ring[sensor_index].pop_back();
    }
    local_ring[sensor_index].push_back(data);
    while (local_ring[sensor_index].size() > ring_size) {
      local_ring[sensor_index].pop_front();
    }
    LOG_F(INFO, "%d::Ring at: %d: is :%d (%d repeats) ", sensor_index, previous_pointer, value, count);
    new_data.push_back(data);
    previous_pointer = previous_pointer + 1;
  }
  this->new_data[sensor_index] = new_data;
}

void FineBlanking::print_current_ring() {


}



json FineBlanking::determine_current_state() {
  std::vector<int> ids = getPhaseDetectionSensorIDs(this->config, 0);
  int length = (int) ids.size();

  auto states = json::array();
  std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();  
  std::chrono::milliseconds current_epoch_time = duration_cast< milliseconds >(current_time.time_since_epoch());
  int time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count(); 
  std::string epoch_time = std::to_string(current_epoch_time.count());

  std::list<DataElement> global_counts = this->new_data[3];
  DataElement first = global_counts.front();
  DataElement last = global_counts.back();
  int global_count = last.value;

  for (int i = 0; i < length; i++) {
    int id = ids[i];
    std::vector<int> elements = this->getWindow(this->window, id);
    double stdev = rolling_std(elements);
    std::vector<int> gradelements = this->getWindow(this->gradient_window, id);
    double grad = gradient(gradelements);
    LOG_F(INFO, "%d:Rolling StandardDeviation. %f", id, stdev);
    LOG_F(INFO, "%d:Rolling Gradient. %f", id, grad);
    if ((grad > 0 && this->states[id] == ramp_up) || elements.size() < this->window) {
      // Stay in Ramp-Up
    } else if (stdev < this->stable_std_threshold) {
      this->states[id] = stable;
    } else if ((this->states[id] != ramp_up || stableSinceRestart)) {
      this->states[id] = unstable;
    }
    auto latest_elements = json::array();
    for (std::list<DataElement>::reverse_iterator it = new_data[id].rbegin(); it != new_data[id].rend(); ++it)
    {
      json element = {
              {"repeat", it->repeat},
              {"value", it->value},
              {"position", it->position},
      };
      latest_elements.push_back(element);
    }
    json state = {
            {"grad", grad},
            {"stdev", stdev},
            {"current_state", current_state},
            {"state", this->states[id]},
            {"time", time},
            {"epoch_time", epoch_time},
            {"sensor_id", id},
            {"latest_elements", latest_elements},
    };
    states.push_back(state);
    this->raw_data_to_file(current_time);
  }
  json state_results = this->setState();
  return {
            {"states", states},
            {"results", state_results},
            {"global_count", global_count},
  };
}

json FineBlanking::setState() {
      int stables = 0;
      int unstables = 0;
      int undefined = 0;
      std::vector<int> ids = getPhaseDetectionSensorIDs(this->config, 0);
      bool changed = false;

      for(unsigned int i = 0; i < ids.size(); i++) {
        int id = ids[i];
        if (this->states[id] == stable) {
          LOG_F(INFO, "Sensor %d is stable", id);
          stables += 1;
        } else if (this->states[id] == unstable) {
          LOG_F(INFO, "Sensor %d is unstable", id);
          unstables += 1;
        } else {
          undefined += 1;
          LOG_F(INFO, "Sensor %d is undefined", id);
        }
      }


      if (stables > unstables && current_state != stable) {
        LOG_F(WARNING, "Process is now stable");
        tsc->tables->updateLabel(42);
        stableSinceRestart = true;
        current_state = stable;
        changed = true;
      }
      if (unstables > stables && current_state != unstable) {
        LOG_F(WARNING, "Process is now unstable");
        tsc->tables->updateLabel(43);
        current_state = unstable;
        changed = true;
      }
      return {
            {"stables", stables},
            {"unstables", unstables},
            {"undefined", undefined},
            {"changed", changed},
    };
}

void FineBlanking::reset(unsigned int value, int sensor_index) {
  LOG_F(WARNING, "Reset Issued");
  this->updateFromRing(this->tsc, sensor_index);
  this->print_current_ring();
  this->determine_current_state();
  this->resetRing(value, sensor_index);
  if (this->gradient_detection_enabled) {
    stableSinceRestart = false;
    current_state = ramp_up;
    LOG_F(WARNING, "Process is now in ramp-up");
    tsc->tables->updateLabel(44);
  } else {
    stableSinceRestart = true;
    LOG_F(WARNING, "Process is now stable");
    tsc->tables->updateLabel(42);
    current_state = stable;
  }
  this->last_pointer[sensor_index] = 0;
  local_ring[sensor_index].clear();
  DataElement data = {value, 0, 1};
  local_ring[sensor_index].push_back(data);
  new_data[sensor_index].clear();
}

void FineBlanking::setRestartThreshold(uint32_t threshold) {
  tsc->tables->updateRestartThreshold(threshold);
}


void FineBlanking::initFile(std::string filepath, const char* params)
{
  std::filesystem::remove(filepath);
  std::ofstream file;
  file.open(filepath, std::ios::out | std::ios::trunc);
  if (file.fail())
    throw std::ios_base::failure(std::strerror(errno));
  file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);
  file << params << std::endl;
  file.close();
}


void FineBlanking::appendLineToFile(std::string filepath, std::string line)
{
  std::ofstream file;
  file.open(filepath, std::ios::out | std::ios::app);
  if (file.fail())
    throw std::ios_base::failure(std::strerror(errno));
  file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

  file << line << std::endl;
  file.close();
}

void FineBlanking::writeRawDataToFile(std::ofstream& file, std::chrono::system_clock::time_point current_time, int value, int pos) {
    std::chrono::milliseconds current_epoch_time = duration_cast< milliseconds >(
    current_time.time_since_epoch());
    file << std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count()) << ",";
    file << std::to_string(current_epoch_time.count()) << ",";
    file << std::to_string(value) << ",";
    file << std::to_string(pos);
    file << std::endl;
}

void FineBlanking::raw_data_to_file(std::chrono::system_clock::time_point current_time) {
  uint32_t previous_pointer = this->last_raw_pointer;
  if (previous_pointer == 0) {
    previous_pointer = 1;
  }
  this->last_raw_pointer =  tsc->raw_pointer->read(0);
  std::ofstream file;
  file.open(this->filename_raw, std::ios::out | std::ios::app);
  if (file.fail())
    throw std::ios_base::failure(std::strerror(errno));
  file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);
  while (previous_pointer < this->last_raw_pointer) {
    int value =  tsc->raw_storage->read(previous_pointer - 1);
    this->writeRawDataToFile(file, current_time, value, (previous_pointer - 1));
    previous_pointer = previous_pointer + 1;
  }
  file.close();
}

bool FineBlanking::is_current_state_rampup() {
  return current_state == ramp_up;
}



