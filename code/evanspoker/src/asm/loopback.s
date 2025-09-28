# GCC doesn't really support naked functions,
# so I'm just going to poke a branch to this function in assembly.

.set LOOPBACK_ADDRESS, 0x7f000001
.set SCE_NP_PORT, 0xe4a

# r11 has the destination port number
# r9 has the destination address
.global LoopbackPatchHook
LoopbackPatchHook:
.set RETURN_ADDRESS, 0x000f487c
    lis %r3, LOOPBACK_ADDRESS@h
    ori %r3, %r3, LOOPBACK_ADDRESS@l
    mr %r4, %r9

    # Compare the addresses
    xor %r4, %r3, %r4
    srawi %r0, %r4, 0x1f
    xor %r3, %r0, %r4
    subf %r3, %r0, %r3
    subi %r3, %r3, 0x1
    rlwinm %r3, %r3, 0x1, 0x1f, 0x1f
    extsw %r3, %r3

    cmpwi %cr7, %r3, 0
    beq %cr7, Exit

    li %r3, SCE_NP_PORT
    sth %r3, 0x80(%r1)
    
    Exit:
        bla 0x005aba30
        ba RETURN_ADDRESS