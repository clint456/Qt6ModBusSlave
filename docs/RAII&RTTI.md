# C++ 中的 RAII 与 RTTI

在 C++ 中，**RAII**（Resource Acquisition Is Initialization）和 **RTTI**（Run-Time Type Information）是两个用途完全不同但都非常重要的机制，分别用于**资源管理**和**运行时类型识别**。

---

## 🔒 RAII（Resource Acquisition Is Initialization）

### 核心思想
资源的获取与对象的生命周期绑定：
- 在**构造函数**中获取资源；
- 在**析构函数**中自动释放资源；
- 从而保证**异常安全**和**无资源泄漏**。

### 典型应用
- **智能指针**管理动态内存：
  - `std::unique_ptr`
  - `std::shared_ptr`
- **文件/网络资源**自动关闭句柄：
  - `std::fstream`
- **锁管理**自动加锁/解锁：
  - `std::lock_guard`
  - `std::scoped_lock`

### 示例：线程安全输出
```cpp
#include <mutex>
#include <iostream>

std::mutex mtx;

void safe_print() {
    std::lock_guard<std::mutex> lock(mtx); // 构造时加锁，析构时解锁
    std::cout << "线程安全输出\n";
}
```
> 这种方式避免了手动调用 `lock()`/`unlock()` 可能导致的遗漏或异常不安全。

---

## 🧠 RTTI（Run-Time Type Information）

### 功能
允许程序在**运行时识别对象的实际类型**，支持安全的类型转换。

### 启用条件
- 类必须具有**多态性**（即至少包含一个虚函数，如虚析构函数）。
- RTTI 信息通常存储在**虚函数表**（vtable）中。

### 主要工具
1. **`typeid`**  
   返回 `std::type_info` 对象，可用于比较类型。
2. **`dynamic_cast`**  
   在继承层次中安全地向下转型：
   - 指针转换失败 → 返回 `nullptr`
   - 引用转换失败 → 抛出 `std::bad_cast` 异常

### 示例：运行时类型判断
```cpp
#include <iostream>
#include <typeinfo>

class Base { 
public: 
    virtual ~Base() = default; 
};

class Derived : public Base {};

int main() {
    Base* b = new Derived();
    
    if (typeid(*b) == typeid(Derived)) {
        std::cout << "对象是 Derived 类型\n";
    }
    
    if (auto d = dynamic_cast<Derived*>(b)) {
        std::cout << "dynamic_cast 转换成功\n";
    }
    
    delete b;
}
```

---

## ✅ 开发建议

| 机制 | 建议 |
|------|------|
| **RAII** | **优先使用** RAII 管理所有资源（内存、文件、锁等），避免手动释放，提升代码安全性与可维护性。 |
| **RTTI** | **谨慎使用** RTTI。仅在确实需要运行时类型判断时启用；若可通过**虚函数**或多态接口、**模板**或**访问者模式**解决，应优先选择这些零开销或编译期方案，以避免 RTTI 带来的性能与二进制体积开销。 |

> 💡 提示：许多高性能 C++ 项目（如游戏引擎、嵌入式系统）会通过编译选项（如 `-fno-rtti`）禁用 RTTI 以优化性能。