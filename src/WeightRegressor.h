#ifndef WEIGHT_REGRESSOR_H
#define WEIGHT_REGRESSOR_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cmath>
#endif

/**
 * WeightRegressor Class
 * 
 * Filters out high-frequency scale noise and vibrations caused by moving parts
 * and motor activity, without introducing the phase lag (delay) of a standard
 * moving average filter.
 * 
 * It fits a linear trend line (ordinary least squares regression) through the
 * last N measurements over time and predicts the current actual weight.
 */
class WeightRegressor {
private:
  static const int N = 10; // Size of moving window (optimized for ultra-constant single/double bean flow)
  unsigned long times[N];
  float weights[N];
  int count = 0;
  int head = 0;

public:
  void reset() {
    count = 0;
    head = 0;
  }

  void addSample(unsigned long t, float w) {
    if (count < N) {
      times[count] = t;
      weights[count] = w;
      count++;
    } else {
      times[head] = t;
      weights[head] = w;
      head = (head + 1) % N;
    }
  }

  float getLatestRaw() const {
    if (count == 0) return 0.0;
    int prevIdx = (head + count - 1) % N;
    return weights[prevIdx];
  }

  int getCount() const {
    return count;
  }

  float predict(unsigned long targetTime) {
    if (count < 4) {
      // Not enough points for a reliable linear fit, use latest raw value
      if (count > 0) {
        return getLatestRaw();
      }
      return 0.0;
    }

    // Subtract the oldest timestamp to center values around 0, protecting precision
    unsigned long t0 = times[head];
    
    double sumT = 0;
    double sumW = 0;
    double sumTW = 0;
    double sumT2 = 0;

    for (int i = 0; i < count; i++) {
      int idx = (head + i) % N;
      double t = (double)(times[idx] - t0);
      double w = (double)weights[idx];
      sumT += t;
      sumW += w;
      sumTW += t * w;
      sumT2 += t * t;
    }

    double denom = (count * sumT2 - sumT * sumT);
#ifdef ARDUINO
    if (abs(denom) < 1.0) {
#else
    if (std::abs(denom) < 1.0) {
#endif
      // If time interval is zero, fall back to average
      return sumW / count;
    }

    double m = (count * sumTW - sumT * sumW) / denom;
    double c = (sumW - m * sumT) / count;

    // Evaluate at target time
    double t_target = (double)(targetTime - t0);
    double predicted = m * t_target + c;

    // Safety checks against outliers/fleeting spikes during active dispensing
    float latestRaw = getLatestRaw();
    if (predicted > latestRaw + 1.5) {
      predicted = latestRaw + 1.5;
    } else if (predicted < latestRaw - 1.5) {
      predicted = latestRaw - 1.5;
    }

    if (predicted < 0.0) {
      predicted = 0.0;
    }

    return (float)predicted;
  }
};

#endif // WEIGHT_REGRESSOR_H
