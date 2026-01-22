# 本项目涉及的知识点
## 1.面向对象（OOP）特性
### 1.1 类集成与Q_OBJECT宏

```cpp
class ModbusServer : public QObject
{
    Q_OBJECT
```

- `class ModbusServer : public QObject` - 继承`QObject`，这是Qt框架的基础类；
- `Q_OBJECT`宏 - 启用Qt元对象系统、支持信号槽、属性等高级特性；
- 所有需要使用信号槽的类都必须继承自`QObject`并声明`Q_OBJECT`

### 1.2 构造函数与成员初始化列表

```cpp
ModbusServer::ModbusServer(QObject *parent)
    : QObject(parent)
    , m_tcpServer(nullptr)
    , m_serialPort(nullptr)
    , m_rtuTimer(nullptr)
    , m_running(false)
    , m_mode(ModeTCP)
    , m_requestCount(0)
    , m_lastFunctionCode(0)
{
    // 创建数据存储
    m_dataStore = new ModbusDataStore(this);
```

- 成员初始化列表`（: QObject(parent), m_tcpServer(nullptr)...）`- 在构造函数体执行前初始化成员变量，效率更高
- nullptr - C++11 的空指针常量，替代了老的NULL宏
- parent-child关系 - new ModbusDataStore(this) 表示新对象的父对象是`this`，当父对象销毁时对象会自动销毁

### 1.3 访问控制修饰符与Q_INVOKABLE

```cpp
public:
    explicit ModbusServer(QObject *parent = nullptr);
    Q_INVOKABLE bool startTcp(quint16 port = 502);

private slots:
    void onNewTcpConnection();

private:
    QTcpServer *m_tcpServer;
```

- public - 公共成员接口或属性，QML和外部代码可以调用
- Q_INVOKABLE - 修饰符，表示方法可以从QML或元对象系统调用
- private slots - 私有槽函数，只能在类内部或通过信号连接调用
- private - 私有成员接口或属性，外部无法直接访问

## C++11/C++14标准特性
### 2.1 类型安全枚举`(enum class)`

```cpp
enum ModbusFunctionCode : quint8 {
    ReadCoils = 0x01,               // 读线圈
    ReadDiscreteInputs = 0x02,      // 读离散输入
    WriteSingleCoil = 0x05,         // 写单个线圈
    WriteMultipleCoils = 0x0F       // 写多个线圈
};
```

- `enum ModbusFunctionCode : quint8` - 指定底层类型为 `quint8`（无符号8位整数）
    - 类型安全 - 不同枚举类型不能隐式转换
    - 内存高效 - 只占用1字节
    - 明确说明枚举值的范围

### constexpr 编译器常量

```cpp
namespace ModbusConst {
    constexpr quint16 MAX_COILS = 65535;
    constexpr quint16 MAX_REGISTERS = 65535;
    constexpr quint16 MAX_READ_COILS = 2000;
    constexpr quint16 MAX_READ_REGISTERS = 125;
}
```

- `constexpr` - 编译器常量，在编译时计算，运行时不占用额外开销
- 优于`const`的地方：可用于模板参数、数组大小等需要编译常量的场景
- 通过命名空间`ModbusConst`组织常量，避免全局污染

### 2.3 const修饰成员函数
```cpp
bool isRunning() const { return m_running; }
ModbusMode mode() const { return m_mode; }
QString statusMessage() const { return m_statusMessage; }
```

- `const`修饰的成员函数保证该函数不会修改对象的状态
- 允许在常对象上调用该函数, 如`const ModbusServer::ModbusServer(QObject *parent)`
- `const`对象的成员函数被认为是逻辑不变的

### 2.4 Lambda表达式
C++11引入的**Lambda**表达式

```txt
[capture](parameters) -> return_type { body }
```

- [capture]：捕获列表（决定能访问哪些外部变量）
- (parameters)：参数列表（和普通函数一样）
- -> return_type：返回类型（可省略，编译器自动推导）
- { body }：函数体

Lambda表达式好处
- 无需声明额外槽函数，逻辑内聚；
- 代码更紧凑，信号处理逻辑就在连接处，一目了然；
- 适合简单、一次性的响应逻辑（如计数、转发信号、打日志等）。

本项目中
```cpp
connect(m_functionHandler, &ModbusFunctionHandler::requestProcessed,
        this, [this](quint8 fc, bool success) {
    incrementRequestCount();
    emit requestReceived(fc);
});
```

- [this](quint8 fc, bool success){ ... } - Lambda 表达式
- [this] - 捕获列表，`this`表示按值捕获当前对象指针
- 参数 `(quint8 fc,bool success)` - Lambda的参数列表
- { ... } - Lambda函数体
- 优点：代码更加紧凑，信号槽连接更加清晰

[Lambda详细讲解](./Lambda.md)

## 3. QT 框架特性
### 3.1 属性系统(Q_PROPERTY)

```cpp
Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
Q_PROPERTY(ModbusMode mode READ mode NOTIFY modeChanged)
Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
```
- `Q_PROPERTY` - 为类型定义属性，支持元对象系统
- 语法：`Q_PROPERTY(type name READ getter NOTIFY signal)`
    - `bool running` - 属性类型和名称
    - `READ isRunning` - 读取方法(getter)
    - `NOTIFY runningChanged` - 属性变化时发送的信号
- 优点：
    - QML可以直接绑定这些属性
    - 自动处理属性变化通知
    - 反射特性系统支持

[QT反射机制详细讲解](./Qt_Reflection.md)

### 3.2 信号与槽机制

```cpp
signals:
    void runningChanged(bool running);
    void modeChanged(ModbusMode mode);
    void errorOccurred(const QString &error);

private slots:
    void onNewTcpConnection();
    void onTcpReadyRead();
```

- `signals` - 定义信号（可以发出但是不能调用），用于同时其他对象事件
- `private slots` - 定义槽函数（也可被调用，也可以连接信号），加上private的代表只能当前对象能够访问
- 信号连接示例：

```cpp
  connect(m_tcpServer, &QTcpServer::newConnection, 
          this, &ModbusServer::onNewTcpConnection);
```

- 优点：
    - 松耦合通信机制
    - 自动线程安全（QT 会在适当的线程之中执行槽函数）
    - 一个信号可以连接多个槽函数，一个槽函数也可以连接多个信号

### 3.3 QT容器类

```cpp
QList<QTcpSocket*> m_tcpClients;
QMap<QTcpSocket*, QByteArray> m_tcpBuffers;
```

- `QList<QTcpSocket*>` - 动态数组(类似`std:vendor`) 
- `QMap<QTcpSocket*, QByteArray>` - 关联数组(类似`std:map`)
- 为什么用QT容器不用STL
    - 与Qt信号槽系统集成
    - 自动管理内存
    - 多线程友好

### 3.4 QT特定的数据类型
```cpp
QMap<quint16, bool> m_coils;
QMap<quint16, quint16> m_holdingRegisters;
QBitArray m_values;
QByteArray m_data;
```
- quint8, quint16 - Qt 定义的无符号整数类型（确保跨平台一致性）
- QBitArray - 优化的位数组，每个元素占用1个bit
- QByteArray - 字节数组，适合网络数据
- QString - Unicode 字符串（自动处理编码）

## 4. 并发与线程安全
### 4.1 读写锁`（QReadWriteLock）`
```cpp
bool ModbusDataStore::readCoil(quint16 address) const
{
    QReadLocker locker(&m_coilsLock);
    return m_coils.value(address, false);
}

bool ModbusDataStore::writeCoil(quint16 address, bool value)
{
    QWriteLocker locker(&m_coilsLock);
    m_coils[address] = value;
    emit coilChanged(address, value);
    return true;
}
```
- `QReadLocker / QWriteLocker` - RAII 风格的锁管理
-   **RAII (Resource Acquisition Is Initialization)**：
    - 资源获取时即初始化（在构造函数中获取锁）
    - 作用域结束时自动释放（在析构函数中释放锁）
- `mutable QReadWriteLock m_coilsLock` - 允许在 `const` 方法中修改锁
- 读写锁的优势：
    - 多个线程可以同时读取
    - 写入时必须独占访问
    - 比普通互斥锁更高效

[RAII与RTTI详细解释](./RAII&RTTI.md)

### 4.2 mutable关键字
```cpp
mutable QReadWriteLock m_coilsLock;
mutable QReadWriteLock m_discreteInputsLock;
```
- `mutable` 允许在`const`成员函数中修改该成员变量
- 用途: 在逻辑上不改变对象状态的操作中使用互斥锁

[mutable关键字详解](./mutable.md)


# 5. 模板编程（Template Programming）
C++模板编程是泛型编程（Generic Programming）的核心机制。它允许编写与类型无关的代码，编译器会在使用时根据具体类型”实例化”出对应的代码。

## 5.1 泛型容器
```cpp
QMap<QString, int> nameToAge;        // 键是 QString，值是 int
QVector<double> temperatures;        // 元素是 double
```

- `QMap<K, V>` 和 `QVector<T>` 都是类模板（class templates）
- 编译器会为每一种不同的类型组合生成一份独立的代码
    - `QVentor<int>`和`QVector<Qstring>`时两个完全不同的类型
    - 它们的成员函数（如`push_back`、`opertor[]`）也会被分别实例化
- 模板的优点

| 优点 | 说明 |
|------|------|
| 类型安全 | 编译器知道容器中存的是什么类型，禁止非法操作（如向 `QVector<int>` 插入 `QString`）。 |
| 零运行时开销 | 没有虚函数、没有类型擦除、没有动态分发；所有代码在编译期确定，性能等同于手写专用版本。 |
| 代码重用 | 一套模板代码可适用于无数类型，避免重复造轮子。 |

> 💡 对比 Java/C# 的泛型：C++ 模板是“代码生成器”，而 Java 泛型是“类型擦除”，运行时无泛型信息。C++ 更高效，但可能增大二进制体积（因多份实例化）。


本项目中
```cpp
QMap<quint16, bool> m_coils;  
QMap<quint16, quint16> m_holdingRegisters;
QVector<quint16> m_values;
```

- `QMap<key_type, value_type>` - 模板类，支持任意键值类型
- `QVector<element_type>` - 模板类，支持任意元素类型
- 编译器根据模板参数生成特定类型的代码
- 模板的优点：
    - 类型安全
    - 零运行时开销（编译时展开）
    - 代码重用

### 5.2 模板在Qt信号槽中的应用
Qt从Qt5开始引入了基于函数指针和模板的新式信号槽语法，取代了旧的字符串形式（SIGNAL()/SLOT() 宏）。
- 新式连接语法
```cpp
connect(button, &QPushButton::clicked, this, &MyWidget::handleClick);
```
这里的`connect`是一个函数模板，其简化原型如下
```cpp
template<typename Sender, typename Signal, typename Receiver, typename Slot>
void connect(const Sender* sender, Signal signal, 
             const Receiver* receiver, Slot slot);
```

类型安全如何实现？
1. 编译时解析函数签名
`&QPushButton::clicked`是一个指向成员函数的指针，编译器知道它的完整签名（比如 `void clicked(bool checked = false)`）。
2. 参数类型自动匹配
Qt的connect 模板内部会检查：
- 信号的参数类型是否兼容槽函数的参数类型（支持隐式转换，且槽的参数可以少于信号）；
- 如果不兼容（如信号发 int，槽收 QString），编译时报错！
```cpp
// ❌ 编译错误：类型不匹配
connect(timer, &QTimer::timeout, this, &MyClass::processData); 
// 假设 processData 需要 int 参数，但 timeout() 无参数 → 不匹配
```

3. 无需宏、无需字符串
老写法
```cpp
connect(button, SIGNAL(clicked()), this, SLOT(handleClick()));
```
- 字符串在运行时解析，无法在编译期检查；
- 重命名函数后容易失效("幽灵连接")
- 不支持Lambda表达式

4. 支持 Lambda 表达式（也是模板的功劳）
```cpp
connect(timer, &QTimer::timeout, [=]() {
    qDebug() << "Tick!";
});
```
- Lambda 被当作一个可调用对象传入 connect 模板；
- 编译器推导其类型并生成适配代码；
- 依然享受类型安全 + 编译期检查。

| 特性 | 旧式信号槽（Qt4 风格） | 新式信号槽（Qt5+ 模板） |
|------|------------------------|--------------------------|
| 类型检查 | 运行时（通过字符串匹配） | 编译时（强类型） |
| 安全性 | 容易出错（拼写错误、签名变更） | 高安全性，错误早暴露 |
| 性能 | 稍慢（需查找元对象系统） | 更快（直接函数指针调用） |
| 功能 | 不支持 lambda | 支持 lambda、任意可调用对象 |
| 可维护性 | 差 | 优秀 |


## 6. 智能指针与内存管理
### 6.1 QT的父子对象管理
```cpp
m_dataStore = new ModbusDataStore(this);
m_functionHandler = new ModbusFunctionHandler(m_dataStore, this);
m_fileStore = new FileStore(this);
```

- 所有对象的第二个参数`this`表示`ModbusServer`其父对象
- 当父对象销毁时，子对象会自动被销毁
- 优点：
    - 不需要手动`delete`子对象
    - 防止内存泄漏
    - 建立清晰的所有权关系

### 6.2 `deleteLater()延迟删除`

```cpp
for (QTcpSocket *socket : m_tcpClients) {
    socket->disconnectFromHost();
    socket->deleteLater();
}
```

- `deleteLater()` - 不立即删除，而是在事件循环中安排删除
- 为什么用这个不用`delete`
    - socket可能还有待处理的时间 ；
    - 直接`delete`可能导致访问已释放的对象；
    - `deleteLater` 确保所有事件处理完后再删除。

## 7. 强类型系统与类型转换
### 7.1 静态类型转换
```cpp
QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
if (!socket) return;
```

- `qobject_cast<>` - Qt提供的类型安全的转换
- 类似C++的`dynamic_cast`
- 失败时返回`nullptr`而不是抛出异常

### 7.2 大端/小端转换
```cpp
quint16 transactionId = qFromBigEndian<quint16>(
    reinterpret_cast<const uchar*>(adu.data())
);
```

- `qFromBigEndian<quint16>()` - 从大端字节序转换为本机字节序
- `reinterpret_cast<const uchar*>()` - 将字节指针类型转换
- 网络编程中常用：确保不同平台间数据格式一致

## 8. 命名空间
```cpp
namespace ModbusConst {
    constexpr quint16 MAX_COILS = 65535;
    constexpr quint16 MAX_REGISTERS = 65535;
}
```
- `namespace ModbusConst` - 将常量组织在命名空间中
- 避免全局符号污染
- 使用时：`ModbusConst::MAX_COILS`