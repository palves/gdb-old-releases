/* FIXME: This decl may not be necessary anymore.  /dje 931025 */
int sim_clear_breakpoints PARAMS ((void));

#define SIM_SINGLE_STEP 1
#define SIM_DONE 2
#define SIM_BREAKPOINT 3
#define SIM_INTERRUPT 4
#define SIM_BAD_INST 5
#define SIM_DIV_ZERO 6
#define SIM_BAD_SYSCALL 7
#define SIM_BAD_ALIGN 8
#define SIM_BAD_ADDR 9
