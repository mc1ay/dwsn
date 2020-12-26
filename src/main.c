#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define XYACCELDELTAMAX 0.005
#define DRAGVARIANCE 0.05

struct Node {
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
    double power_output;
    double* received_signals;
};

int write_node_data(struct Node* nodes, int node_count, int id, double current_time, FILE *fp) {
    char buffer[200];
    sprintf(buffer, "%f %f %f %f ", current_time, nodes[id].x_pos, nodes[id].y_pos, nodes[id].z_pos);
    fputs(buffer, fp);
    for (int i = 0; i < node_count; i++) {
        if (i < node_count - 1) {
            sprintf(buffer, "%f ", nodes[id].received_signals[i]);
        }
        else {
            sprintf(buffer, "%f", nodes[id].received_signals[i]);
        }
        fputs(buffer, fp);
    }
    sprintf(buffer,"\n");
    fputs(buffer, fp);
}

int initialize_nodes(struct Node* nodes, 
                       int node_count,
                       double terminal_velocity,
                       double start_x,
                       double start_y,
                       double start_z,
                       double gravity,
                       double power_output,
                       int output,
                       char* output_dir,
                       int debug) {
    char file_path[100];

    if (debug) {
        printf("Setting inital node coordinates to %f %f %f\n", start_x, start_y, start_z);
    }

    for (int i = 0; i < node_count; i++) {
        nodes[i].terminal_velocity = 
            terminal_velocity + (terminal_velocity * DRAGVARIANCE * (rand() % 201 - 100.0) / 100);
        nodes[i].x_pos = start_x;
        nodes[i].y_pos = start_y;
        nodes[i].z_pos = start_z;
        nodes[i].x_velocity = 0;
        nodes[i].y_velocity = 0;
        nodes[i].z_velocity = 0;
        nodes[i].x_acceleration = 0;
        nodes[i].y_acceleration = 0;
        nodes[i].z_acceleration = gravity;
        nodes[i].power_output = power_output;
        nodes[i].received_signals = malloc(sizeof(double) * node_count);
        for (int j = 0; j < node_count; j++) {
            nodes[i].received_signals[j] = 0;
        }

        if (output) {
            sprintf(file_path, "%s/%d%s", output_dir, i, ".txt");
            if (debug) {
                printf("Creating output file \"%s\"\n", file_path);
            }
            FILE *fp;
            fp  = fopen (file_path, "w");
            write_node_data(nodes, node_count, i, 0.0, fp);
            fclose(fp);
        }
    }
    return 0;
}

int update_acceleration(struct Node* nodes, int node_count, double time_resolution, double spread_factor, int debug) {
    for (int i = 0; i < node_count; i++) {
        // update x/y acceleration
        // use spread_factor as percentage likelyhood that there is some change to acceleration
        if (rand() % 100 < spread_factor) {
            // change x and y by random percentage of max allowed change per second
            double x_accel_change = (rand() % 201 - 100) / 100.0 * time_resolution * XYACCELDELTAMAX;
            double y_accel_change = (rand() % 201 - 100) / 100.0 * time_resolution * XYACCELDELTAMAX;
            if (debug >= 3) {
                printf("Changing x/y accel for node %d by %f,%f\n", i, x_accel_change, y_accel_change);
            }
            nodes[i].x_acceleration += x_accel_change;
            nodes[i].y_acceleration += y_accel_change;
        }
        // update z acceleration 
        // for our purposes z always equals gravity so not update needed (just a placeholder)
    }
    return 0;
}

int update_velocity(struct Node* nodes, int node_count, double time_resolution, int debug) {
    for (int i = 0; i < node_count; i++) {
        // update z velocity
        if (nodes[i].z_pos > 0) { 
            if (nodes[i].z_velocity < nodes[i].terminal_velocity) {
                if (nodes[i].z_velocity + (nodes[i].z_acceleration * time_resolution) < nodes[i].terminal_velocity) {
                    nodes[i].z_velocity += (nodes[i].z_acceleration * time_resolution);
                }
                else {
                    nodes[i].z_velocity = nodes[i].terminal_velocity;
                    if (debug >=2) {
                        printf("Node %d reached terminal velocity of %f m/s\n", i, nodes[i].terminal_velocity);
                    }
                }
            }
        }
        // update x/y velocity
        nodes[i].x_velocity += (nodes[i].x_acceleration * time_resolution);
        nodes[i].y_velocity += (nodes[i].y_acceleration * time_resolution);
    }     
    return 0;
}

int update_position(struct Node* nodes, int node_count, double time_resolution, int debug) {
    for (int i = 0; i < node_count; i++) {
        // Update z position
        if (nodes[i].z_pos > 0) { 
            if (nodes[i].z_pos - (nodes[i].z_velocity * time_resolution) > 0) { 
                nodes[i].z_pos -= (nodes[i].z_velocity * time_resolution);
            }
            else {
                nodes[i].z_pos = 0;
            }
        }
        // Update x/y position
        nodes[i].x_pos += (nodes[i].x_velocity * time_resolution);
        nodes[i].y_pos += (nodes[i].y_velocity * time_resolution);

    }
    return 0;
}

int update_signals(struct Node* nodes, int node_count, double current_time, int debug, int output, char* output_dir, double write_interval, double time_resolution) {
    // Not taking noise floor into account currently
    // Check distance to other node nodes and calculate free space loss
    // to get received signal 
    char file_path[100];

    for (int i = 0; i < node_count; i++) {
        for (int j = 0; j < node_count; j++) {
            if (i != j) {
                double distance = sqrt(
                    pow((nodes[i].x_pos - nodes[j].x_pos),2) +
                    pow((nodes[i].y_pos - nodes[j].y_pos),2) +
                    pow((nodes[i].z_pos - nodes[j].z_pos),2) 
                );
                nodes[i].received_signals[j] = nodes[j].power_output -
                    (20 * log(distance) + 20 * log(2400) + 32.44);
            }
        }
        if (output) {
            if (fmod(current_time, write_interval) < time_resolution) {
                sprintf(file_path, "%s/%d%s", output_dir, i, ".txt");
                FILE *fp;
                fp  = fopen (file_path, "a");
                write_node_data(nodes, node_count, i, current_time, fp);
                fclose(fp);
            }
        }
    }
    return 0;
}

int clock_tick(struct Node* nodes, 
               int node_count, 
               double* current_time, 
               double time_resolution, 
               double gravity,
               double spread_factor,
               int debug,
               int output,
               char* output_dir,
               int write_interval) {
    *current_time += time_resolution; 

    update_acceleration(nodes, node_count, time_resolution, spread_factor, debug);
    update_velocity(nodes, node_count, time_resolution, debug);
    update_position(nodes, node_count, time_resolution, debug);
    update_signals(nodes, node_count, *current_time, debug, output, output_dir, write_interval, time_resolution); 

    return 0;
}

int main(int argc, char **argv) {
    // Initialization and defaults
    clock_t t1, t2;
    int node_count = 10;
    int moving_nodes = 0; 
    int ret = 0;
    double gravity = 9.80665;
    double start_x = 0;
    double start_y = 0;
    double start_z = 30000;
    double current_time = 0;
    double time_resolution = 0.001;
    double terminal_velocity = 8.0;
    double spread_factor = 20;
    double default_power_output = 400;
    double write_interval = 1.0;
    int random_seed = -1;
    int debug = 0;
    int verbose = 1;
    int output = 0;
    char output_dir[50];

    // get command line switches
    int c;
    while ((c = getopt(argc, argv, "d:v:c:g:r:z:t:s:e:p:o:")) != -1)
    switch (c) {
        case 'd':
            debug = atoi(optarg);
            break;
        case 'v':
            verbose = atoi(optarg);
            break;
        case 'c':
            node_count = atoi(optarg);
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
        case 'e':
            random_seed = atoi(optarg);
            break;
        case 'p':
            default_power_output = atof(optarg);
            break;
        case 'o':
            output = atoi(optarg);
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
    
    // Seed random number generator if seed isn't specified
    if (random_seed < 0) {
        srand(time(NULL)); 
        printf("Seeded random number generator\n");
    }

    // Output parameters
    printf("Number of nodes: %d\n", node_count);
    printf("Gravity: %f m/(s^2)\n", gravity);
    printf("Time resolution: %f secs/tick\n", time_resolution); 
    printf("Starting height: %f meters\n", start_z);
    printf("Terminal velocity: %f meters/second\n", terminal_velocity);
    printf("Spread factor: %f\n", spread_factor);
    printf("Default power output: %f\n", default_power_output);
    
    // Make log directory is output option is turned on
    if (output) {
        struct tm *timenow;
        time_t now = time(NULL);
        timenow = gmtime(&now);
        strftime(output_dir, sizeof(output_dir), "output/run/%Y-%m-%d-%H-%M-%S", timenow);
        printf("Creating output directory \"%s\": ", output_dir);

        // Make path for timestamped directory if it doesn't already exist
        struct stat st = {0};
        if (stat("output", &st) == -1) {
           mkdir("output", 0777);
        }
        if (stat("output/run", &st) == -1) {
           mkdir("output/run", 0777);
        }

        // Make directory just for this run
        ret = mkdir(output_dir,0777); 
  
        // check if directory is created or not 
        if (!ret) {
            printf("OK\n"); 
        }
        else { 
            printf("Unable to create directory, exiting\n"); 
            exit(1); 
        } 
    }

    // Get nodes ready
    printf("Initializing nodes\n");
    struct Node nodes[node_count];
    ret = initialize_nodes(nodes, node_count, terminal_velocity, start_x, start_y, start_z, gravity, default_power_output, output, output_dir, debug);
    if (ret == 0) {
        printf("Initialization OK\n");
        moving_nodes = node_count;
    }
    
    // Run until all nodes reach z = 0;
    printf("Running simulation\n");
    t1 = clock();

    while (moving_nodes != 0) {
        clock_tick(nodes, node_count, &current_time, time_resolution, gravity, spread_factor, debug, output, output_dir, write_interval);
        moving_nodes = 0; 
        for (int i = 0; i < node_count; i++) {
            if (nodes[i].z_pos > 0) {
                moving_nodes++;
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
        for (int i = 0; i < node_count; i++) {
            printf("Node %d final velocity: %f %f %f m/s, final position: %f %f %f\n", 
                i, 
                nodes[i].x_velocity, 
                nodes[i].y_velocity, 
                nodes[i].z_velocity, 
                nodes[i].x_pos, 
                nodes[i].y_pos, 
                nodes[i].z_pos);
        }
    }
    printf("Final clock time: %f seconds\n", current_time);
    
    return 0;
}