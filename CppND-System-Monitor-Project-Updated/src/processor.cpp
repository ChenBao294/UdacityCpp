#include "processor.h"
#include "linux_parser.h"
#include <unistd.h>

// TODO: Return the aggregate CPU utilization
/*
PrevIdle = previdle + previowait
Idle = idle + iowait

PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal
NonIdle = user + nice + system + irq + softirq + steal

PrevTotal = PrevIdle + PrevNonIdle
Total = Idle + NonIdle

# differentiate: actual value minus the previous one
totald = Total - PrevTotal
idled = Idle - PrevIdle

CPU_Percentage = (totald - idled)/totald
*/
float Processor::Utilization() {
  // Pre
  long pretotal = LinuxParser::Jiffies();
  long preidle = LinuxParser::IdleJiffies();
  
  // sleep for 0.5 seconds
  usleep(500000);
  
  // current
  long total = LinuxParser::Jiffies();
  long idle = LinuxParser::IdleJiffies();
  
  // CPU_Percentage
  float totald = (float)total - (float)pretotal;
  float idled = (float)idle - (float)preidle;
  
  return (totald - idled) / totald;
}