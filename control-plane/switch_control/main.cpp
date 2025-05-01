#include "tofino_switch_control.hpp"
#include "fine_blanking.hpp"
#include "sampling.hpp"
#include "failure_detection.hpp"

#include <chrono>
#include <thread>
#include <cmath>
#include <math.h>
#include <iostream>
#include <ctime>
#include <vector>
#include <numeric>
#include <fstream>
#include <getopt.h>
#include <sstream>
#include "include/json.hpp"
#include <loguru.hpp>

using json = nlohmann::json;

json config;
enum Phase { sampling, clustering, combined};


TofinoSwitchControl* tsc;
FineBlanking* fb;
Sampling* samplingClass;
FailureDetection* failureDetection;

bool first_packet_initialized = false;

Phase active_phase = combined;


time_t rawtime;
struct tm * timeinfo;
char buffer[80];




// Read and write to a register for dev purposes.
void testRegister(TofinoRegister register_, int registerID, int value) {
  LOG_F(INFO, "Write to register %d", registerID);
  register_.write(registerID, value);
  LOG_F(INFO, "Read from register %d", registerID);
  int result = register_.read(registerID);
  LOG_F(INFO, "Successfully read from register %d. Value: %d", registerID, result);
}

void testGradient() {
  std::vector<int> elements;
    int value = 833536;
    elements.push_back(value);
    value = 836096;
    elements.push_back(value);
    value = 835840;
    elements.push_back(value);
    value = 834816;
    elements.push_back(value);
    value = 834048;
    elements.push_back(value);
    value = 833280;
    elements.push_back(value);
    value = 834816;
    elements.push_back(value);
    assert(fb->gradient(elements) == -128.0);
}



// If a packet arrives here, the data plane encountered data that does not fit in the existing partitions. The CP must create a suitable partition. OR
// If a packet arrives here, the data plane detected a machine restart in the process. Reset Clustering information.
void capture_unknown(sniff_label* label, sniff_sensor* sensor0, sniff_sensor* sensor1, sniff_sensor* sensor2, sniff_sensor* sensor3, sniff_sensor* sensor4, sniff_sensor* sensor5, void* arg) {
  LOG_F(INFO, "Handle CPU packet: %d::%d::%d::%d::%d::%d::%d", sensor0->value, sensor1->value, sensor2->value, sensor3->value, sensor4->value, sensor5->value, label->label);  try
  {
    if (label->label == 41) {
      LOG_F(WARNING, "Restart detected!");
      fb->reset(sensor0->value, 0);
      fb->reset(sensor1->value, 1);
      fb->reset(sensor2->value, 2);
      fb->reset(sensor3->value, 3);

    } else if (label->label == 1){
      LOG_F(WARNING, "Out-of-bounds detected");
      failureDetection->handleOutofBoundsEvent(config, sensor0, sensor1, sensor2, sensor3, sensor4, sensor5);
    } else {
    }
  }
  catch (const std::runtime_error& error)
  {
    LOG_F(ERROR, error.what());
  }

}

void current_state_to_file(json states) {
  std::ofstream file;

  file.open("combined.csv", std::ios::out | std::ios::app);
  if (file.fail())
    throw std::ios_base::failure(std::strerror(errno));
  file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);
  file << states.dump();
  file << std::endl;
  file.close();
}


void readConfig(std::string filepath) {
  std::stringstream ss;
  ss << "Load config file: " << filepath << std::endl;  
  LOG_F(INFO, ss.str().c_str());
  std::ifstream i(filepath);
  i >> config;
}


int main(int argc, char** argv) {
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  auto start_time = std::chrono::system_clock::now();

  strftime(buffer,sizeof(buffer),"%Y-%m-%d-%H-%M-%S",timeinfo);
  std::string datestr(buffer);

  std::string config_file_path = "config.json";

  static const struct option long_options[] =
    {
        { "config", 						required_argument,		0, 'c' },
        { "mode", 			required_argument, 		0, 'm' },
        { "kernel-pkt", no_argument, 0, 'k'}
    };


  bool kernel_pkt_mode = true;
  std::stringstream ss;
	while (true)
    {
		// http://www.mario-konrad.ch/blog/programming/getopt.html
		// https://gist.github.com/ashwin/d88184923c7161d368a9
        const auto opt = getopt_long(argc, argv, "c:m:k", long_options, nullptr);

        if (-1 == opt)
            break;

        switch (opt)
        {
        case 'c':
            config_file_path = std::string(optarg);
            ss.str(std::string());
            ss << "Use config file: " << config_file_path << std::endl;  
			      LOG_F(INFO, ss.str().c_str());
            break;

        case 'm':
            if (!strcmp(optarg, "clustering")) {
              active_phase = clustering;
              LOG_F(INFO, "Load clustering to both pipes");
            } else if (!strcmp(optarg, "sampling")) {
              active_phase = sampling;
              LOG_F(INFO, "Load sampling to both pipes");
            } else if (!strcmp(optarg, "combinedcluster")) {
              LOG_F(INFO, "Load combined sampling and clustering");
            } else {
              LOG_F(INFO, "Load combined sampling and clustering");
            }
            break;
        case 'k':
            kernel_pkt_mode = true;
            LOG_F(INFO, "Kernel packet mode enabled.");
            break;

        case 'h': // -h or --help
        case '?': // Unrecognized option
        default:
            std::cout << "Don't know this option" << std::endl; //PrintHelp();
            break;
        }
    }





  readConfig(config_file_path);

  // Initialize ControlPlane classes.
  tsc = new TofinoSwitchControl(active_phase != clustering, active_phase != sampling, kernel_pkt_mode, config["ring"]["mask"].get<std::string>());
  tsc->initializeDataplaneInterfaces();
  LOG_F(INFO, "Initialize Dataplane.");
  tsc->setupDataplane();
  LOG_F(INFO, "Initialize Sampling.");
  samplingClass = new Sampling(tsc);
  LOG_F(INFO, "Initialize FB next.");
  fb = new FineBlanking(tsc, (int)config["ring"]["fine-blanking"]["stable_std_threshold"], 
    (int)config["ring"]["fine-blanking"]["restart_threshold"], 
    (int)config["ring"]["window"], 
    (int)config["ring"]["local_ring_size"], 
    (int)config["ring"]["fine-blanking"]["gradient_window"], (bool)config["ring"]["fine-blanking"]["enable_gradient_detection"], start_time, config);
  failureDetection = new FailureDetection(tsc);
  failureDetection->setThresholds(config);
  failureDetection->setDifferences(config);
  LOG_F(INFO, "FB initialized.");
  // Initialize tables.
  if (active_phase != clustering) {
    samplingClass->initSampling(config);
    LOG_F(INFO, "Sampling initialized");
  }
  if (active_phase != sampling) {
    LOG_F(INFO, "Clustering initialized");

    tsc->setupUnknownCapture(capture_unknown);
  }

  int sleep = (int)config["ring"]["read_from_dp_interval_in_s"];

  // Read current state regularly
  while (true) {
    if (active_phase != sampling) {
      json sampling_state = samplingClass->readAllCounters(config);
      fb->updateFromRing(tsc, 0);
      fb->updateFromRing(tsc, 1);
      fb->updateFromRing(tsc, 2);
      fb->updateFromRing(tsc, 3);
      fb->updateFromRing(tsc, 4);

      fb->print_current_ring();
      json fineblanking_state = fb->determine_current_state();
      if (fb->is_current_state_rampup()) {
        sleep = (int)config["ring"]["read_from_dp_interval_in_s_during_rampup"];
      } else {
        sleep = (int)config["ring"]["read_from_dp_interval_in_s"];
      }

      json state = {
        {"sampling_state", sampling_state},
        {"fineblaning_state", fineblanking_state},
      };
      current_state_to_file(state);
 
    }
    std::this_thread::sleep_for (std::chrono::milliseconds(sleep));
  }
  return 0;
}
