#include <memory>
#include <mutex>
#include <vector>
#include <condition_variable>

namespace mt {

template <typename T, int N>
class Pool
{
    struct Item
    {
        T data;
        union
        {
            Item *next;
            Pool *pool;
        };
    };

public:
    using Loan = std::unique_ptr<T, void(*)(T*)>;

    Pool();
    Loan take();

private:
    static void release(T *data);

    std::array<Item, N> items;
    std::mutex mutex;
    std::condition_variable returned;
    Item *free = nullptr;
};

template <typename T, int N>
Pool<T, N>::Pool()
{
    for (int i = 0; i < N; i++)
    {
        items[i].next = free;
        free = &items[i];
    }
}

template <typename T, int N>
typename Pool<T, N>::Loan Pool<T, N>::take()
{
    std::unique_lock<std::mutex> lock(mutex);
    returned.wait(lock, [&] { return free != nullptr; });
    auto item = free;
    free = free->next;
    item->pool = this;
    lock.unlock();
    return Loan(&item->data, &release);
}

template <typename T, int N>
void Pool<T, N>::release(T *ptr)
{
    auto item = reinterpret_cast<Item*>(ptr);
    auto pool = item->pool;

    std::unique_lock<std::mutex> lock(pool->mutex);
    item->next = pool->free;
    pool->free = item;
    pool->returned.notify_one();
}

class TaskBase
{
public:
    virtual ~TaskBase() = default;
};

template <typename OUT, typename IN>
class Task : public TaskBase
{
public:
    virtual void execute() = 0;
    virtual bool ready() = 0;
};

class Queue
{
public:
    std::vector<TaskBase*> tasks;
};

class Executor
{
public:
};

class Scheduler
{
public:
    void enqueue(TaskBase* task);
};

} // namespace mt