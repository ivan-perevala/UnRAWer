#include <iomanip>
#include <sstream>

#include "unrawer/timer.hpp"

using unrw::Timer, std::chrono::high_resolution_clock, std::chrono::duration, std::chrono::duration_cast;

Timer::Timer() : start_(high_resolution_clock::now()) {}

Timer::~Timer() {}

template <typename T> T Timer::now() const {
  high_resolution_clock::time_point end = high_resolution_clock::now();
  duration<T> dur = duration_cast<duration<T>>(end - this->start_);

  return dur.count();
};

const std::string Timer::nowText(int w, int p) const {
  std::stringstream ss;
  ss << std::fixed << std::setw(w) << std::setprecision(p) << this->now<float>() << " sec";
  return ss.str();
}

std::ostream &unrw::operator<<(std::ostream &os, const Timer &timer) {
  const float value = timer.now<float>();

  const auto precision = os.precision();
  const auto width = os.width();

  os << std::fixed << std::setw(0) << std::setprecision(6) << value << std::setw(width) << std::setprecision(precision);

  return os;
}