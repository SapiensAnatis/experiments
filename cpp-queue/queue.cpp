#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

template <typename T>
class LockFreeQueue {
   private:
    struct LinkedListNode {
        explicit LinkedListNode(T value) : next(nullptr), value(value) {}
        LinkedListNode(LinkedListNode* previous, T value) : next(previous), value(value) {}

        std::atomic<LinkedListNode*> next;
        T value;
    };

    static_assert(std::atomic<LinkedListNode*>::is_always_lock_free);

   public:
    std::optional<T> Dequeue() {
        auto endPtr = end.load();

        if (endPtr == nullptr) {
            return std::nullopt;
        }

        // don't know how to make this work
        while (!end.compare_exchange_strong(endPtr, endPtr->previous.load())) {
            endPtr = end.load();
        }

        auto value = endPtr->value;

        // compare_exchange needs an lvalue
        LinkedListNode* nullPointer = nullptr;

        // set start to nullptr if it was the same as the end
        start.compare_exchange_strong(endPtr, nullPointer);

        // memory leak -- hazard pointer?

        return value;
    }

    void Enqueue(T value) {
        auto node = new LinkedListNode(value);

        LinkedListNode* nullPointer = nullptr;

        if (start.compare_exchange_strong(nullPointer, node)) {
            auto endPtr = end.load();

            while (!end.compare_exchange_strong(endPtr, node)) {
                endPtr = end.load();
            }

            return;
        }

        auto currentStart = start.load();
        node->next.store(currentStart);

        while (!start.compare_exchange_strong(currentStart, node)) {
            currentStart = start.load();
            node->next.store(currentStart);
        }
    }

   private:
    std::atomic<LinkedListNode*> start = nullptr;
    std::atomic<LinkedListNode*> end = nullptr;
};

int main() {
    LockFreeQueue<int16_t> queue;

    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Enqueue(3);

    std::optional<int16_t> value = queue.Dequeue();

    while (value.has_value()) {
        std::cout << "Dequeued: " << value.value() << "\n";
        value = queue.Dequeue();
    }
}