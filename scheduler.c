#include "banker.h"
#include <time.h>

// Structure to store deadlock prediction history
typedef struct {
    int process_id;
    int request[MAX_RESOURCES];
    bool was_safe;
    int timestamp;
} DeadlockHistory;

#define MAX_HISTORY 100
static DeadlockHistory history[MAX_HISTORY];
static int history_count = 0;

// Update priorities based on aging
void update_priorities(Node *node) {
    for (int i = 0; i < node->num_processes; i++) {
        if (!node->processes[i].is_completed) {
            // Increase priority of waiting processes
            node->processes[i].priority++;
        }
    }
}

// Get the process with highest priority
int get_highest_priority_process(Node *node) {
    int highest_priority = -1;
    int selected_process = -1;
    
    for (int i = 0; i < node->num_processes; i++) {
        if (!node->processes[i].is_completed && 
            node->processes[i].priority > highest_priority) {
            highest_priority = node->processes[i].priority;
            selected_process = i;
        }
    }
    
    return selected_process;
}

// Predict potential deadlock based on historical data
bool predict_deadlock(Node *node, int process_id, int *request) {
    // Simple rule-based prediction
    int total_resources_needed = 0;
    int total_available = 0;
    
    // Calculate total resources needed by all processes
    for (int i = 0; i < node->num_processes; i++) {
        if (!node->processes[i].is_completed) {
            for (int j = 0; j < node->num_resources; j++) {
                total_resources_needed += node->processes[i].need[j];
            }
        }
    }
    
    // Add the current request
    for (int i = 0; i < node->num_resources; i++) {
        total_resources_needed += request[i];
    }
    
    // Calculate total available resources
    for (int i = 0; i < node->num_resources; i++) {
        total_available += node->available[i];
    }
    
    // Check historical patterns
    int similar_unsafe_requests = 0;
    for (int i = 0; i < history_count; i++) {
        bool is_similar = true;
        for (int j = 0; j < node->num_resources; j++) {
            if (history[i].request[j] != request[j]) {
                is_similar = false;
                break;
            }
        }
        if (is_similar && !history[i].was_safe) {
            similar_unsafe_requests++;
        }
    }
    
    // Prediction rules
    if (total_resources_needed > total_available * 2) {
        return true; // High risk of deadlock
    }
    
    if (similar_unsafe_requests > 3) {
        return true; // Historical pattern suggests deadlock
    }
    
    return false;
}

// Update deadlock prediction history
void update_deadlock_history(Node *node, int process_id, bool was_safe) {
    if (history_count < MAX_HISTORY) {
        history[history_count].process_id = process_id;
        history[history_count].was_safe = was_safe;
        history[history_count].timestamp = time(NULL);
        
        // Store the request that led to this state
        for (int i = 0; i < node->num_resources; i++) {
            history[history_count].request[i] = node->processes[process_id].need[i];
        }
        
        history_count++;
    } else {
        // Remove oldest entry and add new one
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = history[i + 1];
        }
        history[MAX_HISTORY - 1].process_id = process_id;
        history[MAX_HISTORY - 1].was_safe = was_safe;
        history[MAX_HISTORY - 1].timestamp = time(NULL);
        
        for (int i = 0; i < node->num_resources; i++) {
            history[MAX_HISTORY - 1].request[i] = node->processes[process_id].need[i];
        }
    }
} 
