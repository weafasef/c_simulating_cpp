#ifndef PYLITE_H
#define PYLITE_H

/* ============================================================
 * pylite —— 用 C 模拟 Python 的对象模型（第一版）
 *
 * 它能做什么：
 *   - 运行时创建类型       py_type("Circle")
 *   - 给类型挂方法         py_method(Circle, "area", circle_area)
 *   - 创建对象             py_new(Circle)
 *   - 动态增删改属性       py_set(obj, "r", py_num(2.0))
 *   - 按名字调方法         py_call(obj, "area", PY_END)
 *   - 打印对象             py_print(obj)
 *
 * 它不能做什么：
 *   - 不能写 obj.area()，只能用字符串名字
 *   - 没有继承
 *   - 没有错误提示，查不到就返回空值
 *
 * 所有东西（数字、字符串、对象、空）都装在 Value 里，
 * 这是 Python 对象模型的核心思路：统一的“值”类型。
 * ============================================================ */

#include <stdarg.h>

/* ---------- 容量上限（先写死，够用就行） ---------- */

#define PY_MAX_TYPES    16   /* 最多建多少个类型 */
#define PY_MAX_OBJECTS 128   /* 最多造多少个对象 */
#define PY_MAX_METHODS  16   /* 每个类型最多几个方法 */
#define PY_MAX_ATTRS    32   /* 每个对象最多几个属性 */
#define PY_NAME_MAX     32   /* 名字最长多少字节 */
#define PY_MAX_ARGS     16   /* 方法最多接几个参数 */

/* ---------- 值 ---------- */

/* 一个 Value 能装四种东西。
 * kind 记录“这次装的是哪种”，union 负责真正存数据。
 * 读的时候先看 kind，再取对应的成员，别取错。 */
typedef enum {
    VAL_NIL,    /* 空，相当于 Python 的 None */
    VAL_NUM,    /* 数字 */
    VAL_STR,    /* 字符串 */
    VAL_OBJ,    /* 对象 */
    VAL_END     /* 只用来标记参数结束，不会存进属性里 */
} ValKind;

typedef struct PyObject PyObject;
typedef struct PyType   PyType;

typedef struct {
    ValKind kind;
    union {
        double num;
        const char *str;
        PyObject *obj;
    };
} Value;

/* ---------- 类型与对象 ---------- */

/* 方法的统一签名：
 *   self  —— 对象自己，相当于 Python 方法里的 self
 *   argc  —— 传进来几个参数
 *   argv  —— 参数数组
 * 返回值也必须是 Value。 */
typedef Value (*PyFunc)(PyObject *self, int argc, Value *argv);

/* 类型：一个名字 + 一张方法表 */
struct PyType {
    char name[PY_NAME_MAX];
    int  method_count;
    struct {
        char name[PY_NAME_MAX];
        PyFunc fn;
    } methods[PY_MAX_METHODS];
};

/* 对象：指向自己的类型 + 一张属性表
 * 注意：属性是运行时存的，跟 C 结构体成员完全不是一回事。 */
struct PyObject {
    PyType *type;
    int  attr_count;
    struct {
        char name[PY_NAME_MAX];
        Value value;
    } attrs[PY_MAX_ATTRS];
};

/* ---------- 造值 ---------- */

Value py_num(double n);      /* 数字      → 相当于 Python 的 2.0 */
Value py_str(const char *s); /* 字符串    → 相当于 "hello" */
Value py_nil(void);          /* 空        → 相当于 None */

/* ---------- 类型与对象 ---------- */

PyType   *py_type(const char *name);          /* 建一个类型 */
void      py_method(PyType *t, const char *name, PyFunc fn); /* 给类型挂方法 */
PyObject *py_new(PyType *t);                  /* 造一个对象 */

/* ---------- 属性 ---------- */

void  py_set(PyObject *o, const char *name, Value v); /* 相当于 o.name = v */
Value py_get(PyObject *o, const char *name);          /* 相当于 o.name */

/* ---------- 方法调用 ---------- */

/* 用法：
 *   py_call(c, "area", PY_END);
 *   py_call(c, "grow", py_num(1.0), PY_END);
 * 最后一个参数必须是 PY_END，用来告诉函数“参数到此为止”。 */
Value py_call(PyObject *o, const char *name, ...);

/* 参数结束标记。写成宏是为了用起来像关键字。 */
#define PY_END ((Value){ .kind = VAL_END })

/* ---------- 打印 ---------- */

const char *py_type_name(PyObject *o); /* 取对象类型名，比如 "Circle" */
void        py_print(PyObject *o);     /* 打印成 Circle(r=2) 这种样子 */

#endif
