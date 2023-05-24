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
这个例子中想要实现的功能是Func首先返回"Hello"以及控制权给调用者
