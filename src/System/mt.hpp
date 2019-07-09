#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

#define _XOPEN_SOURCE
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <ucontext.h>
#undef _XOPEN_SOURCE

namespace mt {

template<typename T> using decay_t = typename std::decay<T>::type;
template<typename T> using result_of_t = typename std::result_of<T>::type;

////////////////////////////////////////////////////////////////////////////////
// Fiber
////////////////////////////////////////////////////////////////////////////////
class Fiber
{
public:
    virtual ~Fiber() = default;
    virtual void schedule() = 0;
    virtual void yield() = 0;

    static inline Fiber* get();

protected:
    static thread_local Fiber* current;
};

Fiber* Fiber::get() { return current; }

////////////////////////////////////////////////////////////////////////////////
// ConditionVariable
////////////////////////////////////////////////////////////////////////////////
class ConditionVariable
{
public:
    inline void notify_one();
    inline void notify_all();

    template <typename Predicate>
    inline void wait(std::unique_lock<std::mutex>& lock, Predicate pred);
private:
    std::mutex mutex;
    std::list<Fiber*> waiting;
};

void ConditionVariable::notify_one()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (waiting.size() > 0)
    {
        auto fiber = waiting.back();
        waiting.pop_back();
        fiber->schedule();
    }
}

void ConditionVariable::notify_all()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (waiting.size() > 0)
    {
        auto fiber = waiting.back();
        waiting.pop_back();
        fiber->schedule();
    }
}

template <typename Predicate>
void ConditionVariable::wait(std::unique_lock<std::mutex>& lock, Predicate pred)
{
    while (!pred())
    {
        lock.unlock();

        auto fiber = Fiber::get();
        {
            std::unique_lock<std::mutex> waitingLock(mutex);
            waiting.push_back(fiber);
        }
        fiber->yield();

        lock.lock();
    }
}

////////////////////////////////////////////////////////////////////////////////
// PoolBase
////////////////////////////////////////////////////////////////////////////////
template <typename T>
class PoolBase
{
protected:
    struct Item
    {
        T data;
        std::atomic<int> refcount = {0};
        union
        {
            Item *next;
            PoolBase *pool;
        };
    };

public:
    class Loan
    {
    public:
        inline Loan(Item*);
        inline Loan(const Loan&);
        inline Loan(Loan&&);
        inline ~Loan();
        inline Loan& operator = (const Loan&);
        inline Loan& operator = (Loan&&);
        inline T& operator * ();
        inline T* operator -> () const;
        inline T* get() const;

    private:
        Loan() = delete;
        Item *item;
    };

    inline Loan borrow();

protected:
    inline void return_(Item *data);

    Item *free = nullptr;
    std::mutex mutex;
    ConditionVariable returned;
};

template <typename T>
using Loan = typename PoolBase<T>::Loan;

template <typename T>
PoolBase<T>::Loan::Loan(Item* item) : item(item) { item->refcount++; }

template <typename T>
PoolBase<T>::Loan::Loan(const Loan& other) : item(other.item)
{
    item->refcount++;
}

template <typename T>
PoolBase<T>::Loan::Loan(Loan&& other) : item(other.item)
{
    other.item = nullptr;
}

template <typename T>
PoolBase<T>::Loan::~Loan()
{
    if (item != nullptr)
    {
        auto refs = item->refcount--;
        if (refs == 0)
        {
            item->pool->return_(item);
        }
    }
}

template <typename T>
typename PoolBase<T>::Loan& PoolBase<T>::Loan::operator = (const PoolBase<T>::Loan& rhs)
{
    rhs->item->refcount++;
}

template <typename T>
typename PoolBase<T>::Loan& PoolBase<T>::Loan::operator = (PoolBase<T>::Loan&& rhs)
{
    item = rhs.item;
    rhs.item = nullptr;
}

template <typename T>
T& PoolBase<T>::Loan::operator * () { return item->data; }

template <typename T>
T* PoolBase<T>::Loan::operator -> () const { return &item->data; }

template <typename T>
T* PoolBase<T>::Loan::get() const { return &item->data; }

template <typename T>
typename PoolBase<T>::Loan PoolBase<T>::borrow()
{
    std::unique_lock<std::mutex> lock(mutex);
    returned.wait(lock, [&] { return free != nullptr; });
    auto item = free;
    free = free->next;
    item->pool = this;
    lock.unlock();
    return Loan(item);
}

template <typename T>
void PoolBase<T>::return_(Item* item)
{
    std::unique_lock<std::mutex> lock(mutex);
    item->next = free;
    free = item;
    returned.notify_one();
}

////////////////////////////////////////////////////////////////////////////////
// Pool
////////////////////////////////////////////////////////////////////////////////
template <typename T, int N>
class Pool : public PoolBase<T>
{
public:
    using Item = typename PoolBase<T>::Item;

    Pool();

private:
    std::array<Item, N> items;
};

template <typename T, int N>
Pool<T, N>::Pool()
{
    for (int i = 0; i < N; i++)
    {
        items[i].next = this->free;
        this->free = &items[i];
    }
}

////////////////////////////////////////////////////////////////////////////////
// WaitGroup
////////////////////////////////////////////////////////////////////////////////
class WaitGroup
{
public:
    inline void add(int n);
    inline void done();
    inline void wait();
private:
    int count;
    ConditionVariable condition;
    std::mutex mutex;
};

void WaitGroup::add(int n)
{
    std::unique_lock<std::mutex> lock(mutex);
    count += n;
}

void WaitGroup::done()
{
    std::unique_lock<std::mutex> lock(mutex);
    count--;
    condition.notify_one();
}

void WaitGroup::wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [this]{ return count == 0; });
}

////////////////////////////////////////////////////////////////////////////////
// Scheduler
////////////////////////////////////////////////////////////////////////////////
class Scheduler
{
public:
    inline Scheduler();

    template<typename Function, typename ... Args>
    inline void run(Function&& f, Args&& ... args);

    inline void setThreadCount(size_t count);
    inline size_t getThreadCount() const;

private:
    class Worker;

    class Fiber : public mt::Fiber
    {
    public:
        inline Fiber(Worker *worker);

        inline void schedule() override;
        inline void yield() override;
        inline void resume();
    private:
        Worker *worker;
        ucontext_t context;
        char *stack;
    };

    using Task = std::function<void()>;
    using TaskQueue = std::queue<Task>;
    using FiberQueue = std::queue<Fiber*>;

    class Worker
    {
    public:
        inline Worker(Scheduler *scheduler);
        inline ~Worker();

        inline void yield(Fiber *fiber);
        inline void enqueue(Fiber *fiber);
        inline void enqueue(const Task &task);

        inline void run();

    private:
        Scheduler * const scheduler;
        std::thread thread;
        FiberQueue fibers;
        TaskQueue tasks;
        Fiber* fiber = nullptr;
        bool shutdown = false;
        std::condition_variable condition;
        std::mutex mutex;
    };

    inline void enqueue(const Task& task);

    std::vector<Worker*> workers;
    std::mutex workersMutex;

    std::list<Fiber*> blocked;
    std::mutex blockedMutex;
};

Scheduler::Scheduler()
{
    setThreadCount(1);
}

template<typename Function, typename ... Args>
void Scheduler::run(Function&& f, Args&& ... args)
{
    enqueue(std::bind(f, args...));
}

void Scheduler::setThreadCount(size_t count)
{
    std::unique_lock<std::mutex> lock(workersMutex);
    while (workers.size() > count)
    {
        auto worker = workers.back();
        workers.pop_back();
        delete worker;
    }
    while (workers.size() < count)
    {
        workers.push_back(new Worker(this));
    }
}

void Scheduler::enqueue(const Task& task)
{
    std::unique_lock<std::mutex> lock(workersMutex);
    workers[0]->enqueue(task);
}

Scheduler::Fiber::Fiber(Worker *worker) : worker(worker)
{
    union SplitPtr
    {
        void *ptr;
        struct { int a; int b; };
    };

    struct Target
    {
        static void Main(int a, int b)
        {
            SplitPtr u;
            u.a = a; u.b = b;
            auto self = reinterpret_cast<Target*>(u.ptr);
            self->worker->run();
        }

        Worker *worker;
    };

    Target target = {worker};
    SplitPtr args;
    args.ptr = &target;

    constexpr size_t stackSize = 65536;
    getcontext(&context);
    context.uc_stack.ss_sp = new uint8_t[stackSize];
    context.uc_stack.ss_size = stackSize;
    context.uc_link = nullptr;  // TODO
    makecontext(&context, reinterpret_cast<void(*)()>(&Target::Main), 2, args.a, args.b);
}

void Scheduler::Fiber::schedule()
{
    worker->enqueue(this);
}

void Scheduler::Fiber::yield()
{
    worker->yield(this);
}

void Scheduler::Fiber::resume()
{
    swapcontext(&static_cast<Scheduler::Fiber*>(current)->context, &context);
    current = this;
}

Scheduler::Worker::Worker(Scheduler *scheduler) : scheduler(scheduler), thread(&Worker::run, this) {}

Scheduler::Worker::~Worker()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (tasks.size() > 0)
    {
        scheduler->enqueue(tasks.back());
        tasks.pop();
    }
    shutdown = true;
    condition.notify_all();
    thread.join();
}

void Scheduler::Worker::enqueue(Fiber* fiber)
{
    std::unique_lock<std::mutex> lock(mutex);
    fibers.push(fiber);
    condition.notify_one(); // TODO: Only notify if idle
}

void Scheduler::Worker::enqueue(const Task& task)
{
    std::unique_lock<std::mutex> lock(mutex);
    tasks.push(task);
    condition.notify_one(); // TODO: Only notify if idle
}

void Scheduler::Worker::yield(Fiber *fiber)
{
    // Current worker fiber is yielding as it is blocked.
    // Spawn a new fiber.
    (new Fiber(this))->resume();
}

void Scheduler::Worker::run()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (true)
    {
        condition.wait(lock, [&] { return fibers.size() > 0 || tasks.size() > 0 || shutdown; });
        if (shutdown){ return; }

        if (fibers.size() > 0)
        {
            // Multiple fibers are now unblocked.
            // TODO: Release this fiber.
            auto fiber = fibers.back();
            fibers.pop();
            lock.unlock();
            fiber->resume();
            lock.lock();
        }

        if (tasks.size() > 0)
        {
            auto task = tasks.back();
            tasks.pop();
            lock.unlock();
            task();
            lock.lock();
        }
    }
}













////////////////////////////////////////////////////////////////////////////////
/*
template <typename T>
class Future
{
public:
    T get();

    template<typename Function, typename ... Args>
    Future then(Function&& f, Args&&... args);

private:
    std::shared_ptr<Scheduler> scheduler;
};

class Task
{
public:
    template<typename Function, typename ... Args>
    static Task* create(Function&& f, Args&&... args);

    virtual void run() = 0;
    virtual bool ready() = 0;
};

template<typename Function, typename ... Args>
Task* Task::create(Function&& f, Args&&... args)
{
    // using FuncTy = decay_t<Function>(decay_t<Args>...);
    using Result = result_of_t<decay_t<Function>(decay_t<Args>...)>;

    std::function<Result()> call = std::bind(f, args ...);

    return Task();
}

class Scheduler
{
public:
    template<typename Function, typename ... Args>
    Future<result_of_t<decay_t<Function>(decay_t<Args>...)>> enqueue(Function&& f, Args&&... args);
};

template<typename Function, typename ... Args>
Future<result_of_t<decay_t<Function>(decay_t<Args>...)>> Scheduler::enqueue(Function&& f, Args&&... args)
{
    auto task = Task::create(f, args);
    return task.future;
}


// class Queue
// {
// public:
//     std::vector<Task*> tasks;
// };

class Executor
{
public:
};
 */

#pragma clang diagnostic pop

} // namespace mt
