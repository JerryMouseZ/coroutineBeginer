#define DEBUG
#include "common.h"
#include <coroutine>
#include <cstdio>
#include <future>
#include <iostream>
#include <string>
using namespace std;

struct Chat {
  struct promise_type {
    string value_in, value_out;
    void unhandled_exception() noexcept {}
    std::suspend_always initial_suspend() noexcept {
      DEBUG_PRINTF("init suspend\n");
      return {};
    }
    std::suspend_always final_suspend() noexcept {
      DEBUG_PRINTF("final_suspend\n");
      return {};
    }
    Chat get_return_object() noexcept {
      DEBUG_PRINTF("get_return_object\n");
      return Chat(this);
    };
    suspend_always yield_value(std::string s) noexcept {
      DEBUG_PRINTF("yield_value\n");
      value_out = std::move(s);
      return {};
    }
    auto await_transform(std::string s) noexcept {
      struct awaiter {
        promise_type &pt;
        constexpr bool await_ready() const noexcept { return true; }
        std::string await_resume() noexcept {
          DEBUG_PRINTF("await_resume\n");
          return pt.value_in;
        }
        void await_suspend(std::coroutine_handle<promise_type> h) noexcept {
          DEBUG_PRINTF("await_suspend\n");
        }
      };
      DEBUG_PRINTF("await_transform\n");
      return awaiter(*this);
    }
    void return_value(std::string s) noexcept {
      DEBUG_PRINTF("return_value\n");
      value_out = std::move(s);
    }
  };

  using Handle = std::coroutine_handle<promise_type>;
  Handle mCtrl;
  explicit Chat(promise_type *p) : mCtrl{Handle::from_promise(*p)} {}
  Chat(Chat &&c) noexcept : mCtrl{c.mCtrl} { c.mCtrl = {}; }
  ~Chat() {
    if (mCtrl) {
      mCtrl.destroy();
    }
  }
  string listen() {
    DEBUG_PRINTF("listen\n");
    if (not mCtrl.done()) {
      mCtrl.resume();
    }
    return std::move(mCtrl.promise().value_out);
  }

  void answer(string msg) {
    DEBUG_PRINTF("answer\n");
    mCtrl.promise().value_in = msg;
    if (not mCtrl.done()) {
      mCtrl.resume();
    }
  }
};

Chat Fun() {
  co_yield "Hello\n";
  std::cout << co_await string{} << std::endl;
  co_return "Here\n";
}

void Use() {
  Chat chat = Fun();
  cout << chat.listen();
  chat.answer("Where are you?");
  cout << chat.listen();
}

int main(int argc, char *argv[]) {
  Use();
  return 0;
}
