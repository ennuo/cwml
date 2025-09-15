#pragma once

namespace NVirtualMachine
{
    enum EInstructionType
    {
        IT_NOP,
        IT_LC_B,
        IT_LC_C,
        IT_LC_I,
        IT_LC_F,
        IT_LC_SW,
        IT_LC_NULL_SP,
        IT_MOV_B,
        IT_LOG_NEG_B,
        IT_MOV_C,
        IT_MOV_I,
        IT_INC_I,
        IT_DEC_I,
        IT_NEG_I,
        IT_BIT_NEG_I,
        IT_LOG_NEG_I,
        IT_ABS_I,
        IT_MOV_F,
        IT_NEG_F,
        IT_ABS_F,
        IT_SQRT_F,
        IT_SIN_F,
        IT_COS_F,
        IT_TAN_F,
        IT_MOV_V4,
        IT_NEG_V4,
        IT_MOV_M44,
        IT_MOV_S_DEPRECATED,
        IT_MOV_RP,
        IT_MOV_CP,
        IT_MOV_SP,
        IT_MOV_O,
        IT_EQ_B,
        IT_NE_B,
        IT_RESERVED0_C,
        IT_RESERVED1_C,
        IT_LT_C,
        IT_LTE_C,
        IT_GT_C,
        IT_GTE_C,
        IT_EQ_C,
        IT_NE_C,
        IT_ADD_I,
        IT_SUB_I,
        IT_MUL_I,
        IT_DIV_I,
        IT_MOD_I,
        IT_MIN_I,
        IT_MAX_I,
        IT_SLA_I,
        IT_SRA_I,
        IT_SRL_I,
        IT_BIT_OR_I,
        IT_BIT_AND_I,
        IT_BIT_XOR_I,
        IT_LT_I,
        IT_LTE_I,
        IT_GT_I,
        IT_GTE_I,
        IT_EQ_I,
        IT_NE_I,
        IT_ADD_F,
        IT_SUB_F,
        IT_MUL_F,
        IT_DIV_F,
        IT_MIN_F,
        IT_MAX_F,
        IT_LT_F,
        IT_LTE_F,
        IT_GT_F,
        IT_GTE_F,
        IT_EQ_F,
        IT_NE_F,
        IT_ADD_V4,
        IT_SUB_V4,
        IT_MULS_V4,
        IT_DIVS_V4,
        IT_DOT4_V4,
        IT_DOT3_V4,
        IT_DOT2_V4,
        IT_CROSS3_V4,
        IT_MUL_M44,
        IT_EQ_S_DEPRECATED,
        IT_NE_S_DEPRECATED,
        IT_EQ_RP,
        IT_NE_RP,
        IT_EQ_O,
        IT_NE_O,
        IT_EQ_SP,
        IT_NE_SP,
        IT_GET_V4_X,
        IT_GET_V4_Y,
        IT_GET_V4_Z,
        IT_GET_V4_W,
        IT_GET_V4_LEN2,
        IT_GET_V4_LEN3,
        IT_GET_V4_LEN4,
        IT_GET_M44_XX,
        IT_GET_M44_XY,
        IT_GET_M44_XZ,
        IT_GET_M44_XW,
        IT_GET_M44_YX,
        IT_GET_M44_YY,
        IT_GET_M44_YZ,
        IT_GET_M44_YW,
        IT_GET_M44_ZX,
        IT_GET_M44_ZY,
        IT_GET_M44_ZZ,
        IT_GET_M44_ZW,
        IT_GET_M44_WX,
        IT_GET_M44_WY,
        IT_GET_M44_WZ,
        IT_GET_M44_WW,
        IT_GET_M44_rX,
        IT_GET_M44_rY,
        IT_GET_M44_rZ,
        IT_GET_M44_rW,
        IT_GET_M44_cX,
        IT_GET_M44_cY,
        IT_GET_M44_cZ,
        IT_GET_M44_cW,
        IT_SET_V4_X,
        IT_SET_V4_Y,
        IT_SET_V4_Z,
        IT_SET_V4_W,
        IT_SET_M44_XX,
        IT_SET_M44_XY,
        IT_SET_M44_XZ,
        IT_SET_M44_XW,
        IT_SET_M44_YX,
        IT_SET_M44_YY,
        IT_SET_M44_YZ,
        IT_SET_M44_YW,
        IT_SET_M44_ZX,
        IT_SET_M44_ZY,
        IT_SET_M44_ZZ,
        IT_SET_M44_ZW,
        IT_SET_M44_WX,
        IT_SET_M44_WY,
        IT_SET_M44_WZ,
        IT_SET_M44_WW,
        IT_SET_M44_rX,
        IT_SET_M44_rY,
        IT_SET_M44_rZ,
        IT_SET_M44_rW,
        IT_SET_M44_cX,
        IT_SET_M44_cY,
        IT_SET_M44_cZ,
        IT_SET_M44_cW,
        IT_GET_SP_MEMBER,
        IT_GET_RP_MEMBER,
        IT_SET_SP_MEMBER,
        IT_SET_RP_MEMBER,
        IT_GET_ELEMENT,
        IT_SET_ELEMENT,
        IT_GET_ARRAY_LEN,
        IT_NEW_ARRAY,
        IT_ARRAY_INSERT,
        IT_ARRAY_APPEND,
        IT_ARRAY_ERASE,
        IT_ARRAY_FIND,
        IT_ARRAY_CLEAR,
        IT_WRITE,
        IT_ARG,
        IT_CALL,
        IT_RET,
        IT_B,
        IT_BEZ,
        IT_BNEZ,
        IT_CAST_SP,
        IT_INT_B,
        IT_INT_C,
        IT_INT_F,
        IT_FLOAT_B,
        IT_FLOAT_C,
        IT_FLOAT_I,
        IT_BOOL_C,
        IT_BOOL_I,
        IT_BOOL_F,
        IT_GET_OBJ_MEMBER,
        IT_SET_OBJ_MEMBER,
        IT_NEW_OBJECT,
        IT_ARRAY_RESIZE,
        IT_ARRAY_RESERVE,
        IT_LC_V4,
        IT_LC_NULL_O,
        IT_CAST_O,
        IT_GET_SP_NATIVE_MEMBER,
        IT_LC_SA,
        IT_BIT_OR_B,
        IT_BIT_AND_B,
        IT_BIT_XOR_B,
        IT_CALLV_O,
        IT_CALLV_SP,
        IT_ASSERT,
#ifndef LBP1
        IT_LC_S64,
        IT_MOV_S64,
        IT_ADD_S64,
        IT_EQ_S64,
        IT_NE_S64,
        IT_BIT_OR_S64,
        IT_BIT_AND_S64,
        IT_BIT_XOR_S64,
#endif

#ifdef LEGACY_ALEAR_VM_EXTENSIONS
        IT_EXT_ADDRESS,
        IT_EXT_LOAD,
        IT_EXT_STORE,
        IT_EXT_INVOKE_CONSTANT,
        IT_EXT_INVOKE_VARIABLE,
        IT_EXT_GET_STATIC_MEMBER,
        IT_EXT_SET_STATIC_MEMBER,
        IT_EXT_GET_SCRIPT_OBJECT,
        IT_EXT_GET_NATIVE_ARRAY,
#endif

        NUM_INSTRUCTION_TYPES
    };

    enum EInstructionClass
    {
        IC_NOP,
        IC_LOAD_CONST,
        IC_CAST,
        IC_UNARY,
        IC_BINARY,
        IC_GET_BUILTIN_MEMBER,
        IC_SET_BUILTIN_MEMBER,
        IC_GET_MEMBER,
        IC_SET_MEMBER,
        IC_GET_ELEMENT,
        IC_SET_ELEMENT,
        IC_NEW_OBJECT,
        IC_NEW_ARRAY,
        IC_WRITE,
        IC_ARG,
        IC_CALL,
        IC_RETURN,
        IC_BRANCH
    };

    struct LoadConstInstructionBool {
        unsigned int Bool : 1;
        unsigned int : 0;
        unsigned int DstIdx : 16;
        unsigned int : 8;
        unsigned int Op : 8;
    };

    struct SetMemberInstruction { /* VMInstruction.h:329 */
        u32 FieldRef:16;
        u32 BaseIdx:16;
        u32 SrcIdx:16;
        u32 Type:8;
        u32 Op:8;
    };

    struct BranchInstruction { /* VMInstruction.h:343 */
        s32 BranchOffset;
        u32 SrcIdx:16;
        u8 field2_0x6;
        u32 Op:8;
    };

    struct ArgInstruction { /* VMInstruction.h:338 */
        u8 field0_0x0;
        u8 field1_0x1;
        u32 ArgIdx:16;
        u32 SrcIdx:16;
        u32 Type:8;
        u32 Op:8;
    };

    struct NewArrayInstruction { /* VMInstruction.h:335 */
        u32 TypeIdx:16;
        u32 SizeIdx:16;
        u32 DstIdx:16;
        u8 field3_0x6;
        u32 Op:8;
    };

    struct LoadConstInstruction { /* VMInstruction.h:313 */
        u32 ConstantIdx;
        u32 DstIdx:16;
        u8 field2_0x6;
        u32 Op:8;
    };

    struct NewObjectInstruction { /* VMInstruction.h:334 */
        u32 TypeIdx:16;
        u8 field1_0x2;
        u8 field2_0x3;
        u32 DstIdx:16;
        u8 field4_0x6;
        u32 Op:8;
    };

    struct CallInstruction { /* VMInstruction.h:340 */
        u8 field0_0x0;
        u8 field1_0x1;
        u32 CallIdx:16;
        u32 DstIdx:16;
        u32 Type:8;
        u32 Op:8;
    };

    struct LoadConstInstructionNullSafePtr { /* VMInstruction.h:319 */
        u8 field0_0x0;
        u8 field1_0x1;
        u8 field2_0x2;
        u8 field3_0x3;
        u32 DstIdx:16;
        u8 field5_0x6;
        u32 Op:8;
    };

    struct SetElementInstruction { /* VMInstruction.h:332 */
        u32 IndexIdx:16;
        u32 BaseIdx:16;
        u32 SrcIdx:16;
        u32 Type:8;
        u32 Op:8;
    };

    struct LoadConstInstructionChar { /* VMInstruction.h:315 */
        u16 Char;
        s16 Pad;
        u32 DstIdx:16;
        u8 field3_0x6;
        u32 Op:8;
    };

    struct LoadConstInstructionFloat { /* VMInstruction.h:317 */
        float Float;
        u32 DstIdx:16;
        u8 field2_0x6;
        u32 Op:8;
    };

    struct ReturnInstruction { /* VMInstruction.h:341 */
        u8 field0_0x0;
        u8 field1_0x1;
        u32 SrcIdx:16;
        u8 field3_0x4;
        u8 field4_0x5;
        u32 Type:8;
        u32 Op:8;
    };

    struct UnaryInstruction { /* VMInstruction.h:322 */
        u8 field0_0x0;
        u8 field1_0x1;
        u32 SrcIdx:16;
        u32 DstIdx:16;
        u8 field4_0x6;
        u32 Op:8;
    };

    struct LoadConstInstructionString { /* VMInstruction.h:318 */
        u32 StringIdx;
        u32 DstIdx:16;
        u8 field2_0x6;
        u32 Op:8;
    };

    struct SetBuiltInMemberInstruction { /* VMInstruction.h:326 */
        u8 field0_0x0;
        u8 field1_0x1;
        u32 BaseIdx:16;
        u32 SrcIdx:16;
        u8 field4_0x6;
        u32 Op:8;
    };

    struct NopInstruction { /* VMInstruction.h:310 */
        u8 field0_0x0;
        u8 field1_0x1;
        u8 field2_0x2;
        u8 field3_0x3;
        u8 field4_0x4;
        u8 field5_0x5;
        u8 field6_0x6;
        u32 Op:8;
    };

    struct CastInstruction { /* VMInstruction.h:321 */
        u32 TypeIdx:16;
        u32 SrcIdx:16;
        u32 DstIdx:16;
        u8 field3_0x6;
        u32 Op:8;
    };

    struct GetElementInstruction { /* VMInstruction.h:331 */
        u32 SrcOrIndexIdx:16;
        u32 BaseIdx:16;
        u32 DstIdx:16;
        u32 Type:8;
        u32 Op:8;
    };

    struct GetBuiltInMemberInstruction { /* VMInstruction.h:325 */
        u8 field0_0x0;
        u8 field1_0x1;
        u32 BaseIdx:16;
        u32 DstIdx:16;
        u8 field4_0x6;
        u32 Op:8;
    };

    struct LoadConstInstructionInt { /* VMInstruction.h:316 */
        s32 Int;
        u32 DstIdx:16;
        u8 field2_0x6;
        u32 Op:8;
    };

    struct BinaryInstruction { /* VMInstruction.h:323 */
        u32 SrcBIdx:16;
        u32 SrcAIdx:16;
        u32 DstIdx:16;
        u8 field3_0x6;
        u32 Op:8;
    };

    struct GetMemberInstruction { /* VMInstruction.h:328 */
        u32 FieldRef:16;
        u32 BaseIdx:16;
        u32 DstIdx:16;
        u32 Type:8;
        u32 Op:8;
    };

    struct WriteInstruction { /* VMInstruction.h:337 */
        u8 field0_0x0;
        u8 field1_0x1;
        u32 SrcIdx:16;
        u8 field3_0x4;
        u8 field4_0x5;
        u32 Type:8;
        u32 Op:8;
    };

    union Instruction {
        u64 Bits;
        NopInstruction Nop;
        LoadConstInstruction LoadConst;
        LoadConstInstructionBool LoadConstBool;
        LoadConstInstructionChar LoadConstChar;
        LoadConstInstructionInt LoadConstInt;
        LoadConstInstructionFloat LoadConstFloat;
        LoadConstInstructionString LoadConstString;
        LoadConstInstructionNullSafePtr LoadConstNullSafePtr;
        CastInstruction Cast;
        UnaryInstruction Unary;
        BinaryInstruction Binary;
        GetBuiltInMemberInstruction GetBuiltInMember;
        SetBuiltInMemberInstruction SetBuiltInMember;
        GetMemberInstruction GetMember;
        SetMemberInstruction SetMember;
        GetElementInstruction GetElement;
        SetElementInstruction SetElement;
        NewObjectInstruction NewObject;
        NewArrayInstruction NewArray;
        WriteInstruction Write;
        ArgInstruction Arg;
        CallInstruction Call;
        ReturnInstruction Return;
        BranchInstruction Branch;
    };
}
