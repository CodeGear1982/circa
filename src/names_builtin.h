// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// This file was generated using name_tool.py

#pragma once

namespace circa {

const int s_addr = 0;
const int s_advance = 1;
const int s_block = 2;
const int s_break = 3;
const int s_case = 4;
const int s_comment = 5;
const int s_continue = 6;
const int s_conditional_done = 7;
const int s_count = 8;
const int s_current = 9;
const int s_current_term = 10;
const int s_declared_state = 11;
const int s_discard = 12;
const int s_done = 13;
const int s_entity_id = 14;
const int s_error = 15;
const int s_expr = 16;
const int s_failure = 17;
const int s_file = 18;
const int s_filename = 19;
const int s_format = 20;
const int s_frames = 21;
const int s_has_state = 22;
const int s_hidden = 23;
const int s_ident = 24;
const int s_incoming = 25;
const int s_index = 26;
const int s_inputs = 27;
const int s_invalid = 28;
const int s_iterator = 29;
const int s_iterator_value = 30;
const int s_items = 31;
const int s_key = 32;
const int s_last = 33;
const int s_list = 34;
const int s_liveness_list = 35;
const int s_local_state = 36;
const int s_local_state_key = 37;
const int s_loop = 38;
const int s_maddr = 39;
const int s_major_block = 40;
const int s_maybe = 41;
const int s_memoize = 42;
const int s_message = 43;
const int s_method_name = 44;
const int s_methodCache = 45;
const int s_multiple = 46;
const int s_next = 47;
const int s_next_case = 48;
const int s_no = 49;
const int s_normal = 50;
const int s_none = 51;
const int s_newline = 52;
const int s_out = 53;
const int s_outputSlot = 54;
const int s_outgoing = 55;
const int s_pc = 56;
const int s_produceOutput = 57;
const int s_repeat = 58;
const int s_return = 59;
const int s_slot = 60;
const int s_slots = 61;
const int s_slotCount = 62;
const int s_state = 63;
const int s_statement = 64;
const int s_step = 65;
const int s_switch = 66;
const int s_success = 67;
const int s_type = 68;
const int s_term = 69;
const int s_top = 70;
const int s_unknown = 71;
const int s_yes = 72;
const int s_EvaluationEmpty = 73;
const int s_HasEffects = 74;
const int s_HasControlFlow = 75;
const int s_HasDynamicDispatch = 76;
const int s_DirtyStateType = 77;
const int s_Builtins = 78;
const int s_ModuleName = 79;
const int s_StaticErrors = 80;
const int s_IsModule = 81;
const int s_AccumulatingOutput = 82;
const int s_Constructor = 83;
const int s_Error = 84;
const int s_ExplicitState = 85;
const int s_ExplicitType = 86;
const int s_Field = 87;
const int s_FieldAccessor = 88;
const int s_Final = 89;
const int s_HiddenInput = 90;
const int s_Implicit = 91;
const int s_IgnoreError = 92;
const int s_LocalStateResult = 93;
const int s_Meta = 94;
const int s_Message = 95;
const int s_MethodName = 96;
const int s_ModifyList = 97;
const int s_Mutability = 98;
const int s_Optional = 99;
const int s_OriginalText = 100;
const int s_OverloadedFunc = 101;
const int s_Ref = 102;
const int s_Rebind = 103;
const int s_Setter = 104;
const int s_Output = 105;
const int s_PreferSpecialize = 106;
const int s_Error_UnknownType = 107;
const int s_Syntax_AnonFunction = 108;
const int s_Syntax_BlockStyle = 109;
const int s_Syntax_Brackets = 110;
const int s_Syntax_ColorFormat = 111;
const int s_Syntax_DeclarationStyle = 112;
const int s_Syntax_ExplicitType = 113;
const int s_Syntax_FunctionName = 114;
const int s_Syntax_IdentifierRebind = 115;
const int s_Syntax_ImplicitName = 116;
const int s_Syntax_Import = 117;
const int s_Syntax_InputFormat = 118;
const int s_Syntax_IntegerFormat = 119;
const int s_Syntax_LineEnding = 120;
const int s_Syntax_LiteralList = 121;
const int s_Syntax_MethodDecl = 122;
const int s_Syntax_Multiline = 123;
const int s_Syntax_NameBinding = 124;
const int s_Syntax_NoBrackets = 125;
const int s_Syntax_NoParens = 126;
const int s_Syntax_Operator = 127;
const int s_Syntax_OriginalFormat = 128;
const int s_Syntax_Parens = 129;
const int s_Syntax_PreWs = 130;
const int s_Syntax_PreDotWs = 131;
const int s_Syntax_PreOperatorWs = 132;
const int s_Syntax_PreEndWs = 133;
const int s_Syntax_PreEqualsSpace = 134;
const int s_Syntax_PreLBracketWs = 135;
const int s_Syntax_PreRBracketWs = 136;
const int s_Syntax_PostEqualsSpace = 137;
const int s_Syntax_PostFunctionWs = 138;
const int s_Syntax_PostKeywordWs = 139;
const int s_Syntax_PostLBracketWs = 140;
const int s_Syntax_PostHeadingWs = 141;
const int s_Syntax_PostNameWs = 142;
const int s_Syntax_PostWs = 143;
const int s_Syntax_PostOperatorWs = 144;
const int s_Syntax_Properties = 145;
const int s_Syntax_QuoteType = 146;
const int s_Syntax_RebindSymbol = 147;
const int s_Syntax_RebindOperator = 148;
const int s_Syntax_RebindingInfix = 149;
const int s_Syntax_ReturnStatement = 150;
const int s_Syntax_Require = 151;
const int s_Syntax_RequireLocal = 152;
const int s_Syntax_StateKeyword = 153;
const int s_Syntax_TypeMagicSymbol = 154;
const int s_Syntax_WhitespaceBeforeEnd = 155;
const int s_Syntax_WhitespacePreColon = 156;
const int s_Syntax_WhitespacePostColon = 157;
const int s_Wildcard = 158;
const int s_RecursiveWildcard = 159;
const int s_Function = 160;
const int s_TypeRelease = 161;
const int s_FileNotFound = 162;
const int s_NotEnoughInputs = 163;
const int s_TooManyInputs = 164;
const int s_ExtraOutputNotFound = 165;
const int s_Default = 166;
const int s_ByDemand = 167;
const int s_Unevaluated = 168;
const int s_InProgress = 169;
const int s_Lazy = 170;
const int s_Consumed = 171;
const int s_Uncaptured = 172;
const int s_Return = 173;
const int s_Continue = 174;
const int s_Break = 175;
const int s_Discard = 176;
const int s_Control = 177;
const int s_ExitLevelFunction = 178;
const int s_ExitLevelLoop = 179;
const int s_HighestExitLevel = 180;
const int s_ExtraReturn = 181;
const int s_Name = 182;
const int s_Primary = 183;
const int s_Anonymous = 184;
const int s_Entropy = 185;
const int s_OnDemand = 186;
const int s_dev_compile = 187;
const int s_hacks = 188;
const int s_no_effect = 189;
const int s_no_save_state = 190;
const int s_effect = 191;
const int s_set_value = 192;
const int s_watch = 193;
const int s_Copy = 194;
const int s_Move = 195;
const int s_Unobservable = 196;
const int s_TermCounter = 197;
const int s_Watch = 198;
const int s_StackReady = 199;
const int s_StackRunning = 200;
const int s_StackFinished = 201;
const int s_InfixOperator = 202;
const int s_FunctionName = 203;
const int s_TypeName = 204;
const int s_TermName = 205;
const int s_Keyword = 206;
const int s_Whitespace = 207;
const int s_UnknownIdentifier = 208;
const int s_LookupAny = 209;
const int s_LookupType = 210;
const int s_LookupFunction = 211;
const int s_Untyped = 212;
const int s_UniformListType = 213;
const int s_AnonStructType = 214;
const int s_StructType = 215;
const int s_NativePatch = 216;
const int s_RecompileModule = 217;
const int s_Filesystem = 218;
const int s_Tarball = 219;
const int s_Bootstrapping = 220;
const int s_Done = 221;
const int s_StorageTypeNull = 222;
const int s_StorageTypeInt = 223;
const int s_StorageTypeFloat = 224;
const int s_StorageTypeBlob = 225;
const int s_StorageTypeBool = 226;
const int s_StorageTypeStack = 227;
const int s_StorageTypeString = 228;
const int s_StorageTypeList = 229;
const int s_StorageTypeOpaquePointer = 230;
const int s_StorageTypeTerm = 231;
const int s_StorageTypeType = 232;
const int s_StorageTypeHandle = 233;
const int s_StorageTypeHashtable = 234;
const int s_StorageTypeObject = 235;
const int s_InterfaceType = 236;
const int s_Delete = 237;
const int s_Insert = 238;
const int s_Element = 239;
const int s_Key = 240;
const int s_Replace = 241;
const int s_Append = 242;
const int s_Truncate = 243;
const int s_ChangeAppend = 244;
const int s_ChangeRename = 245;
const int tok_Identifier = 246;
const int tok_ColonString = 247;
const int tok_Integer = 248;
const int tok_HexInteger = 249;
const int tok_Float = 250;
const int tok_String = 251;
const int tok_Color = 252;
const int tok_Bool = 253;
const int tok_LParen = 254;
const int tok_RParen = 255;
const int tok_LBrace = 256;
const int tok_RBrace = 257;
const int tok_LSquare = 258;
const int tok_RSquare = 259;
const int tok_Comma = 260;
const int tok_At = 261;
const int tok_Dot = 262;
const int tok_DotAt = 263;
const int tok_Star = 264;
const int tok_DoubleStar = 265;
const int tok_Question = 266;
const int tok_Slash = 267;
const int tok_DoubleSlash = 268;
const int tok_Plus = 269;
const int tok_Minus = 270;
const int tok_LThan = 271;
const int tok_LThanEq = 272;
const int tok_GThan = 273;
const int tok_GThanEq = 274;
const int tok_Percent = 275;
const int tok_Colon = 276;
const int tok_DoubleColon = 277;
const int tok_DoubleEquals = 278;
const int tok_NotEquals = 279;
const int tok_Equals = 280;
const int tok_PlusEquals = 281;
const int tok_MinusEquals = 282;
const int tok_StarEquals = 283;
const int tok_SlashEquals = 284;
const int tok_ColonEquals = 285;
const int tok_RightArrow = 286;
const int tok_FatArrow = 287;
const int tok_LeftArrow = 288;
const int tok_Ampersand = 289;
const int tok_DoubleAmpersand = 290;
const int tok_VerticalBar = 291;
const int tok_DoubleVerticalBar = 292;
const int tok_Semicolon = 293;
const int tok_TwoDots = 294;
const int tok_Ellipsis = 295;
const int tok_TripleLThan = 296;
const int tok_TripleGThan = 297;
const int tok_Pound = 298;
const int tok_Def = 299;
const int tok_Struct = 300;
const int tok_UnusedName1 = 301;
const int tok_UnusedName2 = 302;
const int tok_UnusedName3 = 303;
const int tok_If = 304;
const int tok_Else = 305;
const int tok_Elif = 306;
const int tok_For = 307;
const int tok_While = 308;
const int tok_State = 309;
const int tok_Return = 310;
const int tok_In = 311;
const int tok_Let = 312;
const int tok_True = 313;
const int tok_False = 314;
const int tok_Namespace = 315;
const int tok_Include = 316;
const int tok_And = 317;
const int tok_Or = 318;
const int tok_Not = 319;
const int tok_Discard = 320;
const int tok_Nil = 321;
const int tok_Break = 322;
const int tok_Continue = 323;
const int tok_Switch = 324;
const int tok_Case = 325;
const int tok_Require = 326;
const int tok_RequireLocal = 327;
const int tok_Import = 328;
const int tok_Package = 329;
const int tok_Section = 330;
const int tok_Whitespace = 331;
const int tok_Newline = 332;
const int tok_Comment = 333;
const int tok_Eof = 334;
const int tok_Unrecognized = 335;
const int s_NormalCall = 336;
const int s_FuncApply = 337;
const int s_FuncCall = 338;
const int s_FirstStatIndex = 339;
const int stat_TermCreated = 340;
const int stat_TermPropAdded = 341;
const int stat_TermPropAccess = 342;
const int stat_NameSearch = 343;
const int stat_NameSearchStep = 344;
const int stat_FindModule = 345;
const int stat_Bytecode_WriteTerm = 346;
const int stat_Bytecode_CreateEntry = 347;
const int stat_LoadFrameState = 348;
const int stat_StoreFrameState = 349;
const int stat_AppendMove = 350;
const int stat_GetIndexCopy = 351;
const int stat_GetIndexMove = 352;
const int stat_Interpreter_Step = 353;
const int stat_Interpreter_DynamicMethod_CacheHit = 354;
const int stat_Interpreter_DynamicMethod_SlowLookup = 355;
const int stat_Interpreter_DynamicMethod_SlowLookup_Module = 356;
const int stat_Interpreter_DynamicMethod_SlowLookup_Hashtable = 357;
const int stat_Interpreter_DynamicMethod_ModuleLookup = 358;
const int stat_Interpreter_DynamicFuncToClosureCall = 359;
const int stat_Interpreter_CopyTermValue = 360;
const int stat_Interpreter_CopyStackValue = 361;
const int stat_Interpreter_MoveStackValue = 362;
const int stat_Interpreter_CopyConst = 363;
const int stat_FindEnvValue = 364;
const int stat_Make = 365;
const int stat_Copy = 366;
const int stat_Cast = 367;
const int stat_ValueCastDispatched = 368;
const int stat_Touch = 369;
const int stat_BlobDuplicate = 370;
const int stat_ListsCreated = 371;
const int stat_ListsGrown = 372;
const int stat_ListSoftCopy = 373;
const int stat_ListDuplicate = 374;
const int stat_ListDuplicate_100Count = 375;
const int stat_ListDuplicate_ElementCopy = 376;
const int stat_ListCast_Touch = 377;
const int stat_ListCast_CastElement = 378;
const int stat_HashtableDuplicate = 379;
const int stat_HashtableDuplicate_Copy = 380;
const int stat_StringCreate = 381;
const int stat_StringDuplicate = 382;
const int stat_StringResizeInPlace = 383;
const int stat_StringResizeCreate = 384;
const int stat_StringSoftCopy = 385;
const int stat_StringToStd = 386;
const int stat_DynamicCall = 387;
const int stat_FinishDynamicCall = 388;
const int stat_DynamicMethodCall = 389;
const int stat_SetIndex = 390;
const int stat_SetField = 391;
const int stat_SetWithSelector_Touch_List = 392;
const int stat_SetWithSelector_Touch_Hashtable = 393;
const int stat_StackPushFrame = 394;
const int s_LastStatIndex = 395;
const int s_LastBuiltinName = 396;

const char* builtin_symbol_to_string(int name);
int builtin_symbol_from_string(const char* str);

} // namespace circa
