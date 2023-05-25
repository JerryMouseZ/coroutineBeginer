/* #define DEBUG */
#include "common.h"
#include <coroutine>
#include <iostream>
#include <list>
#include <vector>
using namespace std;
struct Task {
  struct promise_type {
    suspend_always final_suspend() noexcept { return {}; }
    suspend_always initial_suspend() { return {}; }
    void unhandled_exception() {}
    Task get_return_object() { return Task(this); }
  };
  using Handle = coroutine_handle<promise_type>;
  Handle mCtrl;
  explicit Task(promise_type *p) : mCtrl(Handle::from_promise(*p)) {}
};

struct Scheduler {
  list<coroutine_handle<>> coroutines;
  bool schedule() {
    DEBUG_PRINTF("\n");
    auto task = coroutines.front();
    coroutines.pop_front();
    if (not task.done())
      task.resume();
    return not coroutines.empty();
  }
  auto suspend() {
    struct awaiter : suspend_always {
      Scheduler &sched;
      explicit awaiter(Scheduler &sched) : sched(sched) {}
      void await_suspend(coroutine_handle<> coro) {
        sched.coroutines.push_back(coro);
      }
    };
    DEBUG_PRINTF("\n");
    return awaiter(*this);
  }
};

Task taskA(Scheduler &sched) {
  std::cout << "Hello, from task A\n";
  co_await sched.suspend();
  std::cout << "A is back doing work\n";
  co_await sched.suspend();
  std::cout << "A is back doing more work\n";
}

Task taskB(Scheduler &sched) {
  std::cout << "Hello, from task B\n";
  co_await sched.suspend();
  std::cout << "B is back doing work\n";
  co_await sched.suspend();
  std::cout << "B is back doing more work\n";
}

void Use() {
  Scheduler scheduler{};
  auto ta = taskA(scheduler);
  auto tb = taskB(scheduler);
  ta.mCtrl.resume();
  tb.mCtrl.resume();
  while (scheduler.schedule())
    ;
}

int main(int argc, char *argv[]) {
  Use();
  return 0;
}
