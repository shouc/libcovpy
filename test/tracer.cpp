//
// Created by shou on 2/14/21.
//

//
// BEGIN FUZZING CODE
//
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#define REPRL_CRFD 100
#define REPRL_CWFD 101
#define REPRL_DRFD 102
#define REPRL_DWFD 103

#define SHM_SIZE 0x100000
#define MAX_EDGES ((SHM_SIZE - 4) * 8)

#define CHECK(cond)                            \
  if (!(cond)) {                               \
    fprintf(stderr, "\"" #cond "\" failed\n"); \
    _exit(-1);                                 \
  }

struct SharedMemData {
    uint32_t numEdges;
    unsigned char edges[];
};

struct SharedMemData *shmem;

uint32_t *__edges_start, *__edges_stop;
void __sanitizer_cov_reset_edgeguards() {
    uint64_t N = 0;
    for (uint32_t *x = __edges_start; x < __edges_stop && N < MAX_EDGES; x++)
        *x = ++N;
}

extern "C" void __sanitizer_cov_trace_pc_guard_init(
        uint32_t *start,
        uint32_t *stop) {
    // Avoid duplicate initialization
    if (start == stop || *start)
        return;

    if (__edges_start != nullptr || __edges_stop != nullptr) {
        fprintf(
                stderr,
                "Coverage instrumentation is only supported for a single module\n");
        _exit(-1);
    }

    __edges_start = start;
    __edges_stop = stop;

    // Map the shared memory region
    const char *shm_key = getenv("SHM_ID");
    if (!shm_key) {
        puts("[COV] no shared memory bitmap available, skipping");
        shmem = (struct SharedMemData *)malloc(SHM_SIZE);
    } else {
        int fd = shm_open(shm_key, O_RDWR, S_IREAD | S_IWRITE);
        if (fd <= -1) {
            fprintf(
                    stderr, "Failed to open shared memory region: %d\n", errno);
            _exit(-1);
        }

        shmem = (struct SharedMemData *)mmap(
                nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (shmem == MAP_FAILED) {
            fprintf(stderr, "Failed to mmap shared memory region\n");
            _exit(-1);
        }
    }

    __sanitizer_cov_reset_edgeguards();

    shmem->numEdges = stop - start;
    printf(
            "[COV] edge counters initialized. Shared memory: %s with %u edges\n",
            shm_key,
            shmem->numEdges);
}

extern "C" void __sanitizer_cov_trace_pc_guard(uint32_t *guard) {
    // There's a small race condition here: if this function executes in two
    // threads for the same edge at the same time, the first thread might disable
    // the edge (by setting the guard to zero) before the second thread fetches
    // the guard value (and thus the index). However, our instrumentation ignores
    // the first edge (see libcoverage.c) and so the race is unproblematic.
    uint32_t index = *guard;
    // If this function is called before coverage instrumentation is properly
    // initialized we want to return early.
    if (!index)
        return;
    shmem->edges[index / 8] |= 1 << (index % 8);
    *guard = 0;
}

//
// END FUZZING CODE
//