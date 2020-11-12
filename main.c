#include <stdio.h>
#include <stdlib.h>

struct Sensor {
    double terminal_velocity;
    double x_pos;
    double y_pos;
    double z_pos;
    double x_velocity;
    double y_velocity;
    double z_velocity;
};

int initialize_sensors(struct Sensor sensors[], 
                       int count,
                       double start_x,
                       double start_y,
                       double start_z) {
    for (int i = 0; i < count; i++) {
        sensors[i].x_pos = start_x;
        sensors[i].y_pos = start_y;
        sensors[i].z_pos = start_z;
        sensors[i].x_velocity = 0;
        sensors[i].y_velocity = 0;
        sensors[i].z_velocity = 0;
    }
    return 0;
}

int update_velocity(struct Sensor sensors[], int sensor_count, double time_resolution, double gravity) {
    for (int i = 0; i < sensor_count; i++) {
        if (sensors[i].z_pos > 0) { 
            // to-do: add code to check for terminal velocity 
            sensors[i].z_velocity += (gravity * time_resolution);
        }
    }     
    return 0;
}

int update_position(struct Sensor sensors[], int sensor_count, double time_resolution, double gravity) {
    for (int i = 0; i < sensor_count; i++) {
        if (sensors[i].z_pos > 0) { 
            if (sensors[i].z_pos - (sensors[i].z_velocity * time_resolution) > 0) { 
                sensors[i].z_pos -= (sensors[i].z_velocity * time_resolution);
            }
            else {
                sensors[i].z_pos = 0;
            }
        }
    }
    return 0;
}


int clock_tick(struct Sensor sensors[], 
               int sensor_count, 
               double* current_time, 
               double time_resolution, 
               double gravity) {
    update_velocity(sensors, sensor_count, time_resolution, gravity);
    update_position(sensors, sensor_count, time_resolution, gravity); 
    *current_time += time_resolution; 
    return 0;
}

int main () {
    // Initialization
    int sensor_count = 10;
    int moving_sensors = 0; 
    int ret = 0;
    double gravity = 9.80665;
    double start_x = 0;
    double start_y = 0;
    double start_z = 1000;
    struct Sensor sensors[sensor_count];
    double current_time = 0;
    double time_resolution = 0.001;
    
    // Output parameters
    printf("Number of sensors: %d\n", sensor_count);
    printf("Gravity: %f m/(s^2)\n", gravity);
    printf("Time resolution: %f secs/tick\n", time_resolution); 
    printf("Starting height: %f meters\n", start_z);

    // Get sensors ready
    printf("Sensor initialization: ");
    ret = initialize_sensors(sensors, sensor_count, start_x, start_y, start_z);
    if (ret == 0) {
        printf("OK\n");
        moving_sensors = sensor_count;
    }
    
    // Run until all sensors reach z = 0;
    printf("Running simulation\n");
    while (moving_sensors != 0) {
        clock_tick(sensors, sensor_count, &current_time, time_resolution, gravity);
        moving_sensors = 0; 
        for (int i = 0; i < sensor_count; i++) {
            if (sensors[i].z_pos > 0) {
                moving_sensors++;
            }
        }
    }        

    // Print final positions
    for (int i = 0; i < sensor_count; i++) {
        printf("Sensor %d final velocity: %f m/s, final position: (%f, %f, %f)\n", 
               i, sensors[i].z_velocity, sensors[i].x_pos, sensors[i].y_pos, sensors[i].z_pos);
    }
    printf("Final time: %f seconds\n", current_time);
    
    return 0;
}

