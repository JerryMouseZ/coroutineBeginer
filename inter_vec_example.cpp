#include "common.h"
#include <bits/iterator_concepts.h>
#include <coroutine>
#include <cstdio>
#include <iterator>
#include <utility>
#include <vector>
using namespace std;
struct Generator {
  struct promise_type {
    int _val{};
    Generator get_return_object() { return Generator{this}; }
    suspend_always initial_suspend() noexcept { return {}; }
    suspend_always final_suspend() noexcept { return {}; }
    suspend_always yield_value(int val) noexcept {
      _val = val;
      return {};
    }
    void unhandled_exception() {}
    /* void return_void() {} */
  };

  using Handle = coroutine_handle<promise_type>;
  Handle mCtrl;

  explicit Generator(promise_type *p) : mCtrl(Handle::from_promise(*p)) {}
  Generator(Generator &&rhs) : mCtrl(rhs.mCtrl) { rhs.mCtrl = {}; }
  ~Generator() {
    if (mCtrl) {
      mCtrl.destroy();
    }
  }

  int value() const { return mCtrl.promise()._val; }
  bool finished() const { return mCtrl.done(); }
  void resume() { mCtrl.resume(); }
  /* int operator()() { */
  /*   mCtrl.resume(); */
  /*   return mCtrl.promise()._val; */
  /* } */
  /* operator bool() { */
  /*   if (not finished()) */
  /*     resume(); */
  /*   return not finished(); */
  /* } */
  struct sentinel_t {};
  struct iterator {
    Handle mCtrl;
    bool operator==(sentinel_t) const { return mCtrl.done(); }
    iterator &operator++() {
      mCtrl.resume();
      return *this;
    }
    int operator*() const { return mCtrl.promise()._val; }
  };
  iterator begin() {
    // 开始运行，拿到第一个值
    mCtrl.resume();
    return {mCtrl};
  }
  sentinel_t end() { return {}; }
};

Generator interleave(vector<int> &a, vector<int> &b) {
  auto lamb = [](const vector<int> &v) -> Generator {
    for (const auto &e : v) {
      co_yield e;
    }
  };
  Generator g1 = lamb(a);
  Generator g2 = lamb(b);
  while (not g1.finished() && not g2.finished()) {
    g1.resume();
    if (not g1.finished()) {
      co_yield g1.value();
    }
    g2.resume();
    if (not g2.finished()) {
      co_yield g2.value();
    }
  }
}

int main(int argc, char *argv[]) {
  vector<int> a{2, 4, 6, 8};
  vector<int> b{3, 5, 7, 9};
  Generator g = interleave(a, b);
  for (const auto &e : g) {
    printf("%d\n", e);
  }
  return 0;
}
