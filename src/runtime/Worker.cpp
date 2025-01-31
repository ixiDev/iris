#include "Worker.h"
#include "Debug.h"
#include "Consistency.h"
#include "Device.h"
#include "Platform.h"
#include "QueueReady.h"
#include "Scheduler.h"
#include "Task.h"
#include "Timer.h"

namespace brisbane {
namespace rt {

Worker::Worker(Device* dev, Platform* platform, bool single) {
  dev_ = dev;
  platform_ = platform;
  dev_->set_worker(this);
  scheduler_ = platform_->scheduler();
  if (scheduler_) consistency_ = scheduler_->consistency();
  else consistency_ = NULL;
  single_ = single;
  if (single) queue_ = platform->queue();
  else queue_ = new QueueReady(128);
  busy_ = false;
}

Worker::~Worker() {
  if (single_) delete queue_;
}

void Worker::TaskComplete(Task* task) {
  if (scheduler_) scheduler_->Invoke();
  Invoke();
}

void Worker::Enqueue(Task* task) {
  while (!queue_->Enqueue(task)) { }
  Invoke();
}

void Worker::Execute(Task* task) {
  if (!task->Executable()) return;
  task->set_dev(dev_);
  if (task->marker()) {
    dev_->Synchronize();
    task->Complete();
    return;
  }
  busy_ = true;
  if (scheduler_) scheduler_->StartTask(task, this);
  if (consistency_) consistency_->Resolve(task);
  dev_->Execute(task);
  if (!task->cmd_last()) {
    if (scheduler_) scheduler_->CompleteTask(task, this);
    //task->Complete();
  }
  busy_ = false;
}

void Worker::Run() {
  while (true) {
    Sleep();
    if (!running_) break;
    Task* task = NULL;
    while (queue_->Dequeue(&task)) Execute(task);
  }
}

unsigned long Worker::ntasks() {
  return queue_->Size() + (busy_ ? 1 : 0);
}

} /* namespace rt */
} /* namespace brisbane */
