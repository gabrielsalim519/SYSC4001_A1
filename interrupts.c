#include "interrupts.h"

int get_isr_address(int interrupt_number, int *initial_mem_address) {
    for (size_t i = 0; i < sizeof(vectors) / sizeof(VectorTable); i++) {
        if (vectors[i].interrupt_number == interrupt_number) {
            *initial_mem_address = vectors[i].initial_mem_address;  
            return vectors[i].isr_address;  
        }
    }
    *initial_mem_address = -1;  
    return -1; 
}

int get_allocated_time(int interrupt_number) {
    for (size_t i = 0; i < sizeof(vectors) / sizeof(VectorTable); i++) {
        if (vectors[i].interrupt_number == interrupt_number) {
            return vectors[i].allocated_time;
        }
    }
    return 0;
}

static int spend(FILE *output, int *current_time, int *remaining_time, int request_time, const char *event_occurence) {
    if (request_time < 0) {
        request_time = 0;
    }
    int taken_time = (*remaining_time >= request_time) ? request_time : (*remaining_time > 0 ? *remaining_time : 0);
    fprintf(output, "%d, %d, %s\n", *current_time, taken_time, event_occurence);
    *current_time += taken_time;
    *remaining_time -= taken_time;
    printf("\nRemaining: %d", *remaining_time);
    return taken_time;
}

void handle_interrupt(int interrupt_num, int *current_time, FILE *output, const char *type, int *toggle, int total_time) {
    const int priority_time = 1;   
    const int mask_time = 1;   
    const int kernel_time = 1;   
    const int context_time = 5;   
    const int vector_find_time = 1;   
    const int load_address_time = 1;  
    const int error_time = 1;   
    const int iret_time = 1;
    const int isr_used_time = 20;

    // const double isr_remaining_time_ratio = 0.40;
    // const double data_remaining_time_ratio = 0.60;

    int remaining_operation_time = total_time;
    
    if (remaining_operation_time < 0) {
        remaining_operation_time = 0;
    }

    const int is_endio = (strcmp(type, "END_IO") == 0);
    const int is_syscall = (strcmp(type, "SYSCALL") == 0);

    int pre_systemcall_time = 0;
    
    if (is_endio) {
        pre_systemcall_time += priority_time + mask_time;
    }
    
    pre_systemcall_time += kernel_time + context_time + vector_find_time + load_address_time;
    const int post_systemcall_time = error_time + iret_time;

    if (is_endio) {
        spend(output, current_time, &remaining_operation_time, priority_time, "check priority of interrupt");
        spend(output, current_time, &remaining_operation_time, mask_time, "check if masked");
    }

    spend(output, current_time, &remaining_operation_time, kernel_time,  "switch to kernel mode");
    spend(output, current_time, &remaining_operation_time, context_time, "context saved");

    int initial_mem_address;
    int isr_address = get_isr_address(interrupt_num, &initial_mem_address);
    
    if (isr_address != -1) {
        char buf1[128];
        snprintf(buf1, sizeof(buf1), "find vector %d in memory position 0x%04X", interrupt_num, (unsigned)initial_mem_address);
        spend(output, current_time, &remaining_operation_time, vector_find_time, buf1);

        char buf2[128];
        snprintf(buf2, sizeof(buf2), "load address 0x%04X into the PC", (unsigned)isr_address);
        spend(output, current_time, &remaining_operation_time, load_address_time, buf2);
    } 
    else {
        fprintf(output, "%d, %d, interrupt number %d not found in vector table\n", *current_time, 0, interrupt_num);
    }

    int payload = remaining_operation_time - post_systemcall_time;
    
    if (payload < 0) {
        payload = 0;
    }
    
    if (is_endio) {
        spend(output, current_time, &remaining_operation_time, payload, "END_IO");
    } 
    /////////////////////////////////////////////////////////////////////////////
    else if (is_syscall) {
        
        // double main_operation_remaining_time = isr_remaining_time_ratio + data_remaining_time_ratio;
        // double weighted_operation_avg = (main_operation_remaining_time > 0.0) ? (isr_remaining_time_ratio / main_operation_remaining_time) : 0.0;
        // int isr_time = (int)(payload * weighted_operation_avg);
        
        int isr_time;
        int transfer_time = payload - isr_used_time;

        if (transfer_time <= 0) {
            isr_time = payload;
            transfer_time = 0;
        }
        else
        {
            isr_time = isr_used_time;
        }

        spend(output, current_time, &remaining_operation_time, isr_time, "SYSCALL: run the ISR");
        const char *transfer_alternate = (*toggle == 0) ? "transfer data" : "transfer data to the display";
        spend(output, current_time, &remaining_operation_time, transfer_time, transfer_alternate);
        *toggle = !*toggle;
    } 
    /////////////////////////////////////////////////////////////////////////////
    spend(output, current_time, &remaining_operation_time, error_time, "check for errors");
    spend(output, current_time, &remaining_operation_time, iret_time,  "IRET");
}

void process_trace(FILE *trace, FILE *output) {
    char line[MAX_LINE_LENGTH];
    int current_time = 0;
    int toggle = 0;  

    while (fgets(line, sizeof(line), trace)) {
        char activity_with_num[20];
        int value;

        if (sscanf(line, " %19[^,] , %d", activity_with_num, &value) == 2) {
            size_t n = strlen(activity_with_num);
            while (n && activity_with_num[n-1] == ' ') activity_with_num[--n] = '\0';

            if (strncmp(activity_with_num, "CPU", 3) == 0) {
                int duration = value;
                fprintf(output, "%d, %d, CPU execution\n", current_time, duration);
                current_time += duration;
            } 
            else if (strncmp(activity_with_num, "SYSCALL", 7) == 0) {
                int interrupt_num = value;                   
                int duration = get_allocated_time(interrupt_num);
                handle_interrupt(interrupt_num, &current_time, output, "SYSCALL", &toggle, duration);
            } 
            else if (strncmp(activity_with_num, "END_IO", 6) == 0) {
                int interrupt_num = value;                  
                int duration = get_allocated_time(interrupt_num);
                handle_interrupt(interrupt_num, &current_time, output, "END_IO", &toggle, duration);
            }
        }
    }
}

int main(void) {
    FILE *trace = fopen("trace.txt", "r");
    if (trace == NULL) {
        perror("Error opening trace file");
        return EXIT_FAILURE;
    }
    
    FILE *output = fopen("execution.txt", "w");
    if (output == NULL) {
        perror("Error opening output file");
        fclose(trace);
        return EXIT_FAILURE;
    }
    process_trace(trace, output);
    fclose(trace);
    fclose(output);
    return 0;
}

/*********************************************************************
gcc -std=c11 -Wall -Wextra interrupts.c interrupts.h -o interrupts.exe
./interrupts.exe
**********************************************************************/