<h2 align="center"> Yet-Another-Cliche-Handy-Thread 线程util库 </h2>

### 项目简介
使用C++14标准开发的轻量级高性能线程util库，提供了常用的异步线程延迟执行任务，定时轮询任务及执行后的回调等util功能

---

### 功能
- 提供了丰富的异步定时器任务功能
- 提供了延时异步任务调用功能
- 提供了丰富的任务回调功能
- 使用原生接口，保证了跨平台支持
- 线程安全的接口保证

---

### Examples

#### 示例1
1. 创建一个HandyThread对象
```cpp
    auto yacht = std::unique_ptr<HandyThread>;
```
2. 创建一个dummy对象用于在yacht中执行任务
```cpp
    auto dummy = std::make_shared<Dummy>(1, 2);
```
3. 使用Run_Once_Task标志创建一个单次任务
```cpp
    yacht->Run(Run_Once_Task, [dummy] { dummy->speak(); });
```

#### 示例2
1. 创建一个HandyThread对象
```cpp
    auto yacht   = std::unique_ptr<HandyThread>;
```
2. 创建一个dummy对象用于在yacht中执行任务，再创建一个handler对象用于执行回调
```cpp
    auto dummy   = std::make_shared<Dummy>(1, 2);
    auto handler = std::make_shared<Handler>();
```
3. 使用Run_Once_Task标志创建一个单次任务，并设置一个回调
```cpp
        yacht
        ->SetCallbackCtx([dummy, handler] {
            handler->Handle(dummy->j);
            return 1;
        })
        ->Run(Run_Once_Task, [dummy] {
            dummy->speak();
            return 1;
        });
```

#### 示例3
创建一个定时器任务
```cpp
    auto yacht   = std::unique_ptr<HandyThread>;
    auto dummy   = std::make_shared<Dummy>(1, 2);
    auto handler = std::make_shared<Handler>();
    yacht
        ->SetCallbackCtx([dummy, handler] {
            handler->Handle(dummy->j);
            return 1;
        })
        ->Run(Timer_Task, [dummy] {
            dummy->speak();
            return 1;
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
```

#### 示例4
创建一个定时器任务，不使用lambda而是直接调用dummy的成员函数
```cpp
    auto yacht   = MakeHandyThreadPtr();
    auto dummy   = std::make_shared<Dummy>(1, 2);
    auto handler = std::make_shared<Handler>();

    int x = 10, y = 20;
    yacht
        ->SetCallbackCtx([dummy, handler] {
            handler->Handle(dummy->j);
            return 1;
        })
        ->Run(Timer_Task, &Dummy::speak3, &(*dummy), std::ref(x), std::ref(y));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
```

#### 示例5
1. 创建一个HandyThread对象
```cpp
    auto yacht   = std::unique_ptr<HandyThread>;
```

2. 创建一个dummy对象用于在t中执行任务，再创建一个handler对象用于执行回调
```cpp
    auto dummy   = std::make_shared<Dummy>(1, 2);
    auto handler = std::make_shared<Handler>();
```

3. 给创建的yacht对象设置Deferred_Config延时启动，并使用Delay_Config设置延时1000ms，设置了延时启动后调用Run不会直接运行任务，而是将任务保存起来
```cpp
    yacht->SetConfig(Deferred_Config, "1")
        ->SetConfig(Delay_Config, "1000")
        ->SetCallbackCtx([dummy, handler] {
            handler->Handle(dummy->j);
            return 1;
        })
        ->Run(Timer_Task, [dummy] {
            dummy->speak();
            return 1;
        });
```

4. 此时运行Launch才会启动任务 
```cpp
    yacht->Launch();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
```
