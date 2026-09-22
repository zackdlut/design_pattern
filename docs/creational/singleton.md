# Singleton（单例）

**类型**：Creational（创建型）

**代码位置**：

- 头文件：[`include/creational/singleton/singleton.h`](../../include/creational/singleton/singleton.h)
- 实现：[`src/creational/singleton/singleton.cpp`](../../src/creational/singleton/singleton.cpp)
- 测试：[`tests/creational/singleton_test.cpp`](../../tests/creational/singleton_test.cpp)
- 客户端：[`examples/creational/singleton/main.cpp`](../../examples/creational/singleton/main.cpp)

## 意图

保证一个类在进程里**只有一个实例**，并提供全局访问点（通常是 `getInstance()`）。

可以把单例想成大楼里的「总配电箱」：整栋楼只允许有一台，谁要用电都去同一个箱子，而不是每人再装一台。

```mermaid
flowchart LR
  A[线程 A] --> S[(唯一实例)]
  B[线程 B] --> S
  C[模块 C] --> S
```

## 适用场景

- 进程级共享资源：日志、配置、线程池、设备句柄
- 创建成本高，且全进程只需要一份
- **不适合**：只是图省事当全局变量用。单例会把依赖藏起来，单测替换困难

## 共同骨架

所有写法都靠这三件事保证「只有一个」：

```mermaid
classDiagram
  class Singleton {
    -Singleton()
    -~Singleton()
    +getInstance() Singleton&
    +Singleton(const Singleton&) deleted
    +operator=(const Singleton&) deleted
  }
```

| 构件                  | 作用                                            |
| --------------------- | ----------------------------------------------- |
| 私有 / 保护 / 删除构造与析构 | 见下一小节：三种权限管的不是同一件事 |
| 删除拷贝 / 移动       | 不能通过拷贝再变出第二个                        |
| 静态`getInstance()` | 唯一合法入口，返回同一份引用                    |

### 为什么有的 `private`，Holder 却是 `= delete`

差在一件事：**这个类本身是不是那唯一的对象。**

`Meyers` / `SingletonLasy` / `LazyLock` / `DoubleCheck` / `CallOnce` 的 `getInstance()` 都要在类内部真正造出一个**自己**：

```cpp
static SingletonMeyers instance_{};     // 要调 SingletonMeyers()
instance_ = new SingletonLazyLock();    // 要调 SingletonLazyLock()
```

构造函数必须 **存在**（`= default`），但不能给外面用（`private`）：

| | 外面 `Singleton x;` | 类里的 `getInstance()` |
|--|---------------------|-------------------------|
| `public` 构造 | 能，单例被绕开 | 能 |
| `private` + `= default` | 不能 | **能**（成员函数可以调私有构造） |
| `= delete` | 不能 | **也不能**，实例造不出来 |

析构同理：Meyers / 饿汉在程序退出时要析构那一个对象，析构函数必须存在，只是不让外面 `delete &getInstance()`，所以也是 `private`，不是 `delete`。

```mermaid
flowchart LR
  subgraph 其他单例["类 = 那个唯一实例"]
    G1[getInstance] -->|类内部调用| C1["private 构造 = default"]
    C1 --> I1[(一个 Singleton 对象)]
  end
```

`SingletonHolder` 相反：`getInstance()` 造的是 `T`，不是 Holder。Holder 自己永远不该有生命周期，构造/析构直接 **`= delete`**，谁都不能造（包括它自己的成员）。

```mermaid
flowchart LR
  subgraph Holder["类 ≠ 实例，只是入口"]
    G2[getInstance] --> I2[(一个 T)]
    G2 -.->|不要走这条路| H[Holder 对象]
  end
```

`SingletonTemplate` 还要让派生类 `Logger` 能初始化基类，所以是 **`protected` + `= default`**。

| | 其他 `Singleton*` | `SingletonHolder<T>` | `SingletonTemplate<T>` |
|--|-------------------|----------------------|------------------------|
| 这个类是什么 | 单例对象的类型 | 取 `T` 的工具 | `T` 的基类 |
| 要不要有「自己的实例」 | 要，恰好一个 | 一个都不要 | 不要基类实例，要派生类实例 |
| 构造怎么写 | `private` + `= default` | `= delete` | `protected` + `= default` |
| 原因 | 成员里能造，外面不能造 | 造出来就是误用 | 派生类还要能初始化基类 |

一句话：

- **private + default**：函数还在，只限「自己人」调用（造那一个单例）。
- **delete**：函数作废，谁都不能调用（Holder 不是实例）。
- **protected + default**：自己人 + 派生类（CRTP 的 `Logger` 还要调基类构造）。

差别还在三件事：**何时创建、如何保证线程安全、何时销毁**。

```mermaid
flowchart TD
  Q1{要不要懒加载?}
  Q1 -->|否，启动就造好| Eager[饿汉 SingletonLasy]
  Q1 -->|是| Q2{怎么保证只初始化一次?}
  Q2 -->|函数内 static| Meyers[Meyers / Template]
  Q2 -->|每次加锁| LazyLock[LazyLock]
  Q2 -->|先看再锁| DCLP[DoubleCheck + atomic]
  Q2 -->|标准库| CallOnce[call_once]
  Q3{单例性写在谁身上?}
  Q3 -->|T 自己就是单例| CRTP[SingletonTemplate]
  Q3 -->|不改 T，只共享一份| Holder[SingletonHolder]
```

---

## 1. Meyers：函数内 `static`

现代 C++ **首选**。第一次调用 `getInstance()` 时才构造；C++11 起，这个初始化由编译器保证线程安全（magic static）。

### 原理

```mermaid
sequenceDiagram
  participant T1 as 线程 1
  participant T2 as 线程 2
  participant S as 函数内 static 实例

  T1->>S: 第一次 getInstance()
  Note over S: 正在初始化，其他线程等待
  T2->>S: 同时 getInstance()
  S-->>T1: 返回同一个对象
  S-->>T2: 返回同一个对象
  T1->>S: 再次调用
  Note over S: 已构造完，直接返回，不再加锁
```

### 基本构成

- 没有类静态指针，也没有 mutex
- 局部 `static SingletonMeyers instance_{}` 就是那一份实例
- 程序退出时会析构（正常 RAII）

### 用法

```cpp
auto &s = SingletonMeyers::getInstance();
auto &t = SingletonMeyers::getInstance();
assert(&s == &t);
```

### 特点

- 代码最短，无手写内存序
- 析构顺序：若另一个静态对象析构时再调用 `getInstance()`，实例可能已经销毁（静态析构顺序问题）

---

## 2. 饿汉 `SingletonLasy`：启动即构造

类名叫 `Lasy`，语义其实是 **Eager（饿汉）**：进程启动、进入 `main` 之前就构造好，`getInstance()` 只是把引用递出去。

类内只能**声明**静态对象（类定义尚未结束，类型不完整），必须在 `.cpp` 里定义：

```cpp
// singleton.cpp
SingletonLasy SingletonLasy::instance_{};
```

### 原理

```mermaid
sequenceDiagram
  participant CRT as 启动 / 动态初始化
  participant I as instance_
  participant Main as main()
  participant User as 调用方

  CRT->>I: 构造唯一实例
  CRT->>Main: 进入 main
  User->>I: getInstance() 直接返回
```

### 基本构成

| 位置     | 内容                                               |
| -------- | -------------------------------------------------- |
| 头文件   | `static SingletonLasy instance_;` 声明           |
| `.cpp` | `SingletonLasy SingletonLasy::instance_{};` 定义 |

不能写成类内 `inline static SingletonLasy instance_{}`：此时类型还不完整。

### 用法

与 Meyers 相同：`SingletonLasy::getInstance()`。

测试里 `EagerReturnsSameInstance` 测的就是这个类。

### 特点

- `getInstance()` 无分支、无锁，可 `noexcept`
- 即使用不到也会构造
- 跨翻译单元时，若别的全局对象在动态初始化阶段就调用它，可能碰到**静态初始化顺序问题**

---

## 3. 懒汉加锁 `SingletonLazyLock`

第一次用才 `new`，每次 `getInstance()` 都加锁。正确、好懂，初始化之后仍付锁的成本。

### 原理

```mermaid
flowchart TD
  A[getInstance] --> B[加锁]
  B --> C{instance_ 为空?}
  C -->|是| D[new 并保存指针]
  C -->|否| E[直接用已有指针]
  D --> F[解锁并返回]
  E --> F
```

### 基本构成

- `inline static std::mutex mutex_`
- `inline static SingletonLazyLock *instance_ = nullptr`
- `new` 之后**故意不 delete**（进程结束由 OS 回收），避开静态析构顺序问题

### 用法

```cpp
auto &s = SingletonLazyLock::getInstance();
```

### 特点

- 一定线程安全
- 初始化完成后每次获取仍加锁，作为对照实现很合适，生产代码一般不会停在这一版

---

## 4. 双重检查锁 `SingletonDoubleCheck`

想法：大多数调用时实例已经存在，不必每次加锁。先无锁看一眼，只有「看起来还没有」才进锁，锁里再检查一次。

**必须用 `std::atomic`。** 普通指针做双重检查在 C++11 内存模型下是数据竞争，是未定义行为。

### 原理

```mermaid
flowchart TD
  A[getInstance] --> B["acquire 读 instance_"]
  B --> C{指针为空?}
  C -->|否，快路径| R[直接返回，不加锁]
  C -->|是| D[加锁]
  D --> E["relaxed 再读一次"]
  E --> F{仍然为空?}
  F -->|否，别人刚造好| U[解锁并返回]
  F -->|是| G[new]
  G --> H["release 写入 instance_"]
  H --> U
```

为什么要 `acquire` / `release`：

```mermaid
sequenceDiagram
  participant W as 线程 W 构造
  participant Mem as 内存
  participant R as 线程 R 读取

  W->>Mem: 1. 构造对象（写字段）
  W->>Mem: 2. release 发布指针
  Note over Mem: release 保证 1 在 2 之前对其他线程可见
  R->>Mem: acquire 读到非空指针
  R->>Mem: 此时对象字段一定已经写完
```

若不用 atomic，可能出现：别的线程看到「指针已经非空」，但对象还没构造完。

### 为什么锁里要用 `relaxed` 再读一次

第二次读是双重检查本身；用 `relaxed` 是因为这时已经在锁里，不需要再靠这次 load 做同步。

两个线程可能同时在锁外看到 `p == nullptr`，一起往锁上挤。先拿到锁的会 `new` 并 `store`。后拿到锁的如果**还用外面读到的那个空指针**，会再 `new` 一次，单例就破了。锁里必须再看一眼「现在指针到底是不是空」——这就是名字里的 Double-Check。

```mermaid
sequenceDiagram
  participant A as 线程 A
  participant B as 线程 B
  participant M as mutex
  participant I as instance_

  A->>I: acquire 读到 nullptr
  B->>I: acquire 读到 nullptr
  A->>M: 先拿到锁
  A->>I: new + release store
  A->>M: 解锁
  B->>M: 后拿到锁
  B->>I: 再读一次 → 已经非空
  Note over B: 不能再用外面那个 nullptr，否则会第二次 new
```

锁外那次必须是 **acquire**：没加锁，要靠它和构造线程的 **release store** 同步，才能保证「看到指针非空 ⇒ 对象已经构造完」。

锁里这次不一样。`std::mutex::lock()` 本身带 acquire，`unlock()` 带 release。先完成的线程是：

```text
加锁 → new 对象 → release store 指针 → 解锁
```

后进锁的线程 `lock()` 会与对方的 `unlock()` 建立 happens-before，因此对方写进对象里的字段、以及对 `instance_` 的 store，都已经可见。这时再 `load` 只是把指针值读出来，**不再承担同步职责**，`memory_order_relaxed` 就够了。写成 `acquire` 也正确，只是多余：锁已经做了同样的同步。

| 位置 | 操作 | 内存序 | 作用 |
|------|------|--------|------|
| 锁外 | 第一次 `load` | **acquire** | 快路径：看到非空时，对象内容必须可见 |
| 锁内 | 第二次 `load` | **relaxed** | 只确认有没有人已经造好，同步交给 mutex |
| 锁内 | `store` | **release** | 发布指针，给锁外那次 acquire 配对 |

一句话：再读是为了「不要造第二个」；`relaxed` 是因为「同步已经由锁做完了」。

### 基本构成

- `std::mutex`：保护真正的创建
- `std::atomic<SingletonDoubleCheck *>`：无锁快路径上的安全发布
- 同样有意泄漏

### 用法

```cpp
auto &s = SingletonDoubleCheck::getInstance();
```

多线程测试：`DoubleCheckIsThreadSafe`（8 个线程同时 `getInstance()`，地址必须相同）。

### 特点

- 初始化后走快路径，几乎无锁
- 手写内存序，教学价值高；生产代码更推荐 Meyers / `call_once`

---

## 5. `std::call_once`：`SingletonCallOnce`

把「只执行一次」交给标准库。`once_flag` 记住有没有做过；callable 若抛异常，下次还会再试。

### 原理

```mermaid
sequenceDiagram
  participant T1 as 线程 1
  participant T2 as 线程 2
  participant F as once_flag
  participant C as create()

  T1->>F: call_once
  T2->>F: call_once（等待）
  F->>C: 只让一个线程执行 create
  C->>C: new SingletonCallOnce
  F-->>T1: 返回
  F-->>T2: 返回（不再执行 create）
```

### 基本构成

- `inline static std::once_flag once_{}`
- `inline static SingletonCallOnce *instance_ = nullptr`
- 私有静态 `create()`：lambda 不是类的成员，放到静态成员函数里才能访问私有构造

### 用法

```cpp
auto &s = SingletonCallOnce::getInstance();
```

### 特点

- 语义比手写 DCLP 清楚
- 初始化逻辑很重、或不方便放进函数内 `static` 时很好用
- 日常情况 Meyers 更短

---

## 6. CRTP 模板 `SingletonTemplate`

前面几种每个类都要抄一遍样板。CRTP 把「单例性」抽到基类：`T` **自己就是**单例类型。

### 原理

```mermaid
classDiagram
  class SingletonTemplate~T~ {
    +getInstance() T&
    #SingletonTemplate()
    #~SingletonTemplate()
  }
  class TemplateDemo {
    -TemplateDemo()
    TemplateDemo ..> SingletonTemplate~TemplateDemo~ : friend
  }
  TemplateDemo --|> SingletonTemplate~TemplateDemo~
```

`getInstance()` 构造的是 `static T`（对 `Logger` 就是 `Logger{}`），所以：

1. 派生类必须 `friend class SingletonTemplate<T>`，否则基类访问不了私有构造
2. 派生类构造放 `private`，外面不能 `Logger log;`
3. **不能**给模板加 `final`，否则无法继承
4. 基类构造/析构必须是 `protected`，派生类才能初始化基类

### 为什么要 `friend` 和 `Logger() = default`

这两行是配对的：一行挡住外面随便造 `Logger`，另一行只把「造 `Logger`」的权限交给模板里的 `getInstance()`。

`getInstance()` 是 **`SingletonTemplate<Logger>` 的成员**，不是 `Logger` 的成员。能调用 `Logger` 私有构造的只有 `Logger` 自己的成员和它声明的 friend。没有 friend，`static T instance{}` 会报构造函数是 private。

```mermaid
flowchart LR
  G["SingletonTemplate&lt;Logger&gt;::getInstance()"]
  C["Logger() 私有构造"]
  G -->|"不是 Logger 的成员"| X[无权调用]
  G -->|"friend 之后"| C
```

如果不写任何构造函数，编译器会生成一个 **public** 默认构造，外面就能 `Logger log;`，单例被绕开。必须自己声明构造函数并放进 `private`。`= default` 表示实现仍用编译器生成的空构造，但**访问权限由你决定**。

```cpp
class Logger : public SingletonTemplate<Logger> {
  friend class SingletonTemplate<Logger>;  // 只允许模板内部构造
  Logger() = default;                      // 声明为私有，禁止 Logger log;
};
```

| 写法 | 外面 `Logger log;` | `getInstance()` 里 `Logger{}` |
|------|-------------------|--------------------------------|
| 不写构造函数 | 能（隐式 public） | 能 |
| `private: Logger() = default;` 且无 friend | 不能 | **不能** |
| 两行都写 | 不能 | 能 |

构造 `Logger` 时还会调基类构造。基类构造成 `protected`，派生类的 `Logger()` 才能初始化基类子对象；外面仍然不能直接造 `SingletonTemplate<Logger>`。若基类构造成 `private` 且不 friend 派生类，`Logger()` 自己也会编不过。

三条权限是一套的：

1. **`Logger() = default` 放 private**：禁止 `Logger log;`
2. **`friend SingletonTemplate<Logger>`**：允许 `getInstance()` 里那句 `static T instance{}`
3. **基类构造 `protected`**：允许派生类完成基类子对象的构造

### 用法

```cpp
class Logger : public SingletonTemplate<Logger> {
  friend class SingletonTemplate<Logger>;
  Logger() = default;
public:
  void info(const std::string &);
};

Logger::getInstance().info("boot");
// Logger log;  // 错误：构造是 private
```

测试里的 `TemplateDemo` 就是这种用法。

### 适合

日志、配置、全局服务——**按设计这个类型就只能有一个**。

---

## 7. 持有器 `SingletonHolder`

不改 `T`，只保证「通过 Holder 拿到的那份 `T` 是同一个」。`T` 仍然可以是普通类（第三方类型、单测里随便构造）。

### 原理

```mermaid
flowchart LR
  subgraph 不改 Database
    D1[Database 普通类]
  end
  H[SingletonHolder<Database>]
  I[(唯一的 Database 实例)]
  H -->|getInstance| I
  X[测试里 Database local] -.->|仍然合法| D1
```

Holder **不能当对象用**。正确用法只有静态函数：

```cpp
// 错误：造出来的是 Holder 自己，和那份 Database 无关
SingletonHolder<Database> x;

// 正确
auto &db = SingletonHolder<Database>::getInstance();
```

| 删掉谁 | 挡住什么 |
|--------|----------|
| `SingletonHolder() = delete` | `SingletonHolder<T> x;` 以及 `new SingletonHolder<T>` |
| `~SingletonHolder() = delete` | 就算用别的手段造出来，局部变量离开作用域时也不能析构 |

两个都删，意图是：**这个类型不允许有生命周期**。放在 `public` 里 `= delete`，误用时会报 “use of deleted function”，比写成 private 更清楚。拷贝/移动一并删除，因为 Holder 不能存在，自然也不能拷、不能移。

这和前面几种单例不同：那些类**就是**唯一实例，构造必须 `private` + `= default`，好让 `getInstance()` 在类内部造出那一个对象。对照见上文「共同骨架」里的权限表。

### 用法（对应测试 `SingletonHolderReturnsSameInstance`）

```cpp
class Database { /* 普通类，构造 public */ };

auto &a = SingletonHolder<Database>::getInstance();
auto &b = SingletonHolder<Database>::getInstance();
assert(&a == &b);
```

### 和 CRTP 的区别

```mermaid
flowchart TB
  subgraph CRTP["SingletonTemplate：类型即单例"]
    L[Logger 继承模板]
    L --> G1["Logger::getInstance()"]
    N1["Logger log; 编译失败"]
  end
  subgraph Holder["SingletonHolder：入口上的共享实例"]
    D[Database 不继承]
    H["SingletonHolder<Database>::getInstance()"]
    N2["Database local; 仍然合法"]
  end
```

|               | `SingletonTemplate<T>`     | `SingletonHolder<T>`                       |
| ------------- | ---------------------------- | -------------------------------------------- |
| 是什么        | `T` 的基类                 | 和`T` 无关的工具类                         |
| 要不要改`T` | 要：继承 + friend + 私有构造 | 不要                                         |
| 调用          | `Logger::getInstance()`    | `SingletonHolder<Database>::getInstance()` |
| `T t;`      | 编译失败                     | 构造 public 时合法                           |
| 适合          | 「这个类型就是单例」         | 「这里需要一份共享的`T`」                  |

---

## 总对照

| 写法                      | 创建时机            | 线程安全       | 销毁           | 推荐场景           |
| ------------------------- | ------------------- | -------------- | -------------- | ------------------ |
| Meyers                    | 首次`getInstance` | magic static   | 程序退出时析构 | **日常首选** |
| `SingletonLasy`（饿汉） | 启动 / 动态初始化   | 单 TU 内安全   | 程序退出时析构 | 确定启动就要有     |
| LazyLock                  | 首次使用            | 每次加锁       | 有意泄漏       | 教学 / 对照        |
| DoubleCheck               | 首次使用            | atomic + mutex | 有意泄漏       | 学内存模型         |
| CallOnce                  | 首次使用            | `once_flag`  | 有意泄漏       | 初始化逻辑较重     |
| Template                  | 同 Meyers           | 同 Meyers      | 同 Meyers      | 业务类本身是单例   |
| Holder                    | 同 Meyers           | 同 Meyers      | 同 Meyers      | 不侵入的共享实例   |

再记两点，和具体写法无关，但最容易混：

1. **初始化线程安全 ≠ 使用线程安全。** `getInstance()` 只保证「构造发生一次」。对象里的可变成员，并发读写仍要自己同步。
2. **有意泄漏**（LazyLock / DCLP / CallOnce）是为了避开「退出阶段用已销毁单例」。Meyers / 饿汉会在 `atexit` 时析构。

## 怎么选

```text
能改 T，且 T 按设计就只能有一个？
  └─ 是 → SingletonTemplate（内部用 Meyers）
T 改不了 / 只是某处要共享一份？
  └─ 是 → SingletonHolder
要手写具体类，而不是模板？
  ├─ 要懒加载 → Meyers（首选）或 call_once
  ├─ 启动就必须有 → 饿汉 SingletonLasy
  └─ 学习锁与内存序 → LazyLock，再看 DoubleCheck
```

## 参考

- 《设计模式：可复用面向对象软件的基础》（GoF）
- C++11 `[stmt.dcl]`：函数局部 `static` 的初始化线程安全
- `std::call_once` / `std::once_flag`（`<mutex>`）
- `std::atomic` 的 acquire / release（双重检查锁）
