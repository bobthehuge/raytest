#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// https://en.wikipedia.org/wiki/D-ary_heap
// https://en.wikipedia.org/wiki/Binary_heap

#define HEAP_NOEXPAND   0x01
#define HEAP_LOCK       0x02
#define HEAP_CANFAIL    0x04

#define HEAP_MAX        0x10
#define HEAP_MIN        0x20

#define FLAG(hf, f) (((hf) & (f)) == (f))
#define SETFLAG(hf, f) ((hf) |= (f))

typedef struct
{
    size_t value;
    void *obj;
} HeapElement;

typedef struct
{
    const size_t d;
    size_t cap;
    size_t len;
    int flags;
    HeapElement *elts;
} HeapArray;

// resize is never called automatically
int heap_resize(HeapArray *heap, size_t n)
{
    if (FLAG(heap->flags, HEAP_NOEXPAND | HEAP_CANFAIL))
        return -HEAP_NOEXPAND;

    void *tmp = realloc(heap->elts, sizeof(HeapElement) * n);

    if (tmp == NULL && n != 0 && FLAG(heap->flags, HEAP_CANFAIL))
        return -1;

    heap->elts = tmp;
    heap->cap = n;

    return 0;
}

int heap_max_insert(HeapArray *heap, HeapElement elt)
{
    if (FLAG(heap->flags, HEAP_LOCK | HEAP_CANFAIL))
        return -HEAP_LOCK;

    if (!FLAG(heap->flags, HEAP_MAX | HEAP_CANFAIL))
        return -HEAP_MIN;

    if (heap->len >= heap->cap && FLAG(heap->flags, HEAP_CANFAIL))
        return -HEAP_NOEXPAND;

    if (heap->len == 0)
    {
        heap->len++;
        *heap->elts = elt;
        return 0;
    }

    size_t i1 = heap->len;
    size_t i2 = (heap->len - 1)/heap->d;

    heap->elts[i1] = elt;
    heap->len++;

    while (i1 > 0 && elt.value >= heap->elts[i2].value)
    {
        HeapElement t = heap->elts[i1];
        heap->elts[i1] = heap->elts[i2];
        heap->elts[i2] = t;
        i1 = i2;
        i2 = (i2 - 1)/heap->d;
    }

    return 0;
}

int heap_min_insert(HeapArray *heap, HeapElement elt)
{
    if (FLAG(heap->flags, HEAP_LOCK | HEAP_CANFAIL))
        return -HEAP_LOCK;

    if (!FLAG(heap->flags, HEAP_MIN | HEAP_CANFAIL))
        return -HEAP_MIN;

    if (heap->len >= heap->cap && FLAG(heap->flags, HEAP_CANFAIL))
        return -HEAP_NOEXPAND;

    if (heap->len == 0)
    {
        heap->len++;
        *heap->elts = elt;
        return 0;
    }

    size_t i1 = heap->len;
    size_t i2 = (heap->len - 1)/heap->d;

    heap->elts[i1] = elt;
    heap->len++;

    while (i1 > 0 && elt.value <= heap->elts[i2].value)
    {
        HeapElement t = heap->elts[i1];
        heap->elts[i1] = heap->elts[i2];
        heap->elts[i2] = t;
        i1 = i2;
        i2 = (i2 - 1)/heap->d;
    }

    return 0;
}

int is_max_heap(HeapArray heap)
{
    for (size_t i = 0; i < heap.len; i++)
        for (size_t j = 1; j <= heap.d && i*heap.d + j < heap.len; j++)
            if (heap.elts[i*heap.d+j].value >= heap.elts[i].value)
                return 0;
    return 1;
}

int is_min_heap(HeapArray heap)
{
    for (size_t i = 0; i < heap.len; i++)
        for (size_t j = 1; j <= heap.d && i*heap.d + j < heap.len; j++)
            if (heap.elts[i*heap.d+j].value <= heap.elts[i].value)
                return 0;
    return 1;
}

// https://stackoverflow.com/questions/33010010/how-to-generate-random-64-bit-unsigned-integer-in-c
#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)

size_t rand64(void) {
  size_t r = 0;
  for (int i = 0; i < 64; i += RAND_MAX_WIDTH) {
    r <<= RAND_MAX_WIDTH;
    r ^= (unsigned)rand();
  }
  return r;
}

#define GETTIME(s,ns) ((double)(s) + (double)(ns) / 1000000000)
struct bench {
    float elapsed;
    size_t total;
    int err;
};

struct bench guccy_insert(HeapArray *heap, size_t elts)
{
    struct bench b;
    struct timespec t1;
    struct timespec t2;

    size_t i = 0;
    
    clock_gettime(CLOCK_REALTIME, &t1);

    for (; i < elts; i++)
    {
        if (heap->len >= heap->cap)
        {
            if (heap_resize(heap, heap->cap + 8))
            {
                clock_gettime(CLOCK_REALTIME, &t2);
                free(heap->elts);
                t2.tv_nsec -= t1.tv_nsec;
                t2.tv_sec -= t1.tv_sec;
                b.elapsed = GETTIME(t2.tv_sec, t2.tv_nsec);
                b.total = i;
                b.err = -2;
                return b;
            }
        }

        size_t value = rand64();

        if (FLAG(heap->flags, HEAP_MAX))
        {
            if (heap_max_insert(heap, (HeapElement){.value=value}))
            {
                clock_gettime(CLOCK_REALTIME, &t2);
                free(heap->elts);
                t2.tv_nsec -= t1.tv_nsec;
                t2.tv_sec -= t1.tv_sec;
                b.elapsed = GETTIME(t2.tv_sec, t2.tv_nsec);
                b.total = i;
                b.err = -1;
                return b;
            }
        }
        else if (FLAG(heap->flags, HEAP_MIN))
        {
            if (heap_min_insert(heap, (HeapElement){.value=value}))
            {
                clock_gettime(CLOCK_REALTIME, &t2);
                free(heap->elts);
                t2.tv_nsec -= t1.tv_nsec;
                t2.tv_sec -= t1.tv_sec;
                b.elapsed = GETTIME(t2.tv_sec, t2.tv_nsec);
                b.err = -1;
                return b;
            }
        }
    }

    clock_gettime(CLOCK_REALTIME, &t2);
    free(heap->elts);
    t2.tv_nsec -= t1.tv_nsec;
    t2.tv_sec -= t1.tv_sec;
    b.elapsed = GETTIME(t2.tv_sec, t2.tv_nsec);
    b.total = i;
    b.err = 0;

    return b;
}

// not really accurate nor exhaustive but gives an order of idea
void guccybench(size_t d, size_t elts, size_t samples)
{
    HeapArray heap = {
        .d = d,
        .cap = 0,
        .len = 0,
        .flags = HEAP_MAX | HEAP_CANFAIL,
        .elts = malloc(0), // needed for realloc
    };

    double avg1 = 0;

    for (size_t i = 0; i < samples; i++)
    {
        struct bench b = guccy_insert(&heap, elts);

        if (b.err)
            printf("Failed at %zuth(max) of %zu with %d\n", b.total, i, b.err);

        heap.cap = 0;
        heap.len = 0;
        heap.elts = malloc(0);
        avg1 += b.elapsed;
    }

    double avg2 = 0;
    heap.flags = HEAP_MIN | HEAP_CANFAIL;

    for (size_t i = 0; i < samples; i++)
    {
        struct bench b = guccy_insert(&heap, elts);

        if (b.err)
            printf("Failed at %zuth(min) of %zu with %d\n", b.total, i, b.err);

        heap.cap = 0;
        heap.len = 0;
        heap.elts = malloc(0);
        avg2 += b.elapsed;
    }

    printf("insert_max AVG for %zu-heap: %fs\n", d, avg1 / samples);
    printf("insert_min AVG for %zu-heap: %fs\n", d, avg2 / samples);
}

int main(void)
{
    srand(time(NULL));
    const size_t elts = 100000000;
    const size_t samples = 10;

    guccybench(2, elts, samples);
    guccybench(3, elts, samples);
    guccybench(4, elts, samples);
    guccybench(5, elts, samples);
    guccybench(6, elts, samples);
    guccybench(7, elts, samples);
    guccybench(8, elts, samples);
    
    return 0;
}
