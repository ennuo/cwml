.global CSRPatchHook
CSRPatchHook:
.set CONTINUE_FAIL_ADDRESS, 0x0008e168
.set CONTINUE_OK_ADDRESS, 0x0008e064
.set AddCSRToQueue, 0x0008b0e8
    addi %r3, %r1, 0x70
    
    std %r2, 0x28(%r1)
    lis %r2, _Z20GetResourceFromCacheR2CPI23CCrapSerialisedResourceE@h
    ori %r2, %r2, _Z20GetResourceFromCacheR2CPI23CCrapSerialisedResourceE@l
    lwz %r2, 0x4(%r2)
    bl ._Z20GetResourceFromCacheR2CPI23CCrapSerialisedResourceE
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0
    beq %cr7, NoDataSource

    addi %r3, %r1, 0x70
    lwz %r4, 0x80(%r24) # CSRsDone
    extsw %r5, %r21

    bla AddCSRToQueue



    ba CONTINUE_OK_ADDRESS
NoDataSource:
    lwz %r3, 0x70(%r1) # <- Instruction we replaced, loads Ref from CP
    ba CONTINUE_FAIL_ADDRESS

