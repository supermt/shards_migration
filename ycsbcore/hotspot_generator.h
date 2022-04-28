//
//  zipfian_generator.h
//  YCSB-cpp
//
//  Copyright (c) 2020 Youngjae Lee <ls4154.lee@gmail.com>.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#ifndef YCSB_C_HOTSPOT_GENERATOR_H_
#define YCSB_C_HOTSPOT_GENERATOR_H_

#include <cassert>
#include <cmath>
#include <cstdint>
#include <mutex>

#include "generator.h"
#include "utils.h"

namespace ycsbc {

class HotSpotGenerator : public Generator<uint64_t> {
 public:
  long lower_bound;
  long upper_bound;
  long hot_interval;
  long cold_interval;
  double hot_set_frac;
  double hot_opn_frac;

  HotSpotGenerator(long lowerBound, long upperBound, double hotsetFraction,
                   double hotOpnFraction) {
    if (hotsetFraction < 0.0 || hotsetFraction > 1.0) {
      hotsetFraction = 0.0;
    }
    if (hotOpnFraction < 0.0 || hotOpnFraction > 1.0) {
      hotOpnFraction = 0.0;
    }
    if (lowerBound > upperBound) {
      long temp = lowerBound;
      lowerBound = upperBound;
      upperBound = temp;
    }
    lower_bound = lowerBound;
    upper_bound = upperBound;
    hot_set_frac = hotsetFraction;
    long interval = upperBound - lowerBound + 1;
    hot_interval = (long)(interval * hotsetFraction);
    cold_interval = interval - hot_interval;
    hot_opn_frac = hotOpnFraction;
    NextValue();
  }

  uint64_t NextValue();
  uint64_t Next() { return NextValue(); }

  uint64_t Last();

  std::mutex mutex_;
  uint64_t last_value_;
  void setLastValue(long value);
};

inline uint64_t HotSpotGenerator::NextValue() {
  std::lock_guard<std::mutex> lock(mutex_);
  long value = 0;
  double random_value = utils::ThreadLocalRandomDouble();
  if (random_value < hot_opn_frac) {
    // Choose a value from the hot set.
    value =
        lower_bound + std::abs(utils::ThreadLocalRandomLong()) % hot_interval;
  } else {
    // Choose a value from the cold set.
    value =
        lower_bound + std::abs(utils::ThreadLocalRandomLong()) % cold_interval;
  }
  setLastValue(value);
  return value;
}

inline uint64_t HotSpotGenerator::Last() { return last_value_; }
void HotSpotGenerator::setLastValue(long value) { last_value_ = value; }

}  // namespace ycsbc

#endif  // YCSB_C_HOTSPOT_GENERATOR_H_
