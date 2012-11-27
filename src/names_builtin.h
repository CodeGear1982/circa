// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#pragma once

namespace circa {

const int name_None = 0;
const int name_Invalid = 1;
const int name_File = 2;
const int name_Newline = 3;
const int name_Out = 4;
const int name_Unknown = 5;
const int name_Repeat = 6;
const int name_Success = 7;
const int name_Failure = 8;
const int name_FileNotFound = 9;
const int name_NotEnoughInputs = 10;
const int name_TooManyInputs = 11;
const int name_ExtraOutputNotFound = 12;
const int name_Default = 13;
const int name_ByDemand = 14;
const int name_Unevaluated = 15;
const int name_InProgress = 16;
const int name_Lazy = 17;
const int name_Consumed = 18;
const int name_Uncaptured = 19;
const int name_Return = 20;
const int name_Continue = 21;
const int name_Break = 22;
const int name_Discard = 23;
const int name_InfixOperator = 24;
const int name_FunctionName = 25;
const int name_TypeName = 26;
const int name_TermName = 27;
const int name_Keyword = 28;
const int name_Whitespace = 29;
const int name_UnknownIdentifier = 30;
const int name_LookupAny = 31;
const int name_LookupType = 32;
const int name_LookupFunction = 33;
const int name_LookupModule = 34;
const int name_Untyped = 35;
const int name_UniformListType = 36;
const int name_AnonStructType = 37;
const int name_StructType = 38;
const int name_NativeModule = 39;
const int name_PatchBlock = 40;
const int name_ParseTime = 41;
const int name_Bootstrapping = 42;
const int name_Done = 43;
const int name_StorageTypeNull = 44;
const int name_StorageTypeInt = 45;
const int name_StorageTypeFloat = 46;
const int name_StorageTypeBool = 47;
const int name_StorageTypeString = 48;
const int name_StorageTypeList = 49;
const int name_StorageTypeOpaquePointer = 50;
const int name_StorageTypeTerm = 51;
const int name_StorageTypeType = 52;
const int name_StorageTypeHandle = 53;
const int name_StorageTypeHashtable = 54;
const int name_StorageTypeObject = 55;
const int tok_Identifier = 56;
const int tok_Name = 57;
const int tok_Integer = 58;
const int tok_HexInteger = 59;
const int tok_Float = 60;
const int tok_String = 61;
const int tok_Color = 62;
const int tok_Bool = 63;
const int tok_LParen = 64;
const int tok_RParen = 65;
const int tok_LBrace = 66;
const int tok_RBrace = 67;
const int tok_LBracket = 68;
const int tok_RBracket = 69;
const int tok_Comma = 70;
const int tok_At = 71;
const int tok_Dot = 72;
const int tok_DotAt = 73;
const int tok_Star = 74;
const int tok_Question = 75;
const int tok_Slash = 76;
const int tok_DoubleSlash = 77;
const int tok_Plus = 78;
const int tok_Minus = 79;
const int tok_LThan = 80;
const int tok_LThanEq = 81;
const int tok_GThan = 82;
const int tok_GThanEq = 83;
const int tok_Percent = 84;
const int tok_Colon = 85;
const int tok_DoubleColon = 86;
const int tok_DoubleEquals = 87;
const int tok_NotEquals = 88;
const int tok_Equals = 89;
const int tok_PlusEquals = 90;
const int tok_MinusEquals = 91;
const int tok_StarEquals = 92;
const int tok_SlashEquals = 93;
const int tok_ColonEquals = 94;
const int tok_RightArrow = 95;
const int tok_LeftArrow = 96;
const int tok_Ampersand = 97;
const int tok_DoubleAmpersand = 98;
const int tok_DoubleVerticalBar = 99;
const int tok_Semicolon = 100;
const int tok_TwoDots = 101;
const int tok_Ellipsis = 102;
const int tok_TripleLThan = 103;
const int tok_TripleGThan = 104;
const int tok_Pound = 105;
const int tok_Def = 106;
const int tok_Type = 107;
const int tok_Begin = 108;
const int tok_Do = 109;
const int tok_End = 110;
const int tok_If = 111;
const int tok_Else = 112;
const int tok_Elif = 113;
const int tok_For = 114;
const int tok_State = 115;
const int tok_Return = 116;
const int tok_In = 117;
const int tok_True = 118;
const int tok_False = 119;
const int tok_Namespace = 120;
const int tok_Include = 121;
const int tok_And = 122;
const int tok_Or = 123;
const int tok_Not = 124;
const int tok_Discard = 125;
const int tok_Null = 126;
const int tok_Break = 127;
const int tok_Continue = 128;
const int tok_Switch = 129;
const int tok_Case = 130;
const int tok_While = 131;
const int tok_Require = 132;
const int tok_Package = 133;
const int tok_Section = 134;
const int tok_Whitespace = 135;
const int tok_Newline = 136;
const int tok_Comment = 137;
const int tok_Eof = 138;
const int tok_Unrecognized = 139;
const int op_NoOp = 140;
const int op_Pause = 141;
const int op_SetNull = 142;
const int op_InlineCopy = 143;
const int op_CallBlock = 144;
const int op_DynamicCall = 145;
const int op_ClosureCall = 146;
const int op_FireNative = 147;
const int op_CaseBlock = 148;
const int op_ForLoop = 149;
const int op_ExitPoint = 150;
const int op_FinishFrame = 151;
const int op_FinishLoop = 152;
const int op_ErrorNotEnoughInputs = 153;
const int op_ErrorTooManyInputs = 154;
const int name_LoopProduceOutput = 155;
const int name_FlatOutputs = 156;
const int name_OutputsToList = 157;
const int name_Multiple = 158;
const int name_Cast = 159;
const int name_DynamicMethodOutput = 160;
const int name_FirstStatIndex = 161;
const int stat_TermsCreated = 162;
const int stat_TermPropAdded = 163;
const int stat_TermPropAccess = 164;
const int stat_InternedNameLookup = 165;
const int stat_InternedNameCreate = 166;
const int stat_Copy_PushedInputNewFrame = 167;
const int stat_Copy_PushedInputMultiNewFrame = 168;
const int stat_Copy_PushFrameWithInputs = 169;
const int stat_Copy_ListDuplicate = 170;
const int stat_Copy_LoopCopyRebound = 171;
const int stat_Cast_ListCastElement = 172;
const int stat_Cast_PushFrameWithInputs = 173;
const int stat_Cast_FinishFrame = 174;
const int stat_Touch_ListCast = 175;
const int stat_ValueCreates = 176;
const int stat_ValueCopies = 177;
const int stat_ValueCast = 178;
const int stat_ValueCastDispatched = 179;
const int stat_ValueTouch = 180;
const int stat_ListsCreated = 181;
const int stat_ListsGrown = 182;
const int stat_ListSoftCopy = 183;
const int stat_ListHardCopy = 184;
const int stat_DictHardCopy = 185;
const int stat_StringCreate = 186;
const int stat_StringDuplicate = 187;
const int stat_StringResizeInPlace = 188;
const int stat_StringResizeCreate = 189;
const int stat_StringSoftCopy = 190;
const int stat_StringToStd = 191;
const int stat_StepInterpreter = 192;
const int stat_InterpreterCastOutputFromFinishedFrame = 193;
const int stat_BlockNameLookups = 194;
const int stat_PushFrame = 195;
const int stat_LoopFinishIteration = 196;
const int stat_LoopWriteOutput = 197;
const int stat_WriteTermBytecode = 198;
const int stat_DynamicCall = 199;
const int stat_FinishDynamicCall = 200;
const int stat_DynamicMethodCall = 201;
const int stat_SetIndex = 202;
const int stat_SetField = 203;
const int name_LastStatIndex = 204;
const int name_LastBuiltinName = 205;

const char* builtin_name_to_string(int name);

} // namespace circa
