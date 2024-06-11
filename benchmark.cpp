
#include "performancecounters/event_counter.h"
#include <algorithm>
#include <charconv>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale.h>
#include <random>
#include <sstream>
#include <stdio.h>
#include <vector>

#if defined(__linux__) && defined(__x86_64__)
#include <xmmintrin.h>
#include <pmmintrin.h>

#elif __APPLE__ && __aarch64__
#include <fenv.h>

#endif

double multiply_doubles(size_t repeat, double double_1, double double_2)
{

  double res;

  for (size_t i = 0; i < repeat; i++)
  {

    res = double_1 * double_2;
  }

  return res;
}

double divide_doubles(size_t repeat, double double_1, double double_2)
{
  double res;

  for (size_t i = 0; i < repeat; i++)
  {

    res = double_1 / double_2;
  }

  return res;
}

double add_doubles(size_t repeat, double double_1, double double_2)
{
  double res;

  for (size_t i = 0; i < repeat; i++)
  {

    res = double_1 + double_2;
  }

  return res;
}

float sub_doubles(size_t repeat, double double_1, double double_2)
{

  double res;
  for (size_t i = 0; i < repeat; i++)
  {

    res = double_1 - double_2;
  }

  return res;
}

float multiply_floats(size_t repeat, float float_1, float float_2)
{

  float res;

  for (size_t i = 0; i < repeat; i++)
  {

    res = float_1 * float_2;
  }

  return res;
}

float divide_floats(size_t repeat, float float_1, float float_2)
{
  float res;

  for (size_t i = 0; i < repeat; i++)
  {

    res = float_1 / float_2;
  }

  return res;
}

float add_floats(size_t repeat, float float_1, float float_2)
{
  float res;

  for (size_t i = 0; i < repeat; i++)
  {

    res = float_1 + float_2;
  }

  return res;
}

float sub_floats(size_t repeat, float float_1, float float_2)
{

  float res;

  for (size_t i = 0; i < repeat; i++)
  {

    res = float_1 - float_2;
  }

  return res;
}

event_collector collector;

template <class T>
event_aggregate time_it_ns(T const &function, size_t repeat, float float_1, float float_2)
{
  event_aggregate aggregate{};

  // warm up the cache:
  for (size_t i = 0; i < 1000; i++)
  {
    float ts = function(1, float_1, float_2);
    if (ts < 0)
    {
      printf("bug\n");
    }
  }
  // for (size_t i = 0; i < repeat; i++) {
  collector.start();
  float ts = function(repeat, float_1, float_2);
  if (ts < 0)
  {
    printf("bug\n");
  }
  event_count allocate_count = collector.end();
  aggregate << allocate_count;
  // }
  return aggregate;
}

void pretty_print(size_t number_of_floats, std::string name,
                  event_aggregate aggregate)
{
  (void)number_of_floats;
  printf(" %32s ", name.c_str());
  printf(" %8.2f ns ", aggregate.elapsed_ns());

  printf(" %8.2f instructions ", aggregate.instructions());
  printf(" %16s ", "");
  printf(" %8.2f cycle count /operations ", aggregate.cycles() / number_of_floats);
  printf("\n");
  printf(" %32s ", "");
  printf(" %8.2f abs cycle count  ", aggregate.cycles());
  printf("\n");
  printf(" %32s ", "");
  printf(" %8.2f best cycle count  ", aggregate.best.cycles() / number_of_floats);
  printf("\n");

  printf(" %32s ", "");
  printf(" %8.2f branches ", aggregate.best.branches());
  printf("\n");

  printf(" %32s ", "");
  printf(" %8.2f branch miss  ", aggregate.best.branch_misses());
  printf(" %32s ", "");
  printf(" %8.2f instructions/cycle ",
         aggregate.best.instructions() / aggregate.best.cycles());
}

void write_to_output(size_t repeat, event_aggregate aggregate, std::string name, float float_1, float float_2)
{
  std::ofstream output;
  output.open(name + ".txt", std::ios_base::app);

  output << name << " " << aggregate.cycles() / repeat << " " << aggregate.best.instructions() / aggregate.best.cycles() << " " << float_1 << " " << float_2 << "\n";
  output.close();
}

template <class F>
void process(size_t repeat, float float_1, float float_2, std::string name, F const &function)
{
  write_to_output(repeat, time_it_ns(function, repeat, float_1, float_2), name, float_1, float_2);
  printf("\n");
}
template <class D>
void process_doubles(size_t repeat, double double_1, double double_2, std::string name, D const &function)
{
  write_to_output(repeat, time_it_ns(function, repeat, double_1, double_2), name, double_1, double_2);
}

int main()
{
  size_t len = 10;
  size_t len_double = 12;
  float args[10] = {0.0, 1.0, 255, 256, 257, 1.0e-42, 1.0e-41, 1.0e-30, 1.0e30, 1.0e10};
  double args_double[12] = {0.0, 1.0, 255, 256, 257, 1.0e-42, 1.0e-41, 1.0e-30, 1.0e30, 1.0e10, 1.0e200, 1.0e-300};
  float float_1;
  float float_2;
  double double_1;
  double double_2;
  size_t repeat = 4000000;

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "Multiply", multiply_floats);
    }
  }

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "Divide", divide_floats);
    }
  }

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "Add", add_floats);
    }
  }

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "Sub", sub_floats);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "MultiplyDouble", multiply_doubles);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "DivideDouble", divide_doubles);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "AddDouble", add_doubles);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "SubDouble", sub_doubles);
    }
  }

// enable ftz/daz on different platforms
#if defined(__linux__) && defined(__x86_64__)
  printf("linux x86");
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
  _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#elif __linux__ && __arm__
  printf("linux arm");
  asm("vmsr fpscr,%0" ::"r"(1 << 24));
#elif __APPLE__ && __aarch64__
  int r = fesetenv(FE_DFL_DISABLE_DENORMS_ENV);
  if (r != 0)
  {
    fprintf(stderr, "fesetenv returned %d\n", r);
    return -1;
  }
  printf("fesetenv returned %d\n", r);
#endif

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "MultiplyFTZ", multiply_floats);
    }
  }

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "DivideFTZ", divide_floats);
    }
  }

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "AddFTZ", add_floats);
    }
  }

  for (size_t i = 0; i < len; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < len; j++)
    {
      float_2 = args[j];
      process(repeat, float_1, float_2, "SubFTZ", sub_floats);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "MultiplyDoubleFTZ", multiply_doubles);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "DivideDoubleFTZ", divide_doubles);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "AddDoubleFTZ", add_doubles);
    }
  }

  for (size_t i = 0; i < len_double; i++)
  {
    double_1 = args_double[i];
    for (size_t j = 0; j < len_double; j++)
    {
      double_2 = args_double[j];
      process_doubles(repeat, double_1, double_2, "SubDoubleFTZ", sub_doubles);
    }
  }

  if (!collector.has_events())
  {
    std::cerr << "I cannot access the performance counters. Make sure you run the program in privileged mode (e.g., sudo) under Linux our macOS/ARM." << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}