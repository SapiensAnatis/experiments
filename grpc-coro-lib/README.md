# grpc-coro-lib

Learning exercise for coroutines: can they be integrated with the gRPC C++ callback API?

The answer appears to be yes, although only a simple streaming RPC has been attempted.

## Why?

In theory if we use the callback API and we can do blocking I/O work in coroutines, then
we could create a gRPC server that is less likely to suffer thread pool starvation. The
coroutines are integrated into the reactor flow, but will suspend and return when co_await
statements are hit, which should allow the gRPC server to resume doing other work while
our RPC waits for something to happen.

In the case that the co_await target is a gRPC method, like `Write()`, the coroutine will be
woken up by another reaction (e.g. `OnWriteDone()`). In the future, if I add coroutines
for other I/O like a PostgreSQL query, then that might end up being woken up by a polling
thread using `epoll` that watches a file descriptor.

Using coroutines to abstract over the callback API seems to also aid with a lot of usability
problems I have previously encountered with the callback API:

1. We no longer have to worry about reactor lifetimes and put `delete this;` in `OnDone`;
   we can just co_await `Finish` and know that `OnDone` is called after we resume from that
   await. That means that we can tie the lifetime of the reactor to the scope of our
   coroutine method with `std::unique_ptr`
2. If we can `co_await` a write in a "blocking" fashion (even though it actually yields
   control back to gRPC) then we don't have to worry about write races and ensuring that
   only one write is in flight at any given point


## How to build

1. Get a compiler that supports C++23
2. Set up vcpkg following https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash
3. Create a CMakeUserPresets.json as described in the above guide with your local VCPKG_ROOT
   variable
4. Build with CMake as per usual

