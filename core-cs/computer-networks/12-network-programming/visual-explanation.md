# Visual Explanation: Event Loops & I/O Multiplexing

---

## 1. Thread-Per-Connection vs Reactor Architecture

### Thread-Per-Connection (Legacy)
```
  Client A ───[TCP]───> [ Thread A (Blocked in read())   ] ───> 4 MB Stack
  Client B ───[TCP]───> [ Thread B (Blocked in read())   ] ───> 4 MB Stack
  Client C ───[TCP]───> [ Thread C (Blocked in read())   ] ───> 4 MB Stack
  ...
  Client 10,000 ──────> [ Thread 10,000 (Blocked)        ] ───> 40,000 MB Stack!
                        [ OS Scheduler Thrashing / L1 Invalidation ]
```

### Reactor Event Loop (Modern)
```
  Client A ───┐
  Client B ───┼──> [ Non-Blocking Sockets ]
  Client C ───┤             │
  ...         │             ▼
  Client 100K ┘    ┌─────────────────────────────────────────┐
                   │       OS Kernel (epoll / kqueue)        │
                   │   RB-Tree of FDs -> Ready Queue List    │
                   └──────────────────┬──────────────────────┘
                                      │ epoll_wait / kevent
                                      ▼
                        ┌───────────────────────────┐
                        │   Single Event Loop       │
                        │   (1 Thread per Core)     │
                        │                           │
                        │ while (running) {         │
                        │   events = poll();        │
                        │   for (ev : events) {     │
                        │     dispatch_handler(ev); │
                        │   }                       │
                        │ }                         │
                        └───────────────────────────┘
```

---

## 2. Level-Triggered (LT) vs Edge-Triggered (ET) Timeline

```
Timeline: Packet Arrival vs Event Loop Notifications

Data Arrival:
       1000 bytes arrive
            │
            ▼
    [Socket Buffer: 1000 bytes]
            │
LT:   epoll_wait() wakes up -> Application reads 400 bytes -> [600 bytes remain]
            │
      epoll_wait() IMMEDIATELY wakes up again! (Buffer still has 600 bytes)
            │
      Application reads remaining 600 bytes -> [Buffer empty]
            │
      epoll_wait() sleeps until new data arrives.

--------------------------------------------------------------------------------

Data Arrival:
       1000 bytes arrive
            │
            ▼
    [Socket Buffer: 1000 bytes] (State Transition: EMPTY -> NON-EMPTY)
            │
ET:   epoll_wait() wakes up ONCE!
            │
      Application reads 400 bytes -> [600 bytes remain]
            │
      epoll_wait() DOES NOT WAKE UP! (No new state transition occurred)
            │
      STALL! Socket hangs forever unless application looped read() until EAGAIN!
```

---

## 3. The Non-Blocking Socket State Machine

```
      +-------------+
      |  ACCEPTED   |  (Set O_NONBLOCK, Register EVFILT_READ / EPOLLIN)
      +------+------+
             |
             v
      +-------------+        read() == EAGAIN
      |   READING   | ------------------------------+
      +------+------+                               |
             |                                      |
             | read() > 0 (Message Complete)        |
             v                                      |
      +-------------+                               |
      | PROCESSING  |                               |
      +------+------+                               |
             |                                      |
             v                                      |
      +-------------+                               |
      |   WRITING   | <-----------------------------+
      +------+------+
             | write() == EAGAIN (Socket buffer full)
             +--------------------+
             |                    | Register EVFILT_WRITE / EPOLLOUT
             v                    |
     (Wait for writable) <--------+
             |
             v (All bytes flushed)
      +-------------+
      |   CLOSING   | (Close fd, Free session memory)
      +-------------+
```
