/* #define DEBUG */
#include "common.h"
#include <coroutine>
#include <cstdio>
#include <iostream>
#include <vector>
using namespace std;
struct Generator {
  // promise_type is a struct that contains the coroutine state
  // and the return value
  struct promise_type {
    int _val;
    void unhandled_exception() noexcept {}
    std::suspend_always initial_suspend() noexcept {
      DEBUG_PRINTF("init suspend\n");
      return {};
    }
    std::suspend_always final_suspend() noexcept {
      DEBUG_PRINTF("final_suspend\n");
      return {};
    }
    suspend_always yield_value(int val) noexcept {
      DEBUG_PRINTF("yield_value\n");
      _val = val;
      return {};
    }
    void return_value(int val) noexcept {
      DEBUG_PRINTF("return_value\n");
      _val = val;
      /* value_out = std::move(s); */
    }
    Generator get_return_object() noexcept {
      DEBUG_PRINTF("get_return_object\n");
      return Generator(this);
    }
  };
  using Handle = coroutine_handle<promise_type>;
  Handle mCtrl;
  explicit Generator(promise_type *p) { mCtrl = Handle::from_promise(*p); }
  Generator(Generator &&rhs) : mCtrl{rhs.mCtrl} { rhs.mCtrl = {}; }
  ~Generator() {
    if (mCtrl)
      mCtrl.destroy();
  }
  int value() { return mCtrl.promise()._val; }
  int operator()() {
    DEBUG_PRINTF("operator()\n");
    mCtrl.resume();
    return mCtrl.promise()._val;
  }
  bool finished() { return mCtrl.done(); }
};

Generator interleave(vector<int> &a, vector<int> &b) {
  auto lamb = [](std::vector<int> &v) -> Generator {
    for (const auto &e : v)
      co_yield e;
  };
  auto g1 = lamb(a);
  auto g2 = lamb(b);
  while (not g1.finished() and not g2.finished()) {
    if (not g1.finished()) {
      g1.mCtrl.resume();
      if (not g1.finished())
        co_yield g1.value();
    }
    if (not g2.finished()) {
      g2.mCtrl.resume();
      if (not g2.finished())
        co_yield g2.value();
    }
  }
}

int main(int argc, char *argv[]) {
  vector<int> a{1, 2, 3};
  vector<int> b{4, 5, 6};
  auto g = interleave(a, b);
  while (not g.finished()) {
    g.mCtrl.resume();
    if (not g.finished())
      cout << g.value() << endl;
  }

  return 0;
}
