# Cpp Coroutine beginer from [CppCon](https://www.youtube.com/watch?v=8sEe-4tig_A&t=2852s)

| keyword   | Action | State   |
| --------- | ------ | ------- |
| co_yield  | output | suspend |
| co_return | output | end     |
| co_await  | input  | suspend |

## Coroutine Chat
```C++
Chat Func()
{
    co_yield "Hello\n";
    std::cout << co_await std::string{};
    co_return "Here\n";
}

void Use()
{
    Chat chat = Func();
    std::cout << chat.listen();
    chat.answer("where are you?\n");
    std::cout << chat.listen();
}
```
这个例子中想要实现的功能是Func首先返回"Hello"以及控制权给调用者，然后等待用户的输入。
用户输入之后，等待，恢复协程的执行，并让协程打印输入的值，最后协程返回"Here"并结束协程的运行。
为了实现这个功能，首先需要一个协程的对象，也就是这个协程函数的返回值`Chat`。
要求这个对象中必须要有一个结构体promise_type，用来定义协程`suspend`以及`resume`时的行为。
如果需要用到co_yield方法，需要实现promise_type::yield_value()。
如果需要用到co_return，需要实现方法promise_type::return_value()。
如果需要用到co_await，需要实现方法awaiter promise_type::await_transform(T val)，并需要定义接口awaiter，awaiter中需要实现await_ready()，await_resume()，以及await_suspend()方法。
```C++
struct Chat{
  struct promise_type{
    string value_in, string value_out;
    std::suspend_always_initial_suspend() {return {};}
    std::suspend_always final_suspend() { return {};}
    std::suspend_always yield_value(std::string s) {value_out = std::move(s); return {};}
    void return_value(std::string s) {value_out = std::move(s);}
    auto await_transform(std::string s) {
      struct awaiter {
        promise_type &pt;
        bool await_ready() {return true;}
        std::string await_resume() {return pt.value_in; }
        void await_suspend(std::coroutine_handle<promise_type> h) {}
      }
      return awaiter(*this);
    }
  };
};
```
此外，协程对象还需要一个coroutine_handle<promise_type>的对象，再实现相应的构造和析构方法，以及用到的listen以及answer函数，就能实现功能了。
```C++
// struct Chat {
using Handle = std::coroutine_handle<promise_type>
Handle mCtrl;
explicit Chat(promise_type *p) : mCtrl(Handle::from_promise(*p)) {}
Chat(Chat &&c) : mCtrl(c.mCtrl) {c.mCtrl = {};}
~Chat() {
  if (mCtrl) {
    mCtrl.destroy();
  }
}
string listen() {
  if (not mCtrl.done()) {
    mCtrl.resume();
  }
  DEBUG_PRINTF("listen\n");
  return std::move(mCtrl.promise().value_out);
}

void answer(string msg) {
  mCtrl.promise().value_in = msg;
  if (not mCtrl.done()) {
    mCtrl.resume();
  }
}
// }
```
再写上main函数，大工告成。
```
int main(int argc, char **argv) {
  Use();
  return 0;
}
```
运行结果为
```Bash
Hello
Where are you?
Here
```
如果我们想知道协程是如何调度和运行的呢，我们可以在每个函数的入口打印一下当前的函数.
```Bash
/home/jz/coroutine_beginer/coroutine_chat.cpp:23 at get_return_object: get_return_object
/home/jz/coroutine_beginer/coroutine_chat.cpp:15 at initial_suspend: init suspend
/home/jz/coroutine_beginer/coroutine_chat.cpp:62 at listen: listen
/home/jz/coroutine_beginer/coroutine_chat.cpp:27 at yield_value: yield_value
Hello
/home/jz/coroutine_beginer/coroutine_chat.cpp:70 at answer: answer
/home/jz/coroutine_beginer/coroutine_chat.cpp:43 at await_transform: await_transform
/home/jz/coroutine_beginer/coroutine_chat.cpp:36 at await_resume: await_resume
Where are you?
/home/jz/coroutine_beginer/coroutine_chat.cpp:47 at return_value: return_value
/home/jz/coroutine_beginer/coroutine_chat.cpp:19 at final_suspend: final_suspend
/home/jz/coroutine_beginer/coroutine_chat.cpp:62 at listen: listen
Here
```
从运行结果可以看出，首先会调用`get_return_object`创建一个协程对象，然后调用`initial_suspend`，由于返回值是`suspend_always`，协程挂起。接着我们在`listen`函数中调用`resume`唤醒协程，在`co_yield`时协程挂起，listen将`yield_value`保存的value_out返回，并让协程的调用者打印。

在`answer`中我们给`value_in`赋值，并恢复协程的运行，在co_await处await_transform返回了一个awaiter对象，然后因为await_ready一直是true，所以协程没有挂起，继续执行。
直到co_return处，调用了`return_value`传递返回值，并且调用final_suspend结束协程。最后的listen处因为协程已经结束，就拿出promise_type中的val返回
