#include <iostream>
#include <cassert>
#include <cmath>
#include "../src/WeightRegressor.h"

// Color codes for pretty terminal prints
#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_CYAN    "\033[36m"
#define ANSI_COLOR_YELLOW  "\033[33m"

void runTest(const char* testName, bool (*testFunc)()) {
    std::cout << ANSI_COLOR_CYAN << "[ RUN      ] " << ANSI_COLOR_RESET << testName << std::endl;
    try {
        if (testFunc()) {
            std::cout << ANSI_COLOR_GREEN << "[       OK ] " << ANSI_COLOR_RESET << testName << std::endl;
        } else {
            std::cout << ANSI_COLOR_RED << "[   FAILED ] " << ANSI_COLOR_RESET << testName << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << ANSI_COLOR_RED << "[ EXCEPTION ] " << ANSI_COLOR_RESET << testName 
                  << " (message: " << e.what() << ")" << std::endl;
    } catch (...) {
        std::cout << ANSI_COLOR_RED << "[ EXCEPTION ] " << ANSI_COLOR_RESET << testName 
                  << " (unknown error)" << std::endl;
    }
}

// 1. Test Initial State / Small Datasets
bool test_initial_state_and_small_dataset() {
    WeightRegressor wr;
    wr.reset();
    
    // With 0 samples, prediction should be 0.0
    if (std::abs(wr.predict(100)) > 0.001) return false;
    
    // Add 1 sample - prediction should fall back to getLatestRaw
    wr.addSample(100, 2.5f);
    if (std::abs(wr.predict(150) - 2.5f) > 0.001) return false;
    
    // Add 2 and 3 samples - still < 4, should fall back to latest raw sample
    wr.addSample(200, 3.0f);
    wr.addSample(300, 3.5f);
    if (std::abs(wr.predict(350) - 3.5f) > 0.001) return false;
    
    return true;
}

// 2. Test Perfect Linear Progression
bool test_perfect_linear_progression() {
    WeightRegressor wr;
    wr.reset();
    
    // Add 5 perfectly linear samples (y = 0.5 * t)
    // Rate of dispense = 0.01g per millisecond (10g per second)
    unsigned long baseTime = 1000;
    for (int i = 0; i < 5; i++) {
        unsigned long t = baseTime + i * 100; // 100, 200, 300ms...
        float w = i * 1.0f; // 0g, 1g, 2g, 3g...
        wr.addSample(t, w);
    }
    
    // Trend line: w = 0.01 * (t - 1000)
    // Predict at t = 1500 (which is 100ms after the last sample at t=1400 which is 4.0g)
    // Expecting 5.0g
    float prediction = wr.predict(1500);
    float expected = 5.0f;
    
    if (std::abs(prediction - expected) > 0.01f) {
        std::cerr << "   Linear prediction fail. Got " << prediction << ", Expected " << expected << std::endl;
        return false;
    }
    return true;
}

// 3. Test High Frequency Vibration Noise Cancellation
bool test_noise_cancellation() {
    WeightRegressor wr;
    wr.reset();
    
    // Base progression is w = 0.01 * (t - 1000).
    // Let's add noise of +/- 0.5g to each measurement.
    unsigned long baseTime = 1000;
    float noise[8] = { 0.2f, -0.3f, 0.4f, -0.4f, 0.3f, -0.2f, 0.4f, -0.3f };
    
    for (int i = 0; i < 8; i++) {
        unsigned long t = baseTime + i * 100;
        float trueWeight = i * 1.0f; // 0g, 1, 2, 3, 4, 5, 6, 7g
        float noisyWeight = trueWeight + noise[i];
        wr.addSample(t, noisyWeight);
    }
    
    // Last sample was at t = 1700 (true = 7.0, noisy = 6.7)
    // Predict at current time t = 1700
    // A simple moving average would be biased by the local samples, but
    // a linear fit should reconstruct the slope (m ~ 0.01) and find a point close to 7.0.
    float prediction = wr.predict(1700);
    float expectedTrue = 7.0f;
    
    std::cout << "   [INFO] Noisy last sample: " << wr.getLatestRaw() 
              << "g | Predicted: " << prediction << "g (True expected: " << expectedTrue << "g)" << std::endl;
              
    // Tolerance is slightly higher due to noise, but should be much closer to true than to outliers
    if (std::abs(prediction - expectedTrue) > 0.5f) {
        return false;
    }
    return true;
}

// 4. Test Outlier Protection Clamp
bool test_outlier_clamp() {
    WeightRegressor wr;
    wr.reset();
    
    // Populate with stable steady line of 5.0g
    unsigned long baseTime = 1000;
    for (int i = 0; i < 7; i++) {
        wr.addSample(baseTime + i * 100, 5.0f);
    }
    
    // Now add a massive noise spike (e.g. coffee bean dropped/impact force or vibration glitch)
    // Scale registers a massive instantaneous jump to 10.0g at index 8.
    wr.addSample(baseTime + 700, 10.0f);
    
    // Without clamp, the linear regression slope would shoot up violently.
    // With clamp, the predicted weight is constrained to be within +/- 1.5g of the latest raw reading (10.0 -> [8.5, 11.5])
    // or if the regression is 5.0, it clamps to latestRaw - 1.5 = 8.5g or similar.
    float prediction = wr.predict(baseTime + 700);
    
    std::cout << "   [INFO] Raw with spike: 10.0g | Predicted: " << prediction << "g" << std::endl;
    
    // It should not shoot all the way to 5.0 (since raw is 10.0, clamp latestRaw - 1.5 = 8.5)
    // and it shouldn't allow the raw 10.0 to pass directly if regression was steady 5.
    if (prediction < 8.49f || prediction > 8.51f) {
        std::cerr << "   Outlier safety clamp failed to constrain prediction to safe band around latestRaw. Got " << prediction << std::endl;
        return false;
    }
    return true;
}

// 5. Test Ring Buffer Writs (Wrapping Around)
bool test_ring_buffer_wrap() {
    WeightRegressor wr;
    wr.reset();
    
    // Feed 15 samples, ensuring ring buffer wraps around several times (N = 10)
    unsigned long baseTime = 1000;
    for (int i = 0; i < 15; i++) {
        wr.addSample(baseTime + i * 100, i * 2.0f); // slope = 0.02
    }
    
    // The active window should contain the last 10 samples (i = 5 to 14)
    // i.e., times: [1500 to 2400], weights: [10.0 to 28.0]
    // Expect 10 samples
    if (wr.getCount() != 10) return false;
    
    // Predict at t = 2400 (corresponds to i = 14, expected = 28.0)
    float prediction = wr.predict(2400);
    if (std::abs(prediction - 28.0f) > 0.01f) {
        std::cerr << "   Wrapped buffer regression fail. Got " << prediction << " instead of 28.0" << std::endl;
        return false;
    }
    
    return true;
}

int main() {
    std::cout << ANSI_COLOR_YELLOW << "==============================================" << ANSI_COLOR_RESET << std::endl;
    std::cout << ANSI_COLOR_YELLOW << "   RUNNING UNIT TESTS FOR WEIGHT REGRESSOR    " << ANSI_COLOR_RESET << std::endl;
    std::cout << ANSI_COLOR_YELLOW << "==============================================" << ANSI_COLOR_RESET << std::endl;
    
    runTest("Test Initial State and Small Datasets", test_initial_state_and_small_dataset);
    runTest("Test Perfect Linear Progression", test_perfect_linear_progression);
    runTest("Test Noise Cancellation", test_noise_cancellation);
    runTest("Test Outlier Clamp (Vibration Spike Protection)", test_outlier_clamp);
    runTest("Test Ring Buffer Wrapping (Overwriting)", test_ring_buffer_wrap);
    
    std::cout << ANSI_COLOR_YELLOW << "==============================================" << ANSI_COLOR_RESET << std::endl;
    return 0;
}
