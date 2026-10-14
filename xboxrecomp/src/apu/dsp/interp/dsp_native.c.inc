/* GPL-2.0-or-later, matching dsp_cpu.c.
 * Fixed native fragments of FIFA Street 1 GP/EP bootstrap and captured hot code.
 * This is program code, not an opcode decoder. Recheck the full guarded block on each
 * instruction entry: DSP DMA and guest overlays can replace P-memory at any time.
 */
bool dsp56k_native_pc_covered(const dsp_core_t *dsp)
{
    if (dsp->is_gp) return dsp->pc == 0x160 || dsp->pc == 0x162 || dsp->pc == 0x164 ||
                          (dsp->pc >= 0x33F && dsp->pc <= 0x342) ||
                          (dsp->pc >= 0x189 && dsp->pc <= 0x193 && dsp->pc != 0x18D) ||
                          (dsp->pc >= 0x25F && dsp->pc <= 0x261);
    return (dsp->pc >= 0x369 && dsp->pc <= 0x373) || dsp->pc == 0x30 || dsp->pc == 0x31 || dsp->pc == 0x32 ||
           dsp->pc == 0x34 || dsp->pc == 0x36 || dsp->pc == 0x38;
}

/* Fixed arithmetic/data-path helpers: no instruction or operand decoding. */
typedef enum { NATIVE_MOVE, NATIVE_MPY, NATIVE_MAC, NATIVE_MACR } NativeProduct;
static void native_product(dsp_core_t *dsp, unsigned acc, NativeProduct mode)
{
    uint32_t product[3], value[3];uint16_t flags=0;
    dsp_mul56(dsp->registers[DSP_REG_Y0],dsp->registers[DSP_REG_X0],product,SIGN_PLUS);
    if (mode == NATIVE_MPY) { value[0]=product[0];value[1]=product[1];value[2]=product[2]; }
    else {
        value[0]=dsp->registers[DSP_REG_A2+acc];value[1]=dsp->registers[DSP_REG_A1+acc];value[2]=dsp->registers[DSP_REG_A0+acc];
        flags=dsp_add56(product,value);
        if (mode == NATIVE_MACR) dsp_rnd56(dsp,value);
    }
    dsp->registers[DSP_REG_A2+acc]=value[0];dsp->registers[DSP_REG_A1+acc]=value[1];dsp->registers[DSP_REG_A0+acc]=value[2];
    emu_ccr_update_e_u_n_z(dsp,value[0],value[1],value[2]);
    dsp->registers[DSP_REG_SR] &= BITMASK(16)-(1<<DSP_SR_V);
    if (mode != NATIVE_MPY) dsp->registers[DSP_REG_SR] |= flags & 0xFE;
}
static void native_ep_filter(dsp_core_t *dsp, bool paired, unsigned yreg, unsigned acc, NativeProduct mode)
{
    uint32_t xaddr=0,x=0,yaddr,y;
    if (paired) { xaddr=dsp->registers[DSP_REG_R0];emu_update_rn(dsp,0,dsp->registers[DSP_REG_N0]); }
    yaddr=dsp->registers[DSP_REG_R0+yreg];emu_update_rn(dsp,yreg,1);
    if (paired) x=dsp56k_read_memory(dsp,DSP_SPACE_X,xaddr);
    y=dsp56k_read_memory(dsp,DSP_SPACE_Y,yaddr);
    /* Parallel reads happen before ALU; source registers still hold old values.
     * Commit both moves afterwards, exactly in X then Y order. */
    if (mode) native_product(dsp,acc,mode);
    if (paired) dsp->registers[DSP_REG_X0]=x;
    dsp->registers[DSP_REG_Y0]=paired?y:(y & 0xFFFFFF);
}

bool dsp56k_execute_native_instruction(dsp_core_t *dsp)
{
    if (TRACE_DSP_DISASM || trace_event_get_state(TRACE_DSP56K_EXECUTE_INSTRUCTION_DISASM)) return false;
    if (!dsp56k_native_pc_covered(dsp)) return false;
    if (dsp->is_gp) {
        if (dsp->pram[0] != 0x0BF080 || dsp->pram[1] != 0x000155) return false;
        if (dsp->pc >= 0x33F) {
            static const uint32_t hot[] = {0x44D800,0x56E100,0x200040,0x565900};
            if (memcmp(dsp->pram+0x33F,hot,sizeof(hot))) return false;
        } else if (dsp->pc >= 0x25F) {
            static const uint32_t hot[] = {0x200040,0x565900,0x200013};
            if (memcmp(dsp->pram+0x25F,hot,sizeof(hot))) return false;
        } else if (dsp->pc >= 0x189) {
            static const uint32_t hot[] = {0x44D800,0x2000D0,0x021595,0x0CC581,
                0x000004,0x44E400,0x200040,0x565C00,0,0,0x00000C};
            if (memcmp(dsp->pram+0x189,hot,sizeof(hot))) return false;
        } else if (dsp->pram[0x160] != 0x0A8581 || dsp->pram[0x161] != 0x000160 ||
                   dsp->pram[0x162] != 0x08F485 || dsp->pram[0x163] != 0x000002 ||
                   dsp->pram[0x164] != 0x00000C) return false;
    } else {
        if (dsp->pram[0] != 0x050C08) return false;
        if (dsp->pc >= 0x369) {
            static const uint32_t hot[] = {0xF08800,0x4EDCD0,0xF088D8,0x4EDCD2,
                0xF088DA,0x4EDCD2,0xF088DA,0x4EDCD2,0xF088DA,0x4EDCD3,0x4EDDDB};
            static const uint32_t header[] = {0xD01B40,0xC9,0x060172,0x87C4B0};
            if (memcmp(dsp->pram+0x369,hot,sizeof(hot)) || memcmp(dsp->pram+0x300,header,sizeof(header))) return false;
        } else {
            static const uint32_t words[] = { 0x200013, 0x014180, 0x0A8581, 0x000031,
                0x44F000, 0xFFFFB3, 0x077084, 0x000005, 0x08F485, 0x000002 };
            /* P:1..7 are mutable counters/data, not stable program identity. */
            if (memcmp(dsp->pram + 0x30, words, sizeof(words))) return false;
        }
    }

    trace_dsp56k_execute_instruction(dsp->is_gp, dsp->pc);
    dsp->disasm_memory_ptr = 0;
    dsp->cur_inst = dsp->pram[dsp->pc];
    dsp->cur_inst_len = 1;
    dsp->instr_cycle = 2;
    switch (dsp->pc) {
    case 0x18A: native_product(dsp,0,NATIVE_MPY);break;
    case 0x18B: /* move x:(r5+4),x1; 24-bit address, R5 unchanged */
        dsp->registers[DSP_REG_X1]=dsp56k_read_memory(dsp,DSP_SPACE_X,(dsp->registers[DSP_REG_R5]+4)&0xFFFFFF)&0xFFFFFF;
        break;
    case 0x18C: /* brclr #1,x1,p:$190; guarded fixed displacement +4 */
        dsp->cur_inst_len=2;dsp->instr_cycle=6;
        if (!(dsp->registers[DSP_REG_X1]&2)) {dsp->pc=0x190;dsp->cur_inst_len=0;}
        break;
    case 0x18E: /* move x:(r4),x0 */
        dsp->registers[DSP_REG_X0]=dsp56k_read_memory(dsp,DSP_SPACE_X,dsp->registers[DSP_REG_R4])&0xFFFFFF;
        break;
    case 0x190: { /* move a,x:(r4)+ */
        uint32_t addr=dsp->registers[DSP_REG_R4],v;emu_update_rn(dsp,4,1);
        emu_pm_read_accu24(dsp,DSP_REG_A,&v);dsp56k_write_memory(dsp,DSP_SPACE_X,addr,v);
        break;
    }
    case 0x191: case 0x192: /* NOP */break;
    case 0x189:
    case 0x33F: { /* move x:(r0)+,x0 */
        uint32_t addr=dsp->registers[DSP_REG_R0];emu_update_rn(dsp,0,1);
        dsp->registers[DSP_REG_X0]=dsp56k_read_memory(dsp,DSP_SPACE_X,addr)&0xFFFFFF;
        break;
    }
    case 0x340: { /* move x:(r1),a, no CCR update */
        uint32_t v=dsp56k_read_memory(dsp,DSP_SPACE_X,dsp->registers[DSP_REG_R1]);
        dsp->registers[DSP_REG_A0]=0;dsp->registers[DSP_REG_A1]=v;dsp->registers[DSP_REG_A2]=v&(1<<23)?0xFF:0;
        break;
    }
    case 0x18F:
    case 0x25F:
    case 0x341: { /* add x0,a */
        uint32_t source[3]={dsp->registers[DSP_REG_X0]&(1<<23)?0xFF:0,dsp->registers[DSP_REG_X0],0};
        uint32_t dest[3]={dsp->registers[DSP_REG_A2],dsp->registers[DSP_REG_A1],dsp->registers[DSP_REG_A0]};
        uint16_t sr=dsp_add56(source,dest);
        dsp->registers[DSP_REG_A2]=dest[0];dsp->registers[DSP_REG_A1]=dest[1];dsp->registers[DSP_REG_A0]=dest[2];
        emu_ccr_update_e_u_n_z(dsp,dest[0],dest[1],dest[2]);
        dsp->registers[DSP_REG_SR] &= BITMASK(16)-((1<<DSP_SR_V)|(1<<DSP_SR_C));
        dsp->registers[DSP_REG_SR] |= sr;
        break;
    }
    case 0x260:
    case 0x342: { /* move a,x:(r1)+, with 24-bit scaling/limiting */
        uint32_t addr=dsp->registers[DSP_REG_R1],v;emu_update_rn(dsp,1,1);
        emu_pm_read_accu24(dsp,DSP_REG_A,&v);dsp56k_write_memory(dsp,DSP_SPACE_X,addr,v);
        break;
    }
    case 0x369: native_ep_filter(dsp,true,4,0,NATIVE_MOVE);break;
    case 0x36A: native_ep_filter(dsp,false,4,0,NATIVE_MPY);break;
    case 0x36B: native_ep_filter(dsp,true,4,1,NATIVE_MPY);break;
    case 0x36C: native_ep_filter(dsp,false,4,0,NATIVE_MAC);break;
    case 0x36D: native_ep_filter(dsp,true,4,1,NATIVE_MAC);break;
    case 0x36E: native_ep_filter(dsp,false,4,0,NATIVE_MAC);break;
    case 0x36F: native_ep_filter(dsp,true,4,1,NATIVE_MAC);break;
    case 0x370: native_ep_filter(dsp,false,4,0,NATIVE_MAC);break;
    case 0x371: native_ep_filter(dsp,true,4,1,NATIVE_MAC);break;
    case 0x372: native_ep_filter(dsp,false,4,0,NATIVE_MACR);break;
    case 0x373: native_ep_filter(dsp,false,5,1,NATIVE_MACR);break;
    case 0x261:
    case 0x30: /* clr a */
        dsp->registers[DSP_REG_A2] = dsp->registers[DSP_REG_A1] = dsp->registers[DSP_REG_A0] = 0;
        dsp->registers[DSP_REG_SR] &= BITMASK(16)-((1<<DSP_SR_E)|(1<<DSP_SR_N)|(1<<DSP_SR_V));
        dsp->registers[DSP_REG_SR] |= (1<<DSP_SR_U)|(1<<DSP_SR_Z);
        break;
    case 0x31: { /* add #1,a: full 56-bit accumulator, all scaling CCR modes */
        uint32_t source[3] = {0, 1, 0};
        uint32_t dest[3] = {dsp->registers[DSP_REG_A2], dsp->registers[DSP_REG_A1], dsp->registers[DSP_REG_A0]};
        uint16_t sr = dsp_add56(source, dest);
        dsp->registers[DSP_REG_A2] = dest[0];dsp->registers[DSP_REG_A1] = dest[1];dsp->registers[DSP_REG_A0] = dest[2];
        /* Pure shared flag arithmetic, no opcode decode or instruction dispatch. */
        emu_ccr_update_e_u_n_z(dsp, dest[0], dest[1], dest[2]);
        dsp->registers[DSP_REG_SR] &= BITMASK(16)-((1<<DSP_SR_V)|(1<<DSP_SR_C));
        dsp->registers[DSP_REG_SR] |= sr;
        break;
    }
    case 0x32: { /* jclr #1,x:$ffffc5,p:$31 */
        uint32_t value = dsp56k_read_memory(dsp, DSP_SPACE_X, 0xFFFFC5);
        uint32_t target = read_memory_p(dsp, 0x33);
        dsp->instr_cycle = 6;
        if (!(value & 2)) { dsp->pc = target; dsp->cur_inst_len = 0; }
        else dsp->cur_inst_len = 2;
        break;
    }
    case 0x34: /* move x:$ffffb3,x0 (parallel MOVE has no ALU side effect) */
        dsp->instr_cycle = 4;dsp->cur_inst_len = 2;
        dsp->registers[DSP_REG_X0] = dsp56k_read_memory(dsp, DSP_SPACE_X, 0xFFFFB3) & 0xFFFFFF;
        break;
    case 0x36: /* movem x0,p:$5 */
        dsp->instr_cycle = 4;dsp->cur_inst_len = 2;
        dsp56k_write_memory(dsp, DSP_SPACE_P, 5, dsp->registers[DSP_REG_X0]);
        dsp->instr_cycle = 8;
        break;
    case 0x38: /* movep #2,x:$ffffc5 */
        dsp->cur_inst_len = 2;dsp->instr_cycle = 4;
        dsp56k_write_memory(dsp, DSP_SPACE_X, 0xFFFFC5, 2);
        dsp->instr_cycle = 6;
        break;
    case 0x160: { /* jclr #1,x:$ffffc5,p:$160 */
        uint32_t value = dsp56k_read_memory(dsp, DSP_SPACE_X, 0xFFFFC5);
        /* Read AFTER I/O, as the original does. A callback may change P:161. */
        uint32_t target = read_memory_p(dsp, 0x161);
        dsp->instr_cycle = 6;
        if (!(value & 2)) { dsp->pc = target; dsp->cur_inst_len = 0; }
        else dsp->cur_inst_len = 2;
        break;
    }
    case 0x162: /* movep #2,x:$ffffc5 */
        dsp->cur_inst_len = 2;
        /* Immediate EA adds two cycles before the peripheral callback. */
        dsp->instr_cycle = 4;
        dsp56k_write_memory(dsp, DSP_SPACE_X, 0xFFFFC5, 2);
        dsp->instr_cycle = 6;
        break;
    case 0x193:
    case 0x164: { /* rts (SR remains unchanged) */
        uint32_t pc = 0, sr;
        dsp_stack_pop(dsp, &pc, &sr);
        dsp->pc = pc;
        dsp->cur_inst_len = 0;
        dsp->instr_cycle = 4;
        break;
    }
    }
    dsp_complete_instruction(dsp);
    return true;
}
