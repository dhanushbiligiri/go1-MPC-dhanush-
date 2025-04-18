// Copyright 2022 DeepMind Technologies Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MJPC_UTILITIES_H_
#define MJPC_UTILITIES_H_

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include <mujoco/mujoco.h>

namespace mjpc {

// maximum number of traces that are visualized
inline constexpr int kMaxTraces = 99;

// set mjData state
void SetState(const mjModel* model, mjData* data, const double* state);

// get mjData state
void GetState(const mjModel* model, const mjData* data, double* state);

// get numerical data from a custom element in mjModel with the given name
double* GetCustomNumericData(const mjModel* m, std::string_view name);

// get a scalar value from a custom element in mjModel with the given name
template <typename T>
std::optional<T> GetNumber(const mjModel* m, std::string_view name) {
  double* data = GetCustomNumericData(m, name);
  if (data) {
    return static_cast<T>(data[0]);
  } else {
    return std::nullopt;
  }
}

// get a single numerical value from a custom element in mjModel, or return the
// default value if a custom element with the specified name does not exist
template <typename T>
T GetNumberOrDefault(T default_value, const mjModel* m, std::string_view name) {
  return GetNumber<T>(m, name).value_or(default_value);
}

// Clamp x between bounds, e.g., bounds[0] <= x[i] <= bounds[1]
void Clamp(double* x, const double* bounds, int n);

// get sensor data using string
double* SensorByName(const mjModel* m, const mjData* d,
                     const std::string& name);

int CostTermByName(const mjModel* m, const mjData* d, const std::string& name);

// get traces from sensors
void GetTraces(double* traces, const mjModel* m, const mjData* d,
               int num_trace);

// get keyframe data using string
double* KeyFrameByName(const mjModel* m, const mjData* d,
                       const std::string& name);

// return a power transformed sequence
void PowerSequence(double* t, double t_step, double t1, double t2, double p,
                   double N);

// find interval in monotonic sequence containing value
void FindInterval(int* bounds, const double* sequence, double value,
                  int length);

// zero-order interpolation
void ZeroInterpolation(double* output, double x, const double* xs,
                       const double* ys, int dim, int length);

// linear interpolation
void LinearInterpolation(double* output, double x, const double* xs,
                         const double* ys, int dim, int length);

// coefficients for cubic interpolation
void CubicCoefficients(double* coefficients, double x, const double* xs, int T);

// finite-difference vector
double FiniteDifferenceSlope(double x, const double* xs, const double* ys,
                             int dim, int length, int i);

// cubic polynomial interpolation
void CubicInterpolation(double* output, double x, const double* xs,
                        const double* ys, int dim, int length);

// returns the path to the directory containing the current executable
std::string GetExecutableDir();

// returns path to a model XML file given path relative to models dir
std::string GetModelPath(std::string_view path);

// dx = (x2 - x1) / h
void Diff(mjtNum* dx, const mjtNum* x1, const mjtNum* x2, mjtNum h, int n);

// finite-difference two state vectors ds = (s2 - s1) / h
void StateDiff(const mjModel* m, mjtNum* ds, const mjtNum* s1, const mjtNum* s2,
               mjtNum h);

// set x to be the point on the segment [p0 p1] that is nearest to x
void ProjectToSegment(double x[3], const double p0[3], const double p1[3]);

// default cost colors
extern const float CostColors[10][3];

// plots - vertical line
void PlotVertical(mjvFigure* fig, double time, double min_value,
                  double max_value, int N, int index);

// plots - update data
void PlotUpdateData(mjvFigure* fig, double* bounds, double x, double y,
                    int length, int index, int x_update, int y_update,
                    double x_bound_lower);

// plots - reset
void PlotResetData(mjvFigure* fig, int length, int index);

// plots - horizontal line
void PlotHorizontal(mjvFigure* fig, const double* xs, double y, int length,
                    int index);

// plots - set data
void PlotData(mjvFigure* fig, double* bounds, const double* xs,
              const double* ys, int dim, int dim_limit, int length,
              int start_index, double x_bound_lower);

// number of available hardware threads
int NumAvailableHardwareThreads();

// check mjData for warnings, return true if any warnings
bool CheckWarnings(mjData* data);

// compute vector with log-based scaling between min and max values
void LogScale(double* values, double max_value, double min_value, int steps);

// get a pointer to a specific element of a vector, or nullptr if out of bounds
template <typename T>
inline T* DataAt(std::vector<T>& vec, typename std::vector<T>::size_type elem) {
  if (elem < vec.size()) {
    return &vec[elem];
  } else {
    return nullptr;
  }
}

// increases the value of an atomic variable.
// in C++20 atomic::operator+= is built-in for floating point numbers, but this
// function works in C++11
inline void IncrementAtomic(std::atomic<double>& v, double a) {
  for (double t = v.load(); !v.compare_exchange_weak(t, t + a);) {}
}

// get a pointer to a specific element of a vector, or nullptr if out of bounds
template <typename T>
inline const T* DataAt(const std::vector<T>& vec,
                       typename std::vector<T>::size_type elem) {
  return DataAt(const_cast<std::vector<T>&>(vec), elem);
}

using UniqueMjData = std::unique_ptr<mjData, void (*)(mjData*)>;

inline UniqueMjData MakeUniqueMjData(mjData* d) {
  return UniqueMjData(d, mj_deleteData);
}

}  // namespace mjpc

#endif  // MJPC_UTILITIES_H_
