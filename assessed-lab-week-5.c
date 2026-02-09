#include <fcntl.h>      // open()
#include <stdio.h>      // printf(), stderr
#include <stdint.h>     // uint32_t
#include <unistd.h>     // usleep()
#include <sys/mman.h>   // mmap()

// GPIO base address for RasberryPi 4
#define GPIO_BASE 0xFE200000UL

// GPIO block size is 4kB to be memory mapped
#define GPIO_BLOCK_SIZE 4096

// Defining pin numbers for each LED in the traffic light
#define RED_LED 10
#define YELLOW_LED 11
#define GREEN_LED 13

// Set offsets for each action (selecting, setting, and clearing)
#define GPFSEL_OFF 1
#define GPSET_OFF 7
#define GPCLR_OFF 10

// Delay in microseconds for in-between phases (500ms)
#define DELAY 500000

volatile uint32_t *gpio;
unsigned short getGpio();
void gpioSet(const int offset, int value);

int main(int argc, char *argv[]) {
    // Getting an address to the device memory mapped to gpio
    printf("LOG: Getting gpio\n");
    unsigned short result = getGpio();

    switch (result) {
        case 1:
            fprintf(stderr, "ERR: Unable to open \"/dev/mem\"\nAre you root?\n");
            break;
        case 2:
            fprintf(stderr, "ERR: Unable to map \"/dev/mem\" to gpio\n");
            break;
    }
    
    if (result != 0) return result;

    printf("LOG: Setting mode for each pin\n");
    
    // Set output mode forGPIO pins 10, 11, and 13
    printf("*(gpio+1): %b", *(gpio + GPFSEL_OFF));
    *(gpio + GPFSEL_OFF) = 0b001000001001;

    // Blink red light
    while (1) {
        // Set RED_LED (10) to high
        *(gpio + GPSET_OFF) = 0b1;

        // Halt program for $DELAY microseconds
        // Reference: https://pubs.opengroup.org/onlinepubs/009696899/functions/usleep.html
        usleep(DELAY);
        
        // Clear RED_LED (set to low)
        *(gpio + GPCLR_OFF) = 0b1;

        // Delay
        usleep(DELAY);
    }
    
    return 0;
}

// Function to memory-map "/dev/mem" to an gpio
unsigned short getGpio() {
    printf("LOG: Opening \"/dev/mem\" now\n");
    // Opening the /dev/memory device to map it to gpio
    int fileDescriptor = open(
        "/dev/mem", // Address where memory exists
        O_RDWR | O_SYNC | O_CLOEXEC // Flags: Read/Write mode + syncronised data access + close upon execution
    );

    printf("LOG: fileDescriptor: %d\n", fileDescriptor);
    // If there is an error opening the file descriptor
    if (fileDescriptor < 0)     
        return 1;
    
    // Mapping the /dev/mem to gpio
    gpio = (volatile uint32_t *) mmap(
        0,                     // Let any memory location chosen
        GPIO_BLOCK_SIZE,        // Size of memory to map
        PROT_READ | PROT_WRITE, // Writing and reading protection
        MAP_SHARED,             // Shared mapping
        fileDescriptor,         // Where to read data from
        GPIO_BASE               // The base GPIO address
    );

    // In case of error mapping gpio memory
    if (gpio == MAP_FAILED) {
        perror("mmap");
        close(fileDescriptor);
        return 2;
    }

    // Success
    return 0;
}

// Function to set pins to high
void gpioSet(const int offset, int value) {
    *(gpio + offset) = value;
}
