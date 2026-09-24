# 客户端示例（Client）

每个模式对应一个可执行程序，放在 `examples/<category>/<pattern>/main.cpp`。

- **已实现**：`singleton`、`factory_method`、`abstract_factory`、`builder`、`prototype`、`adapter`、`decorator`、`proxy`、`facade`、`composite`、`bridge`、`flyweight`、`strategy`、`observer`、`template_method` 是贴近真实场景的客户端。
- **未实现**：其余模式只提供程序骨架（可编译、打印提示、注释里写典型用法）。实现对应头文件后，把骨架里的 `TODO` 换成真实调用即可。

## 构建与运行

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target examples
./build/examples/example_singleton
./build/examples/example_factory_method
./build/examples/example_abstract_factory
./build/examples/example_builder
./build/examples/example_prototype
./build/examples/example_adapter
./build/examples/example_decorator
./build/examples/example_proxy
./build/examples/example_facade
./build/examples/example_composite
./build/examples/example_bridge
./build/examples/example_flyweight
./build/examples/example_strategy
./build/examples/example_observer
./build/examples/example_template_method
```

关闭示例：

```bash
cmake -S . -B build -DDESIGN_PATTERN_BUILD_EXAMPLES=OFF
```

单独编译某一个：

```bash
cmake --build build --target example_singleton
```

## 新增模式客户端

1. 新建 `examples/<category>/<pattern>/main.cpp`
2. `#include "<category>/<pattern>/<pattern>.h"`
3. 重新配置/编译后会出现 `example_<pattern>`（CMake 按目录名 glob）
