#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the maximum length of a line in the trace file
#define MAX_LINE_LENGTH 100

// ISR Vector Table structure
typedef struct {
    int interrupt_number;     // e.g., 3
    int isr_address;          // e.g., 0x029C
    int initial_mem_address;  // e.g., 0x0002 (each vector is 2 bytes)
    int allocated_time;       // e.g., 100 
} VectorTable;

// Vector table (interrupt numbers, their corresponding ISR addresses, initial memory address, and their allocated times)
VectorTable vectors[] = {
    {0, 0x01E3, 0x0000, 110}, 
    {1, 0x029C, 0x0002, 100},
    {2, 0x0695, 0x0004, 150},
    {3, 0x042B, 0x0006, 300},
    {4, 0x0292, 0x0008, 250},
    {5, 0x048B, 0x000A, 211},
    {6, 0x0639, 0x000C, 265},
    {7, 0x00BD, 0x000E, 152},
    {8, 0x06EF, 0x0010, 1000},
    {9, 0x036C, 0x0012, 156},
    {10, 0x07B0, 0x0014, 564},
    {11, 0x01F8, 0x0016, 523},
    {12, 0x03B9, 0x0018, 145},
    {13, 0x06C7, 0x001A, 636},
    {14, 0x0165, 0x001C, 456},
    {15, 0x0584, 0x001E, 68},
    {16, 0x02DF, 0x0020, 956},
    {17, 0x05B3, 0x0022, 235},
    {18, 0x060A, 0x0024, 123},
    {19, 0x0765, 0x0026, 652},
    {20, 0x07B7, 0x0028, 119},
    {21, 0x0523, 0x002A, 425},
    {22, 0x03B7, 0x002C, 345},
    {23, 0x028C, 0x002E, 182},
    {24, 0x05E8, 0x0030, 561},
    {25, 0x05D3, 0x0032, 500}
}; 

// Function to get the ISR address and initial memory address from the vector table
int get_isr_address(int interrupt_num, int *initial_mem_address);

// Function to get corresponding allocated time of an interrupt
int get_allocated_time(int interrupt_num);

// Function to simulate interrupt handling with time division based on percentages
void handle_interrupt(int interrupt_num, int *current_time, FILE *output, const char *type, int *toggle, int total_time);

// Function to process the trace file and log events
void process_trace(FILE *trace, FILE *output);

// Function to properly distribute allocated time per SYSCALL/ENDIO event.
static int spend(FILE *output, int *current_time, int *remaining_time, int request_time, const char *event_occurence);

#endif