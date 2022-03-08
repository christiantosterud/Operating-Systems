#define _FILE_OFFSET_BITS 64

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include "dyn_array.h"
#include "processing_scheduling.h"
#include "bits/types.h"
#include <time.h>




// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)




//Sorts the given elements of a pointer in order from lowest to highest number of arrival time
int arrival_sort(const void * a, const void * b)
{
    ProcessControlBlock_t *blockA = (ProcessControlBlock_t *)a;
    ProcessControlBlock_t *blockB = (ProcessControlBlock_t *)b;
    return( blockA->arrival - blockB->arrival);
}

//Sorts the given elements of a pointer in order from lowest to highest number of time remaining
int time_remaining_sort(const void *a, const void *b)
{
    ProcessControlBlock_t *blockA = (ProcessControlBlock_t *)a;
    ProcessControlBlock_t *blockB = (ProcessControlBlock_t *)b;
    return( blockA->remaining_burst_time - blockB->remaining_burst_time);
}

//Sorts the given elements of a pointer in order from lowest to highest number of priority
int priority_sort(const void *a, const void *b)
{
    ProcessControlBlock_t *blockA = (ProcessControlBlock_t *)a;
    ProcessControlBlock_t *blockB = (ProcessControlBlock_t *)b;
    return( blockA->priority - blockB->priority);
}

//Is used after arrival time sort to determine which pcbs have arrived and are ready to be put on to the cpu
int arrived_pcbs(ProcessControlBlock_t *base, uint32_t t, uint32_t count)
{
    if((uint32_t)base->arrival > t) return count;
    else {
        count++;
        base++;
        return arrived_pcbs(base, t, count);
    } 
}


// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
    // decrement the burst time of the pcb
    --process_control_block->remaining_burst_time;
}

/**
 * @brief Runs the First Come First Served Process Scheduling algorithm over the incoming ready_queue
 * 
 * @param ready_queue a dyn_array of type ProcessControlBlock_t that contain be up to N elements
 * @param result used for first come first served stat tracking \ref ScheduleResult_t
 * @return true if function ran successful
 * @return false for an error
 */
bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if(ready_queue == NULL || result == NULL)
    {
        return false;
    }

    float total_Burst, total_Wait, deadTime = 0;
    int number_Jobs = 0;
    size_t queue_Size = dyn_array_capacity(ready_queue);
    // 1. Use of FIFO queue 
    bool flag = true;
    do {
        // Execute the front of the queue
        // Grabs the front/first process of the queue
        void* head = dyn_array_front(ready_queue);
        ProcessControlBlock_t * controlHead = (ProcessControlBlock_t *) head;
        if(controlHead->started == false)
        {
            controlHead->started = true;
            
            //calculate the wait time
            total_Wait = total_Burst - controlHead->arrival;

            // if nothing was in the queue yet after the a process was completed. 
            // The CPU would be doing nothing but waiting, therefore adding time to the total time to process all the PCBs
            if(controlHead->arrival > total_Burst)
            {
                deadTime = controlHead->arrival - total_Burst;
            }

            // Run the current process and get the burst time aka "run time"
            total_Burst += controlHead->remaining_burst_time;

            // Remove the process
            flag = dyn_array_pop_front(ready_queue);
            number_Jobs++;
        }
    } while (flag == true && number_Jobs < (int) queue_Size); // Another job in the queue
    
    result->average_waiting_time = total_Wait/number_Jobs;  // wait time = time between arrival of the process and the first time the process is scheduled to run on the CPU
    result->average_turnaround_time = total_Burst/number_Jobs; // turnaround time = time a process takes to complete
    result->total_run_time = total_Burst + deadTime; //total run time = total time for completion

    return true;
    
    // for(i = 0; i < (ready_queue->size); i++)
    // {
    //     void* head = dyn_array_front(ready_queue);
    //     ready_queue->size;
    // }
    // UNUSED(ready_queue);
    // UNUSED(result);
    // return false;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if(ready_queue == NULL || result == NULL)
    {
        return false;
    }
    int number_Jobs = 0;
    int queue_Size = dyn_array_capacity(ready_queue);
    bool flag = true;
    qsort(ready_queue, queue_Size, sizeof(ProcessControlBlock_t), time_remaining_sort);
    do {
        

    } while(flag == true && number_Jobs < (int) queue_Size);

    // UNUSED(ready_queue);
    UNUSED(result);
    return false;   
}

/**
 * @brief DO NOT NEED TO DO.
 * ANDERSEN SAID WE DO NOT NEED TO
 */
bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    UNUSED(ready_queue);
    UNUSED(result);
    return false;   
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
    /*if(ready_queue == NULL || result == NULL)
    {
        return false;
    }

    float total_Burst, total_Wait, deadTime = 0;
    int number_Jobs = 0;
    size_t queue_Size = dyn_array_capacity(ready_queue);
    // 1. Use of FIFO queue 
    bool flag = true;
    do {
        break;

    } while(flag == true && number_Jobs < (int) queue_Size)
*/



    //Need to use the bool started in this algorithm
    //Chnadra wouldn't use started he would use a something else from the struct
    //We can create our own variables in the struct
    UNUSED(ready_queue);
    UNUSED(result);
    UNUSED(quantum);
    return false;
}

// Reads the PCB burst time values from the binary file into ProcessControlBlock_t remaining_burst_time field
// for N number of PCB burst time stored in the file.
// \param input_file the file containing the PCB burst times
// \return a populated dyn_array of ProcessControlBlocks if function ran successful else NULL for an error
dyn_array_t *load_process_control_blocks(const char *input_file) 
{
    FILE *fp;
    if (input_file)
    {
        //open the file in read binary mode
        fp = fopen(input_file, "rb");
        //check if file opened successfully
        if (fp == NULL) return NULL;
        
        //find the size in bytes of the file
        fseek(fp, 0 , SEEK_END);
        long filelen = ftell(fp);

        //find the number of total uint32s in the file
        long NumofUint32Actual = filelen/sizeof(uint32_t);
        
        //set file pointer back to beginning of file so it can be read
        rewind(fp);

        //grab first uint32 to find number of processes
        uint32_t NumofProcesses = fread(&NumofProcesses, sizeof(uint32_t), 1, fp);

        //find the number of expected uint32s in the file
        long NumOfUint32Expected= 1 + 3 * (long)NumofProcesses;

        //Create array of pcbs to be put in dynamic array
        ProcessControlBlock_t * processes = (ProcessControlBlock_t*)malloc(sizeof(ProcessControlBlock_t)*NumofProcesses);
        
        int i;
        //check if file is still open and all parameters for each process are included in the file
        if (fp != NULL && (NumofUint32Actual == NumOfUint32Expected) ) 
        {
            for (i = 0; i < (int)NumofProcesses; i++)
            {
                fread(&processes[i].remaining_burst_time, sizeof(uint32_t), 1, fp);
                fread(&processes[i].priority, sizeof(uint32_t), 1, fp);
                fread(&processes[i].arrival, sizeof(uint32_t), 1, fp);
                processes[i].arrival = false;
            }
        }

        //Was using to double check uint32 values were being set correctly
        //printf("%" PRIu32 "\n",processes[0].remaining_burst_time); 

        // create dynamic array
        //dyn_array_t * readyQueue = dyn_array_create(NumofUint32Actual, (int)NumofProcesses, NULL);
        // Imports data read from PCB into dynamic array
        dyn_array_t * readyQueue = dyn_array_import(processes, (int)NumofProcesses, sizeof(ProcessControlBlock_t), NULL);
        //dyn_array_t * readyqueue = dyn_array_import(processes, sizeof(uint32_t), sizeof(ProcessControlBlock_t), dyn_Array_Process); 
        return readyQueue;
    }
    else return NULL;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if(ready_queue == NULL || result == NULL)
    {
        return false;
    }
    int number_Jobs = 0;
    int queue_Size = dyn_array_capacity(ready_queue);
    //Right now I am using an int that is getting incremented for the time not
    //sure if we are supposed to be using an actual timer but the concepts should
    //be the same
    int t = 0;
    bool flag = true;
    qsort(ready_queue, queue_Size, sizeof(ProcessControlBlock_t), arrival_sort);
    do {
        //Grab the front element in the queue
        ProcessControlBlock_t *head = dyn_array_front(ready_queue);


        //Count variable is used to indicate how many elements have arrived in the queue
        //based on the current time
        int count = 0;


        //arrived_pcbs is a recursive call to get the number of elements that have arrived
        count = arrived_pcbs(head, t, count);


        //qsort will sort the "count" of elements that have arrived
        //and align them based on remaining burst time
        qsort(head, count, sizeof(ProcessControlBlock_t), time_remaining_sort);


        //Set cur to the element that is ready to get on the cpu and 
        //has the shortest wait time
        ProcessControlBlock_t *cur = (ProcessControlBlock_t *)head;


        if(!cur->started) cur->timeStarted = t;
        //Need to setup a variable to track each ones exact start time due to
        // the fact that they will be getting moved on and off of the cpu
        if(cur->remaining_burst_time > 0) virtual_cpu(cur);
        //Need to ask some questions about the rest of this
        else {
            flag = dyn_array_pop_front(ready_queue);
            number_Jobs++;
            t++;
        }

        

    } while(flag == true && number_Jobs < (int) queue_Size);




   // UNUSED(ready_queue);
    UNUSED(result);
    return false;
}




