#include "banker.h"
#include <windows.h>
#include <time.h>
#include <stdlib.h>

// Sample process data
typedef struct {
    int max[MAX_RESOURCES];
    int allocation[MAX_RESOURCES];
    int priority;
} ProcessData;

// Sample datasets for different users
ProcessData user1_processes[] = {
    {{7, 5, 3}, {0, 1, 0}, 1},
    {{3, 2, 2}, {2, 0, 0}, 2},
    {{9, 0, 2}, {3, 0, 2}, 3}
};

ProcessData user2_processes[] = {
    {{2, 2, 2}, {2, 1, 1}, 1},
    {{4, 3, 3}, {0, 0, 2}, 2},
    {{3, 3, 2}, {1, 0, 0}, 3}
};

ProcessData user3_processes[] = {
    {{4, 3, 3}, {1, 1, 1}, 1},
    {{6, 1, 1}, {2, 1, 0}, 2},
    {{3, 2, 2}, {0, 0, 2}, 3}
};

// Initialize a node with sample data
void init_node_with_data(Node *node, int node_id, ProcessData *processes, int num_processes) {
    init_node(node, node_id, 3); // 3 resources
    
    // Set available resources
    node->available[0] = 10;
    node->available[1] = 5;
    node->available[2] = 7;
    
    // Add processes
    node->num_processes = num_processes;
    for (int i = 0; i < num_processes; i++) {
        node->processes[i].pid = i;
        node->processes[i].priority = processes[i].priority;
        node->processes[i].is_completed = false;
        
        for (int j = 0; j < node->num_resources; j++) {
            node->processes[i].max[j] = processes[i].max[j];
            node->processes[i].allocation[j] = processes[i].allocation[j];
            node->processes[i].need[j] = processes[i].max[j] - processes[i].allocation[j];
        }
    }
}

// Simulate process requests
DWORD WINAPI process_simulator(LPVOID arg) {
    Node *node = (Node *)arg;
    srand(time(NULL));
    
    while (1) {
        // Select a random process
        int process_id = rand() % node->num_processes;
        if (node->processes[process_id].is_completed) {
            continue;
        }
        
        // Generate random request
        int request[node->num_resources];
        for (int i = 0; i < node->num_resources; i++) {
            request[i] = rand() % (node->processes[process_id].need[i] + 1);
        }
        
        // Check for deadlock prediction
        if (predict_deadlock(node, process_id, request)) {
            printf("Node %d: Deadlock predicted for process %d\n", node->node_id, process_id);
            continue;
        }
        
        // Try to request resources
        if (request_resources(node, process_id, request)) {
            printf("Node %d: Process %d request granted\n", node->node_id, process_id);
            
            // Check if process is completed
            bool completed = true;
            for (int i = 0; i < node->num_resources; i++) {
                if (node->processes[process_id].need[i] > 0) {
                    completed = false;
                    break;
                }
            }
            
            if (completed) {
                node->processes[process_id].is_completed = true;
                printf("Node %d: Process %d completed\n", node->node_id, process_id);
            }
        } else {
            printf("Node %d: Process %d request denied\n", node->node_id, process_id);
        }
        
        // Update priorities
        update_priorities(node);
        
        // Sleep for a random time
        Sleep(rand() % 1000);
    }
    
    return 0;
}

int main() {
    // Initialize nodes
    Node nodes[MAX_NODES];
    HANDLE threads[MAX_NODES * 2]; // One for process simulator, one for message handler
    
    // Initialize nodes with different user data
    init_node_with_data(&nodes[0], 0, user1_processes, 3);
    init_node_with_data(&nodes[1], 1, user2_processes, 3);
    init_node_with_data(&nodes[2], 2, user3_processes, 3);
    
    // Create threads for each node
    for (int i = 0; i < MAX_NODES; i++) {
        threads[i * 2] = CreateThread(NULL, 0, process_simulator, &nodes[i], 0, NULL);
        threads[i * 2 + 1] = CreateThread(NULL, 0, message_handler, &nodes[i], 0, NULL);
    }
    
    // Main loop for monitoring
    while (1) {
        for (int i = 0; i < MAX_NODES; i++) {
            print_state(&nodes[i]);
        }
        printf("\n---\n");
        Sleep(5000);
    }
    
    // Wait for all threads to complete (though they run indefinitely)
    WaitForMultipleObjects(MAX_NODES * 2, threads, TRUE, INFINITE);
    
    // Close thread handles
    for (int i = 0; i < MAX_NODES * 2; i++) {
        CloseHandle(threads[i]);
    }
    
    return 0;
} 
