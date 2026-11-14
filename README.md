# c_simulating_cpp

用 C 语言模拟 Python 对象模型的迷你库。

## 功能

- 运行时创建类型、挂方法
- 对象属性动态增删改
- 按名字调用方法
- 统一的值类型（数字 / 字符串 / 对象 / 空）

## 文件

| 文件 | 说明 |
|---|---|
| `pylite.h` / `pylite.c` | 迷你运行时库 |
| `main.c` | 使用示例 |
| `shape_demo.c` | 结构体 + 函数指针版本，用于对照 |

## 编译运行

```sh
clang -std=c17 -Wall -Wextra pylite.c main.c -o build/main
./build/main
```

VS Code 里按 `Cmd+Shift+B` 编译，`Tasks: Run Task` → `run` 运行。

## 用法

```c
PyType *Circle = py_type("Circle");
py_method(Circle, "area", circle_area);

PyObject *c = py_new(Circle);
py_set(c, "r", py_num(2.0));

py_print(c);                  /* Circle(r=2) */
py_call(c, "area", PY_END);   /* 相当于 Python 的 c.area() */
```
