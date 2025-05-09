#include <gdt.h>
#include <mp.h>
#include <printf.h>
#include <sched.h>
#include <smap.h>
#include <spin_lock.h>
#include <vmalloc.h>

void mp_work_start(void(task)(void)) {
    int i = 1;
    while (core_data[i] != NULL) {
        if (core_data[i]->state == IDLE && core_data[i]->current_task == NULL) {
            core_data[i]->current_task = create_task(task);
            break;
        }
        i++;
    }
}

void wakeup() {
    spin_lock(&wakeup_lock);
    enable_smap_smep_umip();
    gdt_install();
    serial_init();
    while (!cr3_ready)
        ;
    current_cpu++;
    asm volatile("mov %0, %%cr3" ::"r"(cr3));
    int cpu = current_cpu;
    struct core *current_core = vmm_alloc_pages(1);
    current_core->id = cpu;
    current_core->state = IDLE;
    current_core->current_task = NULL;
    core_data[cpu] = current_core;
    spin_unlock(&wakeup_lock);

    while (1) {
        spin_lock(&wakeup_lock);
        if (current_core->current_task != NULL) {
            current_core->state = BUSY;
            spin_unlock(&wakeup_lock);
            current_core->current_task->entry();
            delete_task(current_core->current_task);
            current_core->current_task = NULL;
            current_core->state = IDLE;
        }
        spin_unlock(&wakeup_lock);
    }
}
