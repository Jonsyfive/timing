
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
// #include <fenv.h>
#include <vector>

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
  (void)repeat;
  float res;

  // for (size_t i = 0; i < repeat; i++) {

  res = float_1 - float_2;

  // }

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

void process_floats(size_t repeat, float float_1, float float_2, std::string name)
{
  // pretty_print(repeat, "Divide",
  //              time_it_ns( divide_floats, repeat));
  //  pretty_print(repeat, "Multiply",
  //  time_it_ns( multiply_doubles, repeat));
  write_to_output(repeat, time_it_ns(divide_floats, repeat, float_1, float_2), name, float_1, float_2);
  printf("\n");
}

// void process_doubles( size_t repeat, double double_1, double double_2) {
//   pretty_print(repeat, "Divide",
//                time_it_ns( divide_doubles, repeat));
//   write_to_output(repeat,time_it_ns( divide_doubles, repeat), "Divide");

// }

int main()
{
  float args[10] = {0.0, 1.0, 255, 256, 257, 1.0e-42, 1.0e-41, 1.0e-30, 1.0e30, 1.0e10};
  float float_1;
  float float_2;
  size_t repeat = 1000000000;
  // int r = fesetenv(FE_DFL_ENV);

  // if (r != 0)
  // {
  //   fprintf(stderr, "fesetenv returned %d\n", r);
  //   return -1;
  // }
  // printf("fesetenv returned %d\n", r);

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_floats(repeat, float_1, float_2, "Divide");
      process_floats(repeat, float_2, float_1, "Divide");
    }
  }

  // r = fesetenv(FE_DFL_DISABLE_DENORMS_ENV);

  // if (r != 0)
  // {
  //   fprintf(stderr, "fesetenv returned %d\n", r);
  //   return -1;
  // }
  // printf("fesetenv returned %d\n", r);
  // for (size_t i = 0; i < 10; i++)
  // {
  //   float_1 = args[i];
  //   for (size_t j = 0; j < 10; j++)
  //   {
  //     float_2 = args[j];
  //     process_floats(repeat, float_1, float_2, "DivideFTZ");
  //     process_floats(repeat, float_2, float_1, "DivideFTZ");
  //   }
  // }

  if (!collector.has_events())
  {
    std::cerr << "I cannot access the performance counters. Make sure you run the program in privileged mode (e.g., sudo) under Linux our macOS/ARM." << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}