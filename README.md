# Printer Spooler — Print Queue Simulator in C++

A terminal-based printer spooler that simulates how real operating systems manage print jobs. Built using a deque as the core data structure, supporting normal and urgent job insertion, mid-queue deletion, pause/resume, and real-time queue display.

---

## Problem Overview

A printer spooler is a buffer that holds print jobs sent by users and feeds them to the printer one at a time. In a real OS, the spooler sits between the application and the printer so the user does not have to wait for the printer to finish before doing something else.

The requirements for this implementation:

- Normal jobs enter at the rear of the queue
- Urgent jobs jump directly to the front
- Jobs are processed in FIFO order from the front
- Any job can be cancelled by its ID even if it is in the middle
- The queue should not waste memory as jobs are added and removed
- Page counts across all pending jobs must be tracked
- Individual jobs can be paused and resumed without being removed

---

## How to Compile and Run

```bash
g++ main.cpp -o main
./main
```

No external libraries needed. Works on any Linux or macOS terminal that supports ANSI escape codes.

---

## Data Structure

The entire program runs on a single `std::deque<Job>`:

```cpp
struct Job {
    int id;
    string name;
    int pages;
    string priority;
    bool paused;
};

deque<Job> q;
int nextId = 1;
```

A deque (double-ended queue) was chosen because it supports O(1) insertion and removal at both the front and the rear, as well as O(1) index-based access like an array. A regular `std::queue` only allows rear insertion and front removal with no index access, which makes mid-queue operations like cancel and pause impossible without rebuilding the queue.

The `Job` struct holds everything about a single print job. The `paused` boolean is the only state that gets modified in-place — every other field is set once when the job is created and never changed.

---

## Circular Queue Concept and Memory

The circular queue concept exists to solve a specific problem with naive array-based queues. Imagine a fixed array of size 10. You add jobs at position 0, 1, 2. You process and remove from position 0. Now position 0 is empty but your `front` pointer moved to position 1. Over time the front keeps moving right and you waste the empty slots at the beginning.

A circular queue wraps the rear pointer back to position 0 when it reaches the end, reusing those empty slots:

```
front = (front + 1) % capacity   // wrap around on dequeue
rear  = (rear  + 1) % capacity   // wrap around on enqueue
```

In this implementation `std::deque` handles all of this internally. When you call `pop_front()`, the memory at the front is released and becomes available again. When you call `push_back()`, it either reuses available space or allocates a new block. The deque manages this through an internal map of fixed-size memory blocks, which is exactly the circular memory reuse principle without you having to implement the modulo arithmetic yourself.

---

## Adding Jobs — Front and Rear Insertion

```cpp
void addJob(string name, int pages, string priority) {
    Job j;
    j.id = nextId;
    j.name = name;
    j.pages = pages;
    j.priority = priority;
    j.paused = false;
    nextId++;

    if (priority == "urgent")
        q.push_front(j);
    else
        q.push_back(j);
}
```

This is the part where the deque earns its place. A normal queue data structure has no `push_front`. It only has enqueue at the rear. But a deque has both ends open.

When a job comes in as urgent, `push_front` places it directly at position 0, in front of every existing job. Every other job shifts back by one index conceptually, but the deque does not physically move any of them — it just adjusts its internal front pointer. This makes `push_front` O(1) the same as `push_back`.

When a normal job arrives, `push_back` places it after all existing jobs, maintaining FIFO order among normal jobs.

---

## Processing Jobs — FIFO Dequeue

```cpp
void processJob() {
    if (q.empty()) {
        cout << "No jobs in queue.\n";
        return;
    }
    if (q.front().paused) {
        cout << "Front job is paused. Resume it first.\n";
        return;
    }
    cout << "Processing: " << q.front().name << "\n";
    q.pop_front();
}
```

Processing always happens from the front. This is the definition of FIFO — first in, first out. Whatever job is at index 0 gets printed next.

There are two guard checks before processing. First, the queue must not be empty. Second, the front job must not be paused. If the front job is paused, the spooler is blocked until that job is resumed or cancelled, which mirrors how a real print queue behaves when a job stalls.

`pop_front()` removes the front element and is O(1). After this call, what was at index 1 is now at index 0, and so on.

---

## Cancel by ID — Mid-Queue Deletion

```cpp
void cancelJob(int id) {
    for (int i = 0; i < q.size(); i++) {
        if (q[i].id == id) {
            cout << "Cancelled job: " << q[i].name << "\n";
            q.erase(q.begin() + i);
            return;
        }
    }
    cout << "Job ID not found.\n";
}
```

This is the operation that a basic queue cannot do. In a standard queue you can only remove from the front. To cancel a job in the middle you would have to dequeue everything before it, remove the target, then re-enqueue everything — which destroys the order and is expensive.

Because `std::deque` supports index access with `q[i]`, we can loop through and find the job by ID in O(n) time, then call `erase` to remove it from whatever position it is in. The deque internally shifts the remaining elements to fill the gap.

`q.begin() + i` is an iterator pointing to position `i`. `erase` takes an iterator, not an index, which is why this conversion is necessary.

After `erase`, the job is gone from the queue. All jobs behind it move one position forward. Jobs ahead of it are untouched.

---

## Display and Page Counter

```cpp
void displayQueue() {
    int totalPages = 0;
    for (int i = 0; i < q.size(); i++) {
        totalPages += q[i].pages;
    }

    for (int i = 0; i < q.size(); i++) {
        Job j = q[i];
        // print job details with color coding
    }
}
```

Both the display and the page counter use the same kind of linear traversal — loop from index 0 to `q.size() - 1` and either print or accumulate.

The page counter is a running sum. Start `total` at 0 and add each job's page count to it. After the loop, `total` holds the sum of all pending pages. This is O(n) where n is the number of jobs currently in the queue.

The display loop uses `q[i]` for direct index access. This is O(1) per access because deque stores elements in contiguous blocks and can calculate the block and offset for any index without traversing from the front. A linked list cannot do this — it would have to walk node by node to reach `q[i]`, making display O(n squared) for all n elements.

---

## Pause and Resume — In-Place State Toggle

```cpp
void togglePause(int id) {
    for (int i = 0; i < q.size(); i++) {
        if (q[i].id == id) {
            q[i].paused = !q[i].paused;
            if (q[i].paused)
                cout << "Job #" << id << " paused.\n";
            else
                cout << "Job #" << id << " resumed.\n";
            return;
        }
    }
    cout << "Job ID not found.\n";
}
```

Pause and resume do not remove the job from the queue. The job stays exactly where it is. Only the `paused` field inside the struct is flipped.

`!q[i].paused` is boolean negation — if it was `true` it becomes `false`, if it was `false` it becomes `true`. This is how a toggle works without needing two separate functions.

This is only possible because deque gives you reference access to elements by index. `q[i].paused = ...` directly modifies the struct inside the deque. With a standard queue you would have no way to reach an element in the middle and modify it without removing it first.

---

## Promote to Front

```cpp
void promoteJob(int id) {
    for (int i = 0; i < q.size(); i++) {
        if (q[i].id == id) {
            Job j = q[i];
            q.erase(q.begin() + i);
            q.push_front(j);
            return;
        }
    }
    cout << "Job ID not found.\n";
}
```

Promotion is a combination of two operations — erase from current position, then push to front. The job is copied out first, erased, and then re-inserted at the front. The order matters. If you push to front first, the indices shift and your `erase` call hits the wrong element.

This is the custom feature that extends the deque behavior beyond what the problem statement requires for individual roles. It is useful when a job that was added as normal suddenly becomes urgent after it is already in the queue.

---

## Complexity Summary

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| Add normal job | O(1) | push_back |
| Add urgent job | O(1) | push_front |
| Process next job | O(1) | pop_front |
| Cancel by ID | O(n) | linear search then erase |
| Pause/resume | O(n) | linear search |
| Promote to front | O(n) | linear search then erase + push_front |
| Display all | O(n) | full traversal |
| Count pages | O(n) | full traversal with accumulator |

---

## Why Not Other Data Structures

A plain array queue would require shifting all elements when inserting at the front, making urgent insertion O(n). It also wastes memory at the front as elements are dequeued unless circular indexing is implemented manually.

A linked list supports O(1) front and rear insertion but has no index access. Cancelling or pausing a job requires traversal with a pointer, which works but means you cannot write `q[i]` — you have to use a pointer or iterator that you walk forward manually.

A standard `std::queue` in C++ is a restricted wrapper. It intentionally hides everything except `push`, `pop`, `front`, and `back`. It cannot do mid-deletion, index access, or front insertion. It is not suitable for this problem.

`std::deque` is the correct choice because it satisfies every requirement of the problem: O(1) at both ends, O(1) index access, and support for mid-queue operations through iterators.

---

## File Structure

```
spooler.cpp    — entire program, single file
README.md      — this file
```

---

## Role Division

The code is divided into four logical sections corresponding to four contributors. Each section is self-contained and depends only on the shared `deque<Job> q` global and the `Job` struct.

- Data structure definition and deque declaration
- Job insertion logic with priority-based front and rear routing
- Job processing and mid-queue cancellation by ID
- Queue display, page counter, pause/resume toggle, and menu loop
