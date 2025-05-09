#include <stdint.h>
#include <string.h>
#include <task.h>
#include <vmalloc.h>


uint64_t globid = 1;

struct task *create_task(void (*entry_point)(void)) {
    struct task *new_task = (struct task *) vmm_alloc_pages(1);
    uint64_t stack_top = (uint64_t) vmm_alloc_pages(1) + 0x1000;

    memset(new_task, 0, sizeof(struct task)); // zero out the task

    new_task->state = READY;
    new_task->regs.rip = (uint64_t) entry_point;
    new_task->regs.cs = 0x08;
    new_task->regs.rflags = 0x202;
    new_task->regs.rsp = stack_top;
    new_task->regs.ss = 0x10;
    new_task->stack = (void *) stack_top - 0x1000;
    new_task->entry = entry_point;
    new_task->id = globid++;

    return new_task;
}

void delete_task(struct task *t) {
    vmm_free_pages(t->stack, 1);
    vmm_free_pages(t,1);
}