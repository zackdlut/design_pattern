# C++ 设计模式学习项目

这是一个只包含骨架的 C++ 工程，用来自己动手实现 GoF 23 种设计模式。每个模式在 `include/`、`src/`、`tests/`、`docs/` 下都有独立目录或文件，实现全部留空。

## 目录结构

```text
include/<category>/<pattern>/<pattern>.h   头文件
src/<category>/<pattern>/<pattern>.cpp     源文件
tests/<category>/<pattern>_test.cpp        Google Test 用例
docs/<category>/<pattern>.md               学习笔记
examples/<category>/<pattern>/main.cpp     客户端示例
```

`<category>` 为 `creational`（创建型）、`structural`（结构型）、`behavioral`（行为型）。

## 构建

需要 CMake 3.20+、C++20 编译器。首次配置会通过 FetchContent 下载 [GoogleTest](https://github.com/google/googletest)。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

未实现的测试使用 `GTEST_SKIP()`，因此空骨架下 `ctest` 会全部跳过而不是失败。实现某个模式后，把对应测试里的占位 `TEST` 换成真正的断言即可。

如果不需要测试：

```bash
cmake -S . -B build -DDESIGN_PATTERN_BUILD_TESTS=OFF
```

客户端示例默认开启，产物在 `build/examples/example_<pattern>`。只编示例：

```bash
cmake --build build --target examples
./build/examples/example_singleton
./build/examples/example_factory_method
```

关闭示例：

```bash
cmake -S . -B build -DDESIGN_PATTERN_BUILD_EXAMPLES=OFF
```

未实现的模式在 `examples/` 下只有可编译骨架；`singleton`、`factory_method`、`abstract_factory`、`builder`、`prototype`、`adapter`、`decorator`、`proxy`、`facade`、`composite` 和 `bridge` 是真实使用场景。详见 [`examples/README.md`](examples/README.md)。

## 建议学习顺序

1. 创建型：`singleton` → `factory_method` → `abstract_factory` → `builder` → `prototype`
2. 结构型：`adapter` → `decorator` → `proxy` → `facade` → `composite` → `bridge` → `flyweight`
3. 行为型：`strategy` → `observer` → `command` → `template_method` → `iterator` → `state` → 其余

每个模式的笔记模板在 `docs/` 下，可边实现边填写。
