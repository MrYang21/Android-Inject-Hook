#include <jni.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "Dobby/include/dobby.h"

uintptr_t get_py(const char* name) {
    FILE* file = fopen("/proc/self/maps", "r");
    if (!file) {
        return 0;
    }
    char line[1024];
    uintptr_t start = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, name) && strstr(line, ".so")) {
            sscanf(line, "%lx-%*lx", &start);
            break;
        }
    }
    fclose(file);
    return start;
}

bool hook(uintptr_t py, uintptr_t offset, void* replace, void** backup) {
    void* target = (void*)(py + offset);
    uintptr_t page_start = (uintptr_t)target & ~(uintptr_t)(4095);

    int page_size = sysconf(_SC_PAGESIZE);
    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_WRITE) != 0) {
        return false;
    }

    DobbyHook(target, replace, backup);

    if (mprotect((void*)page_start, page_size, PROT_READ | PROT_EXEC) != 0) {
    }
    return true;
}
long new_UE4, UE4, safe;
long UE4_size = 0xD609348;

typedef void* (*osub_memcpy)(void* dest, long src, int n);
osub_memcpy original_func = nullptr;
void* sub_memcpy(void* dest, long src, int n)
{
    if (src >= UE4 && src < (UE4 + UE4_size))
    {
        src = new_UE4 + (src - UE4);
    }

    return original_func(dest, src, n);
}
void anogs_hook() {
    uintptr_t py = get_py("libanogs.so");
    if (!py) {
        return;
    }
    hook(py, 0x5C3138, (void*)sub_memcpy, (void**)&original_func);
    hook(py, 0x5C3230, (void*)sub_memcpy, (void**)&original_func);
}





void ue4_hook() {
    uintptr_t py = get_py("libUE4.so");
    if (!py) {
        return;
    }
    hook(py, 0xD609348, (void*)sub_memcpy, (void**)&original_func);
}

void __attribute__((constructor)) hook_entry() {
    anogs_hook();
    ue4_hook();
}
