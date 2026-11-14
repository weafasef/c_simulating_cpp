#include "pylite.h"

#include <stdio.h>
#include <string.h>

/* ============================================================
 * pylite 的实现
 *
 * 核心就两张表：
 *   类型表：每个类型一张“方法表”，按名字查找函数
 *   对象表：每个对象一张“属性表”，按名字查找值
 * 所谓“按名字”，就是 strcmp 一个一个人的比。
 * ============================================================ */

/* 所有类型和对象都放在静态数组里，不用 malloc，也不用 free。
 * 好处：不会内存泄漏；坏处：数量有上限（见 pylite.h）。 */
static PyType   g_types[PY_MAX_TYPES];
static int      g_type_count = 0;

static PyObject g_objects[PY_MAX_OBJECTS];
static int      g_object_count = 0;

/* 把名字安全地拷进固定长度的字符数组。
 * 太长就截断，保证最后一定有 '\0'。 */
static void copy_name(char *dst, const char *src)
{
    snprintf(dst, PY_NAME_MAX, "%s", src);
}

/* ---------- 造值 ---------- */

Value py_num(double n)
{
    Value v = {0};      /* 先清零，避免 union 里有垃圾数据 */
    v.kind = VAL_NUM;
    v.num = n;
    return v;
}

Value py_str(const char *s)
{
    Value v = {0};
    v.kind = VAL_STR;
    v.str = s;
    return v;
}

Value py_nil(void)
{
    Value v = {0};
    v.kind = VAL_NIL;
    return v;
}

/* ---------- 类型 ---------- */

PyType *py_type(const char *name)
{
    if (g_type_count >= PY_MAX_TYPES) {
        return NULL;    /* 满了 */
    }
    PyType *t = &g_types[g_type_count++];
    memset(t, 0, sizeof *t);
    copy_name(t->name, name);
    return t;
}

void py_method(PyType *t, const char *name, PyFunc fn)
{
    if (t == NULL || t->method_count >= PY_MAX_METHODS) {
        return;
    }
    int i = t->method_count++;
    copy_name(t->methods[i].name, name);
    t->methods[i].fn = fn;
}

/* ---------- 对象 ---------- */

PyObject *py_new(PyType *t)
{
    if (t == NULL || g_object_count >= PY_MAX_OBJECTS) {
        return NULL;
    }
    PyObject *o = &g_objects[g_object_count++];
    memset(o, 0, sizeof *o);
    o->type = t;
    return o;
}

/* ---------- 属性 ---------- */

void py_set(PyObject *o, const char *name, Value v)
{
    if (o == NULL) {
        return;
    }

    /* 已经有同名属性就覆盖 */
    for (int i = 0; i < o->attr_count; i++) {
        if (strcmp(o->attrs[i].name, name) == 0) {
            o->attrs[i].value = v;
            return;
        }
    }

    /* 没有就新增一个 */
    if (o->attr_count >= PY_MAX_ATTRS) {
        return;     /* 属性表满了 */
    }
    int i = o->attr_count++;
    copy_name(o->attrs[i].name, name);
    o->attrs[i].value = v;
}

Value py_get(PyObject *o, const char *name)
{
    if (o != NULL) {
        for (int i = 0; i < o->attr_count; i++) {
            if (strcmp(o->attrs[i].name, name) == 0) {
                return o->attrs[i].value;
            }
        }
    }
    return py_nil();    /* 找不到就返回空，不报错 */
}

/* ---------- 方法调用 ---------- */

Value py_call(PyObject *o, const char *name, ...)
{
    /* 第一步：把可变参数收进数组，直到遇到 PY_END */
    Value args[PY_MAX_ARGS];
    int argc = 0;

    va_list ap;
    va_start(ap, name);
    while (argc < PY_MAX_ARGS) {
        Value v = va_arg(ap, Value);
        if (v.kind == VAL_END) {
            break;
        }
        args[argc++] = v;
    }
    va_end(ap);

    if (o == NULL || o->type == NULL) {
        return py_nil();
    }

    /* 第二步：在对象所属类型的方法表里，按名字找函数 */
    for (int i = 0; i < o->type->method_count; i++) {
        if (strcmp(o->type->methods[i].name, name) == 0) {
            /* 第三步：找到就调用，把对象自己当第一个参数 */
            return o->type->methods[i].fn(o, argc, args);
        }
    }

    return py_nil();    /* 没找到这个方法 */
}

/* ---------- 打印 ---------- */

const char *py_type_name(PyObject *o)
{
    if (o == NULL || o->type == NULL) {
        return "None";
    }
    return o->type->name;
}

/* 打印一个 Value；遇到对象就递归调 py_print。
 * 注意：如果对象互相引用，这里会无限递归，第一版先不管。 */
static void print_value(Value v)
{
    switch (v.kind) {
    case VAL_NUM:
        printf("%g", v.num);
        break;
    case VAL_STR:
        printf("\"%s\"", v.str);
        break;
    case VAL_OBJ:
        py_print(v.obj);
        break;
    default:
        printf("None");
        break;
    }
}

void py_print(PyObject *o)
{
    if (o == NULL || o->type == NULL) {
        printf("None");
        return;
    }

    /* 格式：Circle(name="circle", r=2) */
    printf("%s(", o->type->name);
    for (int i = 0; i < o->attr_count; i++) {
        if (i > 0) {
            printf(", ");
        }
        printf("%s=", o->attrs[i].name);
        print_value(o->attrs[i].value);
    }
    printf(")");
}
