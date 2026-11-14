#include <stdio.h>      /* printf */
#include "pylite.h"     /* 自己实现的模拟类型库 */

/* ============================================================
 * 用 pylite 写“像 Python 一样”的代码
 *
 * 对比一下：
 *
 *   Python                        pylite
 *   ------                        ------
 *   Circle = type("Circle", ())   PyType *Circle = py_type("Circle");
 *   c = Circle()                  PyObject *c = py_new(Circle);
 *   c.r = 2.0                     py_set(c, "r", py_num(2.0));
 *   c.area()                      py_call(c, "area", PY_END);
 *   print(c)                      py_print(c);
 *
 * 区别只有一个：方法名、属性名必须写成字符串。
 * 因为 C 的语法不允许运行时按名字访问成员。
 * ============================================================ */


/* ---------- 圆的方法 ---------- */

/* 每个方法都是同一个签名：
 *   self  —— 对象自己，相当于 Python 里的 self
 *   argc  —— 参数个数
 *   argv  —— 参数数组
 * 返回值统一是 Value。
 *
 * 这个函数算面积，不需要参数，所以用 (void) 显式忽略它们，
 * 否则编译器会警告“参数没用到”。 */
static Value circle_area(PyObject *self, int argc, Value *argv)
{
    (void)argc;
    (void)argv;

    double r = py_get(self, "r").num;   /* 相当于 self.r */
    return py_num(3.1415926 * r * r);
}

/* grow(n)：把半径增加 n。没有返回值，所以返回空值。 */
static Value circle_grow(PyObject *self, int argc, Value *argv)
{
    double r = py_get(self, "r").num;

    if (argc >= 1) {
        r += argv[0].num;               /* argv[0] 就是 Python 里的第一个参数 */
    }

    py_set(self, "r", py_num(r));       /* 相当于 self.r = r */
    return py_nil();
}

/* describe()：打印一句话，演示读字符串属性。 */
static Value circle_describe(PyObject *self, int argc, Value *argv)
{
    (void)argc;
    (void)argv;

    printf("我是 %s，半径 %g\n",
           py_get(self, "name").str,
           py_get(self, "r").num);
    return py_nil();
}


/* ---------- 矩形的方法 ---------- */

static Value rect_area(PyObject *self, int argc, Value *argv)
{
    (void)argc;
    (void)argv;

    double w = py_get(self, "w").num;
    double h = py_get(self, "h").num;
    return py_num(w * h);
}


/* ---------- 通用代码：只认名字，不认类型 ---------- */

/* 这个函数完全不知道传进来的是圆还是矩形。
 * 它只知道：调用名叫 "area" 的方法，就能拿到面积。
 * 这就是 Python 那种“鸭子类型”的感觉。 */
static void show_area(PyObject *o)
{
    Value v = py_call(o, "area", PY_END);
    printf("%-8s area = %g\n", py_type_name(o), v.num);
}


int main(void)
{
    /* ---------- 1. 建类型，挂方法 ---------- */

    PyType *Circle = py_type("Circle");
    py_method(Circle, "area",     circle_area);
    py_method(Circle, "grow",     circle_grow);
    py_method(Circle, "describe", circle_describe);

    PyType *Rect = py_type("Rect");
    py_method(Rect, "area", rect_area);

    /* ---------- 2. 造对象，设属性 ---------- */

    PyObject *c = py_new(Circle);
    py_set(c, "name", py_str("circle"));
    py_set(c, "r",    py_num(2.0));

    PyObject *r = py_new(Rect);
    py_set(r, "name", py_str("rect"));
    py_set(r, "w",    py_num(3.0));
    py_set(r, "h",    py_num(4.0));

    /* ---------- 3. 打印 ---------- */

    py_print(c);
    printf("\n");
    py_print(r);
    printf("\n");

    /* ---------- 4. 同一段代码处理不同对象 ---------- */

    show_area(c);
    show_area(r);

    /* ---------- 5. 改属性、调方法 ---------- */

    py_call(c, "grow", py_num(1.0), PY_END);   /* 相当于 c.grow(1.0) */
    printf("grow 之后 r = %g\n", py_get(c, "r").num);
    show_area(c);

    py_call(c, "describe", PY_END);            /* 相当于 c.describe() */

    return 0;
}
