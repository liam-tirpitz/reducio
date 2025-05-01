## Configuration
The [config.json](config.json) contains the dynamic run configuration for the Control Plane.
It consists of two sections, one for each processing step.
The clustering section additionally contains application specific details for the fineblanking application.
```
{
  "aggregation": {
    "ingress_port": 53,
    "recirc_port": 68,
    "out_pipe1_port": 60,
    "phase_change_delay": 1700,
    "zero_noise_bits": 14
  },
  "ring": {
    "read_from_dp_interval_in_s": 6,
    "window": 7,
    "local_ring_size": 500,
    "mask": "0xFFFFFF00",
    "fine-blanking": {
      "stable_std_threshold": 1100,
      "gradient_window": 4,
      "restart_threshold": 5000
    }
  }
}
```


| Section     | Name                                | Example   | Description                                                       |
|-------------|-------------------------------------|-----------|-------------------------------------------------------------------|
| aggregation    | ingress_port                        | 53        | Port on Pipe 0 that receives the raw sensor data                                         |
| aggregation    | recirc_port                         | 68        | Recirculation port on Pipe 0 |
| aggregation    | out_pipe1_port                      | 60        | Port that bridges from Pipe 0 to Pipe 1                                                                  |
| aggregation    | phase_change_delay                  | 50        | Number of packets that need to be classified as zero-noise to consider a punch completed                           |
| aggregation    | zero_noise_bits                     | 11        | Number of bits, that are considered zero noise. For example, if z= 11 all values between [-2^11, 2^11] are considered zero noise.  |
| ring           | local_ring_size                     | 500       | Number of ring values that are held in CP memory |
| ring           | read_from_dp_interval_in_s          | 2         | Time in seconds between data reads from the DP .|
| ring           | window                              | 7         | How many values from the ring are considered for the calulcation of standard deviation and gradient   |
| ring           | mask                                | 0xFFFFFF00| Mask applied to incoming values, reduces accuracy but also memeory and communication overhead   |
| fine-blanking  | stable_std_threshold                | 1100      | Defines the std threshold for a stable phase. If the std is above the threshold, the next packets are labeled as instable. |
| fine-blanking  | gradient_window                     | 4         | Defines the gradient window. Only the previous x values are considered to detect the end of the ramp-up phase |
| fine-blanking  | restart_threshold                   | 5000      | If the maximum force between two punches drops x Newton or more, we assume that the process was restarted and reset the clustering information accordingly.|
