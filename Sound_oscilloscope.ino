#include <Arduino.h>
#include <Adafruit_ZeroFFT.h>  // FFT library

#define MIC_PIN A0       
#define SAMPLE_SIZE 8   // Reduced sample size to save memory
#define SAMPLE_RATE 5000 // Sampling rate in Hz

int16_t samples[SAMPLE_SIZE]; // Single array for raw and processed data

void setup() {
    Serial.begin(9600);
  }


void loop() {
    // Step 1: Read Analog Signal with precise timing
    unsigned long startTime = micros();
    for (int i = 0; i < SAMPLE_SIZE; i++) {
        samples[i] = analogRead(MIC_PIN); // Read from microphone
        while (micros() - startTime < (i * 1000000UL / SAMPLE_RATE)) {
            // Wait for the next sample interval
        }
    }

    // Step 2: Apply Digital Filtering
    removeDCOffset(samples,SAMPLE_SIZE); // Remove DC offset
    applyHammingWindow(samples, SAMPLE_SIZE); // Apply windowing function
    applyMovingAverageFilter(samples, SAMPLE_SIZE, 3); // Smooth data

    // Step 3: Apply FFT
    ZeroFFT(samples, SAMPLE_SIZE); // Perform FFT on the same array

    // Step 4: Find the Peak Frequency
    float dominantFreq = findPeakFrequency(samples, SAMPLE_SIZE, SAMPLE_RATE);

    // Step 5: Display the Frequency
    Serial.print("Detected Frequency: ");
    Serial.print(dominantFreq);
    Serial.println(" Hz");

    delay(500); // Delay between measurements
}

// Function to remove DC offset (mean value)
void removeDCOffset(int16_t *data, int size) {
    int32_t sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    int16_t mean = sum / size;
    for (int i = 0; i < size; i++) {
        data[i] -= mean; // Remove DC offset
    }
}

// Apply Hamming window to reduce spectral leakage
void applyHammingWindow(int16_t *data, int size) {
    for (int i = 0; i < size; i++) {
        float window = 0.54 - 0.46 * cos(2 * PI * i / (size - 1));
        data[i] = (int16_t)(data[i] * window); // Apply window
    }
}

// Moving Average Filter (Smooths data)
void applyMovingAverageFilter(int16_t *data, int size, int windowSize) {
    for (int i = 0; i < size; i++) {
        int32_t sum = 0;
        int count = 0;
        for (int j = -windowSize / 2; j <= windowSize / 2; j++) {
            int index = i + j;
            if (index >= 0 && index < size) {
                sum += data[index];
                count++;
            }
        }
        data[i] = sum / count; // Update the same array
    }
}

// Function to find the peak frequency
float findPeakFrequency(int16_t *fftData, int size, float sampleRate) {
    int peakIndex = 1; // Start from 1 to ignore DC component
    int16_t peakValue = fftData[1];

    for (int i = 2; i < size / 2; i++) { // Only consider the first half of the FFT output
        if (fftData[i] > peakValue) {
            peakValue = fftData[i];
            peakIndex = i;
        }
    }

    return (peakIndex * sampleRate) / size;  // Convert index to frequency
}