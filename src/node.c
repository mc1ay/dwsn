/**
 * @file    node.c
 * @brief   Node specific functions for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "node.h"

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