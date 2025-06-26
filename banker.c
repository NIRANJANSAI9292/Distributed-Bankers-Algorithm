#include "banker.h"

// Initialize a node with given resources
void init_node(Node *node, int node_id, int num_resources) {
    node->node_id = node_id;
    node->num_resources = num_resources;
    node->num_processes = 0;
    
    // Initialize available resources
    for (int i = 0; i < num_resources; i++) {
        node->available[i] = 0;
    }
    
    // Initialize processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        node->processes[i].pid = -1;
        node->processes[i].priority = 0;
        node->processes[i].is_completed = false;
        for (int j = 0; j < num_resources; j++) {
            node->processes[i].allocation[j] = 0;
            node->processes[i].max[j] = 0;
            node->processes[i].need[j] = 0;
        }
    }
}

// Check if the current state is safe
bool is_safe_state(Node *node) {
    int work[node->num_resources];
    bool finish[node->num_processes];
    
    // Initialize work and finish arrays
    for (int i = 0; i < node->num_resources; i++) {
        work[i] = node->available[i];
    }
    for (int i = 0; i < node->num_processes; i++) {
        finish[i] = node->processes[i].is_completed;
    }
    
    // Find a process that can be completed
    bool found;
    do {
        found = false;
        for (int i = 0; i < node->num_processes; i++) {
            if (!finish[i]) {
                bool can_complete = true;
                for (int j = 0; j < node->num_resources; j++) {
                    if (node->processes[i].need[j] > work[j]) {
                        can_complete = false;
                        break;
                    }
                }
                
                if (can_complete) {
                    for (int j = 0; j < node->num_resources; j++) {
                        work[j] += node->processes[i].allocation[j];
                    }
                    finish[i] = true;
                    found = true;
                }
            }
        }
    } while (found);
    
    // Check if all processes can complete
    for (int i = 0; i < node->num_processes; i++) {
        if (!finish[i]) {
            return false;
        }
    }
    return true;
}

// Request resources for a process
bool request_resources(Node *node, int process_id, int *request) {
    // Validate process ID
    if (process_id < 0 || process_id >= node->num_processes) {
        return false;
    }
    
    // Check if request exceeds need
    for (int i = 0; i < node->num_resources; i++) {
        if (request[i] > node->processes[process_id].need[i]) {
            return false;
        }
    }
    
    // Check if request exceeds available resources
    for (int i = 0; i < node->num_resources; i++) {
        if (request[i] > node->available[i]) {
            return false;
        }
    }
    
    // Try to allocate resources
    for (int i = 0; i < node->num_resources; i++) {
        node->available[i] -= request[i];
        node->processes[process_id].allocation[i] += request[i];
        node->processes[process_id].need[i] -= request[i];
    }
    
    // Check if the new state is safe
    if (is_safe_state(node)) {
        return true;
    } else {
        // Rollback the allocation
        for (int i = 0; i < node->num_resources; i++) {
            node->available[i] += request[i];
            node->processes[process_id].allocation[i] -= request[i];
            node->processes[process_id].need[i] += request[i];
        }
        return false;
    }
}

// Release resources from a process
bool release_resources(Node *node, int process_id, int *release) {
    // Validate process ID
    if (process_id < 0 || process_id >= node->num_processes) {
        return false;
    }
    
    // Check if release is valid
    for (int i = 0; i < node->num_resources; i++) {
        if (release[i] > node->processes[process_id].allocation[i]) {
            return false;
        }
    }
    
    // Release the resources
    for (int i = 0; i < node->num_resources; i++) {
        node->available[i] += release[i];
        node->processes[process_id].allocation[i] -= release[i];
        node->processes[process_id].need[i] += release[i];
    }
    
    return true;
}

// Check if a request can be granted
bool can_grant_request(Node *node, int process_id, int *request) {
    // Make a copy of the node state
    Node temp_node = *node;
    
    // Try to allocate resources in the temporary state
    for (int i = 0; i < node->num_resources; i++) {
        temp_node.available[i] -= request[i];
        temp_node.processes[process_id].allocation[i] += request[i];
        temp_node.processes[process_id].need[i] -= request[i];
    }
    
    // Check if the temporary state is safe
    return is_safe_state(&temp_node);
}

// Print the current state of the system
void print_state(Node *node) {
    printf("\nNode %d State:\n", node->node_id);
    printf("Available Resources: ");
    for (int i = 0; i < node->num_resources; i++) {
        printf("%d ", node->available[i]);
    }
    printf("\n\n");
    
    printf("Process\tAllocation\tMax\t\tNeed\n");
    for (int i = 0; i < node->num_processes; i++) {
        printf("P%d\t", i);
        for (int j = 0; j < node->num_resources; j++) {
            printf("%d ", node->processes[i].allocation[j]);
        }
        printf("\t");
        for (int j = 0; j < node->num_resources; j++) {
            printf("%d ", node->processes[i].max[j]);
        }
        printf("\t");
        for (int j = 0; j < node->num_resources; j++) {
            printf("%d ", node->processes[i].need[j]);
        }
        printf("\n");
    }
} 
