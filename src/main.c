#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define XYACCELDELTAMAX 0.005
#define DRAGVARIANCE 0.05

struct Sensor {
    double terminal_velocity;
    double x_pos;
    double y_pos;
    double z_pos;
    double x_velocity;
    double y_velocity;
    double z_velocity;
    double x_acceleration;
    double y_acceleration;
    double z_acceleration;
};

int initialize_sensors(struct Sensor sensors[], 
                       int count,
                       double terminal_velocity,
                       double start_x,
                       double start_y,
                       double start_z,
                       double gravity) {
    for (int i = 0; i < count; i++) {
        sensors[i].terminal_velocity = 
            terminal_velocity + (terminal_velocity * DRAGVARIANCE * (rand() % 201 - 100.0) / 100);
        sensors[i].x_pos = start_x;
        sensors[i].y_pos = start_y;
        sensors[i].z_pos = start_z;
        sensors[i].x_velocity = 0;
        sensors[i].y_velocity = 0;
        sensors[i].z_velocity = 0;
        sensors[i].x_acceleration = 0;
        sensors[i].y_acceleration = 0;
        sensors[i].z_acceleration = gravity;
    }
    return 0;
}

int update_acceleration(struct Sensor sensors[], int sensor_count, double time_resolution, double spread_factor, int debug) {
    for (int i = 0; i < sensor_count; i++) {
        // update x/y acceleration
        // use spread_factor as percentage likelyhood that there is some change to acceleration
        if (rand() % 100 < spread_factor) {
            // change x and y by random percentage of max allowed change per second
            double x_accel_change = (rand() % 201 - 100) / 100.0 * time_resolution * XYACCELDELTAMAX;
            double y_accel_change = (rand() % 201 - 100) / 100.0 * time_resolution * XYACCELDELTAMAX;
            if (debug >= 3) {
                printf("Changing x/y accel for sensor %d by %f,%f\n", i, x_accel_change, y_accel_change);
            }
            sensors[i].x_acceleration += x_accel_change;
            sensors[i].y_acceleration += y_accel_change;
        }
        // update z acceleration 
        // for our purposes z always equals gravity so not update needed (just a placeholder)
    }
    return 0;
}

int update_velocity(struct Sensor sensors[], int sensor_count, double time_resolution, int debug) {
    for (int i = 0; i < sensor_count; i++) {
        // update z velocity
        if (sensors[i].z_pos > 0) { 
            if (sensors[i].z_velocity < sensors[i].terminal_velocity) {
                if (sensors[i].z_velocity + (sensors[i].z_acceleration * time_resolution) < sensors[i].terminal_velocity) {
                    sensors[i].z_velocity += (sensors[i].z_acceleration * time_resolution);
                }
                else {
                    sensors[i].z_velocity = sensors[i].terminal_velocity;
                    if (debug >=2) {
                        printf("Sensor %d reached terminal velocity of %f m/s\n", i, sensors[i].terminal_velocity);
                    }
                }
            }
        }
        // update x/y velocity
        sensors[i].x_velocity += (sensors[i].x_acceleration * time_resolution);
        sensors[i].y_velocity += (sensors[i].y_acceleration * time_resolution);
    }     
    return 0;
}

int update_position(struct Sensor sensors[], int sensor_count, double time_resolution, int debug) {
    for (int i = 0; i < sensor_count; i++) {
        // Update z position
        if (sensors[i].z_pos > 0) { 
            if (sensors[i].z_pos - (sensors[i].z_velocity * time_resolution) > 0) { 
                sensors[i].z_pos -= (sensors[i].z_velocity * time_resolution);
            }
            else {
                sensors[i].z_pos = 0;
            }
        }
        // Update x/y position
        sensors[i].x_pos += (sensors[i].x_velocity * time_resolution);
        sensors[i].y_pos += (sensors[i].y_velocity * time_resolution);

    }
    return 0;
}

int clock_tick(struct Sensor sensors[], 
               int sensor_count, 
               double* current_time, 
               double time_resolution, 
               double gravity,
               double spread_factor,
               int debug) {
    update_acceleration(sensors, sensor_count, time_resolution, spread_factor, debug);
    update_velocity(sensors, sensor_count, time_resolution, debug);
    update_position(sensors, sensor_count, time_resolution, debug); 
    *current_time += time_resolution; 
    return 0;
}

int main(int argc, char **argv) {
    // Initialization and defaults
    clock_t t1, t2;
    int sensor_count = 10;
    int moving_sensors = 0; 
    int ret = 0;
    double gravity = 9.80665;
    double start_x = 0;
    double start_y = 0;
    double start_z = 30000;
    double current_time = 0;
    double time_resolution = 0.001;
    double terminal_velocity = 8.0;
    double spread_factor = 20;
    int debug = 0;
    int verbose = 1;

    // get command line switches
    int c;
    while ((c = getopt(argc, argv, "d:v:c:g:r:z:t:s:")) != -1)
    switch (c) {
        case 'd':
            debug = atoi(optarg);
            break;
        case 'v':
            verbose = atoi(optarg);
            break;
        case 'c':
            sensor_count = atoi(optarg);
            break;
        case 'g':
            gravity = atof(optarg);
            break;
        case 'r':
            time_resolution = atof(optarg);
            break;
        case 'z':
            start_z = atof(optarg);
            break;
        case 't':
            terminal_velocity = atof(optarg);
            break;
        case 's': 
            spread_factor = atof(optarg);
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
                return 1;
        default:
            abort ();
    }
    
    // Output parameters
    printf("Number of sensors: %d\n", sensor_count);
    printf("Gravity: %f m/(s^2)\n", gravity);
    printf("Time resolution: %f secs/tick\n", time_resolution); 
    printf("Starting height: %f meters\n", start_z);
    printf("Terminal velocity: %f meters/second\n", terminal_velocity);
    printf("Spread factor: %f\n", spread_factor);

    // Get sensors ready
    printf("Sensor initialization: ");
    struct Sensor sensors[sensor_count];

    ret = initialize_sensors(sensors, sensor_count, terminal_velocity, start_x, start_y, start_z, gravity);
    if (ret == 0) {
        printf("OK\n");
        moving_sensors = sensor_count;
    }
    
    // Run until all sensors reach z = 0;
    printf("Running simulation\n");
    t1 = clock();

    while (moving_sensors != 0) {
        clock_tick(sensors, sensor_count, &current_time, time_resolution, gravity, spread_factor, debug);
        moving_sensors = 0; 
        for (int i = 0; i < sensor_count; i++) {
            if (sensors[i].z_pos > 0) {
                moving_sensors++;
            }
        }
    }

    // Calculate simulation time
    t2 = clock();
    double runTime = (double)(t2 - t1) / CLOCKS_PER_SEC;

    // Print summary information
    printf("Simulation complete\n");
    printf("Simulation time: %f seconds\n", runTime);        

    if (debug) {
        for (int i = 0; i < sensor_count; i++) {
            printf("Sensor %d final velocity: %f %f %f m/s, final position: %f %f %f\n", 
                i, 
                sensors[i].x_velocity, 
                sensors[i].y_velocity, 
                sensors[i].z_velocity, 
                sensors[i].x_pos, 
                sensors[i].y_pos, 
                sensors[i].z_pos);
        }
    }
    printf("Final clock time: %f seconds\n", current_time);
    
    return 0;
}