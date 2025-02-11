#include "PolicyData.h"
#include "Debug.h"
#include "Command.h"
#include "Device.h"
#include "Kernel.h"
#include "Mem.h"
#include "Task.h"

namespace brisbane {
namespace rt {

PolicyData::PolicyData(Scheduler* scheduler) {
  SetScheduler(scheduler);
}

PolicyData::~PolicyData() {
}

void PolicyData::GetDevices(Task* task, Device** devs, int* ndevs) {
  size_t total_size[BRISBANE_MAX_NDEVS];
  for (int i = 0; i < ndevs_; i++) total_size[i] = 0UL;
  for (int i = 0; i < task->ncmds(); i++) {
    Command* cmd = task->cmd(i);
    if (cmd->type() == BRISBANE_CMD_KERNEL) {
      KernelArg* args = cmd->kernel_args();
      for (int i = 0; i < cmd->kernel_nargs(); i++) {
        Mem* mem = (args + i)->mem;
        if (!mem || !mem->Owner()) continue;
        total_size[mem->Owner()->devno()] += mem->size();
      }
    } else if (cmd->type() == BRISBANE_CMD_H2D || cmd->type() == BRISBANE_CMD_D2H) {
      Mem* mem = cmd->mem();
      if (!mem || !mem->Owner()) continue;
      total_size[mem->Owner()->devno()] += mem->size();
    }
  }
  int target_dev = 0;
  size_t max_size = 0UL;
  for (int i = 0; i < ndevs_; i++) {
    if (total_size[i] > max_size) {
      max_size = total_size[i];
      target_dev = i;
    }
  }
  devs[0] = devs_[target_dev];
  *ndevs = 1;
}

} /* namespace rt */
} /* namespace brisbane */
