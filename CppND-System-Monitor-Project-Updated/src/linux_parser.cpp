#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  int memtotal, memfree, datavalue;
  string line, datakey;
  
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()){
    while(std::getline(stream, line)){
      std::stringstream linestream(line);
      while(linestream >> datakey >> datavalue){
        if(datakey == "MemTotal:"){
          memtotal = datavalue;
        }else if(datakey == "MemFree:"){
          memfree = datavalue;
        }
      }
    }
  }
  
  return (float)(memtotal - memfree) / (float)memtotal;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string data;
  
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    if(linestream >> data){
      return std::stol(data);
    }
  }
  return std::stol(data);
}

// Following resource was used to calculate CPU usage
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  string line, key;
  vector<string> storestat;
  
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    while(linestream.good()){
      std::getline(linestream, key, ' ');
      storestat.emplace_back(key);
    }
  }
  return (stoi(storestat[13]) + stoi(storestat[14]) + stoi(storestat[15]) + stoi(storestat[16]));
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> jiffies = LinuxParser::CpuUtilization();
  return stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_])
    + stol(jiffies[CPUStates::kSystem_]) + stol(jiffies[CPUStates::kIRQ_])
    + stol(jiffies[CPUStates::kSoftIRQ_]) + stol(jiffies[CPUStates::kSteal_]);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> jiffies = LinuxParser::CpuUtilization();
  return stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, key;
  vector<string> jiffies(CPUStates::kGuestNice_ + 1);
  
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        if(key == "cpu"){
          linestream >> jiffies[CPUStates::kUser_] 
            		 >> jiffies[CPUStates::kNice_]
            		 >> jiffies[CPUStates::kSystem_]
            		 >> jiffies[CPUStates::kIdle_]
            		 >> jiffies[CPUStates::kIOwait_]
            		 >> jiffies[CPUStates::kIRQ_]
            		 >> jiffies[CPUStates::kSoftIRQ_]
            		 >> jiffies[CPUStates::kSteal_]
            		 >> jiffies[CPUStates::kGuest_]
            		 >> jiffies[CPUStates::kGuestNice_];
        }
      }
    }
  }
  return jiffies;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key;
  int processes;
  
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while(getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key >> processes){
        if(key == "processes"){
          return processes;
        }
      }
    }
  }
  return processes;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key;
  int running_processes;
  
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
    while(getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key >> running_processes){
        if(key == "procs_running"){
          return running_processes;
        }
      }
    }
  }
  return running_processes;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line;
  
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if(stream.is_open()){
    std::getline(stream, line);
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line, key, size, unit;
  long unitInMb = 0;
  
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if(stream.is_open()){
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key >> size >> unit){
        if(key == "VmSize:"){
          unitInMb = std::stol(size) / float(1000.0);
          return std::to_string(unitInMb);
        }
      }
    }
  }
  return std::to_string(unitInMb);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line, key, userid;
  
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if(stream.is_open()){
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key >> userid){
        if(key == "Uid:"){
          return userid;
        }
      }
    }
  }
  return userid;
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string line, username, userid;
  
  std::ifstream stream(kPasswordPath);
  if(stream.is_open()){
    while(std::getline(stream, line)){
      std::replace(line.begin(), line.end(), ':', ' ');
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::istringstream linestream(line);
      while(linestream >> username >> userid){
        if(userid == LinuxParser::Uid(pid)){
          return username;
        }
      }
    }
  }
  return username;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string line, key;
  long uptime;
  vector<string> stateinfo;
  
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(stream.is_open()){
    while(std::getline(stream, line)){
      std::istringstream linestream(line);
      while(linestream >> key){
        stateinfo.emplace_back(key);
      }
    }
  }
  uptime = LinuxParser::UpTime() - std::stol(stateinfo[21]) / sysconf(_SC_CLK_TCK);
  return uptime;
}
