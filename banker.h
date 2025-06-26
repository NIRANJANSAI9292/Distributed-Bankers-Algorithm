#ifndef BANKER_H
#define BANKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10
#define MAX_NODES 3

// Structure to represent a process
typedef struct {
    int pid;
    int priority;
    int allocation[MAX_RESOURCES];
    int max[MAX_RESOURCES];
    int need[MAX_RESOURCES];
    bool is_completed;
} Process;

// Structure to represent a node in the system
typedef struct {
    int node_id;
    int available[MAX_RESOURCES];
    Process processes[MAX_PROCESSES];
    int num_processes;
    int num_resources;
} Node;

// Core Banker's Algorithm functions
bool is_safe_state(Node *node);
bool request_resources(Node *node, int process_id, int *request);
bool release_resources(Node *node, int process_id, int *release);
bool can_grant_request(Node *node, int process_id, int *request);

// Priority scheduling functions
void update_priorities(Node *node);
int get_highest_priority_process(Node *node);

// Deadlock prediction functions
bool predict_deadlock(Node *node, int process_id, int *request);
void update_deadlock_history(Node *node, int process_id, bool was_safe);

// Utility functions
void print_state(Node *node);
void init_node(Node *node, int node_id, int num_resources);

// Simulation control
extern volatile bool should_exit;
DWORD WINAPI process_simulator(LPVOID arg);

#endif // BANKER_H 
