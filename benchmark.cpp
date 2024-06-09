
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
  collector.start();
  float ts = function(repeat, float_1, float_2);
  if (ts < 0)
  {
    printf("bug\n");
  }
  event_count allocate_count = collector.end();
  aggregate << allocate_count;
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

void process_divide_floats(size_t repeat, float float_1, float float_2, std::string name)
{

  write_to_output(repeat, time_it_ns(divide_floats, repeat, float_1, float_2), name, float_1, float_2);
  printf("\n");
}

void process_multiply_floats(size_t repeat, float float_1, float float_2, std::string name)
{

  write_to_output(repeat, time_it_ns(multiply_floats, repeat, float_1, float_2), name, float_1, float_2);
  printf("\n");
}

void process_add_floats(size_t repeat, float float_1, float float_2, std::string name)
{

  write_to_output(repeat, time_it_ns(add_floats, repeat, float_1, float_2), name, float_1, float_2);
  printf("\n");
}

void process_sub_floats(size_t repeat, float float_1, float float_2, std::string name)
{

  write_to_output(repeat, time_it_ns(sub_floats, repeat, float_1, float_2), name, float_1, float_2);
  printf("\n");
}

int main()
{
  float args[10] = {0.0, 1.0, 255, 256, 257, 1.0e-42, 1.0e-41, 1.0e-30, 1.0e30, 1.0e10};
  float float_1;
  float float_2;
  size_t repeat = 1000000;

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_divide_floats(repeat, float_1, float_2, "Divide");
      process_divide_floats(repeat, float_2, float_1, "Divide");
    }
  }

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_multiply_floats(repeat, float_1, float_2, "Mulitply");
      process_multiply_floats(repeat, float_2, float_1, "Multiply");
    }
  }

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_add_floats(repeat, float_1, float_2, "Add");
      process_add_floats(repeat, float_2, float_1, "Add");
    }
  }

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_sub_floats(repeat, float_1, float_2, "Sub");
      process_sub_floats(repeat, float_2, float_1, "Sub");
    }
  }

  asm("vmsr fpscr,%0" ::"r"(1 << 24));
  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_divide_floats(repeat, float_1, float_2, "DivideFTZ");
      process_divide_floats(repeat, float_2, float_1, "DivideFTZ");
    }
  }

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_multiply_floats(repeat, float_1, float_2, "MulitplyFTZ");
      process_multiply_floats(repeat, float_2, float_1, "MultiplyFTZ");
    }
  }

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_add_floats(repeat, float_1, float_2, "AddFTZ");
      process_add_floats(repeat, float_2, float_1, "AddFTZ");
    }
  }

  for (size_t i = 0; i < 10; i++)
  {
    float_1 = args[i];
    for (size_t j = 0; j < 10; j++)
    {
      float_2 = args[j];
      process_sub_floats(repeat, float_1, float_2, "SubFTZ");
      process_sub_floats(repeat, float_2, float_1, "SubFTZ");
    }
  }

  if (!collector.has_events())
  {
    std::cerr << "I cannot access the performance counters. Make sure you run the program in privileged mode (e.g., sudo) under Linux our macOS/ARM." << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}