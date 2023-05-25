#define DEBUG
#include "common.h"
#include <coroutine>
#include <iostream>
#include <list>
#include <vector>
using namespace std;

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
};

static Scheduler gsched;

struct suspend {
  auto operator co_await() {
    struct awaiter : std::suspend_always {
      void await_suspend(coroutine_handle<> coro) const noexcept {
        DEBUG_PRINTF("\n");
        gsched.coroutines.push_back(coro);
      }
    };
    DEBUG_PRINTF("\n");
    return awaiter{};
  };
};

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

Task taskA() {
  std::cout << "Hello, from task A\n";
  co_await suspend();
  std::cout << "A is back doing work\n";
  co_await suspend();
  std::cout << "A is back doing more work\n";
}

Task taskB() {
  std::cout << "Hello, from task B\n";
  co_await suspend();
  std::cout << "B is back doing work\n";
  co_await suspend();
  std::cout << "B is back doing more work\n";
}

void Use() {
  auto ta = taskA();
  auto tb = taskB();
  ta.mCtrl.resume();
  tb.mCtrl.resume();
  while (gsched.schedule())
    ;
}

int main(int argc, char *argv[]) {
  Use();
  return 0;
}
