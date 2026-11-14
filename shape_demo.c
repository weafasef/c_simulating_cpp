#include <stdio.h>  
/* ============================================================
 * 这个程序演示：C 语言怎么用「结构体 + 函数指针」模拟
 * C++ 里的「类」和「虚函数」。
 *
 * 记住三件事就够了：
 *   1. 结构体 = 数据
 *   2. 函数   = 操作
 *   3. 结构体里的函数指针 = 把操作挂到数据上
 * ============================================================ */


/* ---------- 1. 先定义一个“接口” ---------- */

/* 这一行的作用：
 *   有个结构体，真名叫 struct Shape；
 *   现在给它起个小名 Shape，后面就能直接写 Shape。
 *
 * 为什么必须提前写？
 *   因为下面 struct Shape 里面要用到 Shape 这个名字，
 *   但那时候结构体还没定义完，编译器还不认识它。
 *   先声明一下，编译器就知道“有这么个类型”。 */
typedef struct Shape Shape;

struct Shape {
    /* name：形状的名字，比如 "circle" */
    const char *name;

    /* 这一行是全篇最关键的，拆开看：

     *     float  (*area)  (const Shape *self);
     *       ^       ^              ^
     *       |       |              |
     *       |       |              +-- 它指向的函数，参数是 Shape 指针
     *       |       +----------------- 变量名叫 area，是个指针（括号不能省）
     *       +------------------------- 它指向的函数，返回值是 float
     *
     * 一句话：area 不是函数，而是一个「装函数地址的变量」。
     * 谁想当形状，就负责把自己的面积函数地址放进 area。
     *
     * 括号为什么不能省？
     *   写了括号 ( *area ) 才是“指向函数的指针”；
     *   不写括号 float *area(...) 就变成“返回 float* 的函数”，意思完全不同。 */
    float (*area)(const Shape *self);
};


/* ---------- 2. 两个具体的形状 ---------- */

/* 圆：第一个成员必须是 Shape。
 * 这样 Shape 就“长”在 Circle 的开头，两者地址相同。
 * 后面的 r 才是圆自己的数据。 */
typedef struct {
    Shape base;   /* 必须放第一个！这是 C 模拟“继承”的固定套路 */
    float r;      /* 半径 */
} Circle;

/* 矩形：同样的套路 */
typedef struct {
    Shape base;
    float w;
    float h;
} Rect;


/* ---------- 3. 每个形状自己实现 area ---------- */

/* static 的意思是：这个函数只在本文件里用，别的文件看不到。
 *
 * 参数是 Shape*，但实际传进来的地址其实是 &circle.base。
 * 因为 base 是 Circle 的第一个成员，它的地址和整个 Circle 的地址一样，
 * 所以可以安全地转回 Circle*，再取 r。 */
static float circle_area(const Shape *self)
{
    const Circle *c = (const Circle *)self;   /* 把“形状”转回“圆” */
    return 3.14159f * c->r * c->r;
}

static float rect_area(const Shape *self)
{
    const Rect *r = (const Rect *)self;
    return r->w * r->h;
}


/* ---------- 4. 只认 Shape 的通用代码 ---------- */

/* 这个函数不知道传进来的是圆还是矩形，
 * 它只认一件事：Shape 里有 area，调用它就能得到面积。
 * 这就是 C 里模拟“多态”的地方。 */
static void print_area(const Shape *s)
{
    /* s->area 是取出函数指针；
     * s->area(s) 是调用这个指针指向的函数，并把 s 传进去。 */
    printf("%-8s area = %.2f\n", s->name, s->area(s));
}


int main(void)
{
    /* 这里用嵌套花括号初始化：
     *
     *   { { "circle", circle_area }, 2.0f }
     *     ^^^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^
     *     |                           |
     *     |                           +-- r = 2.0f
     *     +------------------------------ base：名字是 circle，面积函数是 circle_area
     *
     * 注意 circle_area 后面没有括号：
     *   写 circle_area    是“取函数地址”；
     *   写 circle_area()  是“调用这个函数”。
     * 这里我们要的是地址，所以不能加括号。 */
    Circle c = { { "circle", circle_area }, 2.0f };
    Rect   r = { { "rect",   rect_area   }, 3.0f, 4.0f };

    /* &c.base：取 base 成员的地址，类型正好是 Shape*。
     * 因为 base 在最前面，这个地址也就是整个 Circle 的地址。 */
    print_area(&c.base);
    print_area(&r.base);

    return 0;
}
