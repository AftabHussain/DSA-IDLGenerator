; ModuleID = 'merged_mgr_mydvc.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.packet = type { i32, i8* }
%struct.device_operations = type { i32 (%struct.packet*)*, void (%struct.device_operations*)*, void (%struct.device_operations*)*, {}*, i32, i8* }
%struct.device_manager = type { [10 x i8*], i32, i32 }
%struct.device = type { i8*, [10 x i8], void (%struct.device*, i8*)*, float, float }
%struct.device.0 = type { i8*, [10 x i8], {}*, float, float }

@.str.1 = private unnamed_addr constant [11 x i8] c"new device\00", align 1
@.str = private unnamed_addr constant [15 x i8] c"another device\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"PRINTER\00", align 1
@main.devops = internal global { i32 (%struct.packet*)*, void (%struct.device_operations*)*, void (%struct.device_operations*)*, i32 (%struct.device_operations*)*, i32, i8* } { i32 (%struct.packet*)* @mydev_init, void (%struct.device_operations*)* @mydev_uninit, void (%struct.device_operations*)* @readAndWriteData, i32 (%struct.device_operations*)* @readData, i32 0, i8* null }, align 8
@dm = common global %struct.device_manager zeroinitializer, align 8
@.str.3 = private unnamed_addr constant [10 x i8] c"write sth\00", align 1

; Function Attrs: norecurse nounwind uwtable
define i32 @fn_n(%struct.device_operations* nocapture %devops) #0 !dbg !4 {
  tail call void @llvm.dbg.value(metadata %struct.device_operations* %devops, i64 0, metadata !33, metadata !131), !dbg !132
  %1 = getelementptr inbounds %struct.device_operations, %struct.device_operations* %devops, i64 0, i32 5, !dbg !133
  %2 = getelementptr inbounds %struct.device_operations, %struct.device_operations* %devops, i64 0, i32 4, !dbg !134
  %3 = load i32, i32* %2, align 8, !dbg !134, !tbaa !136
  %4 = icmp sgt i32 %3, 0, !dbg !142
  %. = select i1 %4, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i64 0, i64 0), !dbg !143
  store i8* %., i8** %1, align 8, !dbg !144
  ret i32 0, !dbg !146
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

; Function Attrs: nounwind uwtable
define i32 @main() #2 !dbg !34 {
  %my_printer = alloca %struct.device, align 8
  %1 = bitcast %struct.device* %my_printer to i8*, !dbg !147
  call void @llvm.lifetime.start(i64 40, i8* %1) #5, !dbg !147
  %2 = getelementptr inbounds %struct.device, %struct.device* %my_printer, i64 0, i32 2, !dbg !148
  store void (%struct.device*, i8*)* bitcast (void (%struct.device.0*, i8*)* @reg_device to void (%struct.device*, i8*)*), void (%struct.device*, i8*)** %2, align 8, !dbg !149, !tbaa !150
  tail call void @llvm.dbg.value(metadata %struct.device* %my_printer, i64 0, metadata !38, metadata !153), !dbg !154
  call void bitcast (void (%struct.device.0*, i8*)* @reg_device to void (%struct.device*, i8*)*)(%struct.device* nonnull %my_printer, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i64 0, i64 0)) #5, !dbg !155
  %3 = call i32 @reg_devops(void (%struct.device_operations*)* nonnull @readAndWriteData, %struct.device_operations* bitcast ({ i32 (%struct.packet*)*, void (%struct.device_operations*)*, void (%struct.device_operations*)*, i32 (%struct.device_operations*)*, i32, i8* }* @main.devops to %struct.device_operations*)) #5, !dbg !156
  call void @llvm.lifetime.end(i64 40, i8* %1) #5, !dbg !157
  ret i32 0, !dbg !157
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #3

; Function Attrs: norecurse nounwind uwtable
define internal void @readAndWriteData(%struct.device_operations* nocapture %devops) #0 !dbg !60 {
  tail call void @llvm.dbg.value(metadata %struct.device_operations* %devops, i64 0, metadata !62, metadata !131), !dbg !158
  %1 = getelementptr inbounds %struct.device_operations, %struct.device_operations* %devops, i64 0, i32 4, !dbg !159
  %2 = load i32, i32* %1, align 8, !dbg !159, !tbaa !136
  %3 = icmp sgt i32 %2, 0, !dbg !161
  br i1 %3, label %4, label %6, !dbg !162

; <label>:4                                       ; preds = %0
  %5 = getelementptr inbounds %struct.device_operations, %struct.device_operations* %devops, i64 0, i32 5, !dbg !163
  store i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), i8** %5, align 8, !dbg !165, !tbaa !166
  br label %6, !dbg !167

; <label>:6                                       ; preds = %4, %0
  ret void, !dbg !168
}

; Function Attrs: norecurse nounwind uwtable
define internal i32 @mydev_init(%struct.packet* nocapture %p) #0 !dbg !54 {
  tail call void @llvm.dbg.value(metadata %struct.packet* %p, i64 0, metadata !56, metadata !131), !dbg !169
  %1 = getelementptr inbounds %struct.packet, %struct.packet* %p, i64 0, i32 0, !dbg !170
  store i32 0, i32* %1, align 8, !dbg !171, !tbaa !172
  %2 = getelementptr inbounds %struct.packet, %struct.packet* %p, i64 0, i32 1, !dbg !174
  store i8* null, i8** %2, align 8, !dbg !175, !tbaa !176
  ret i32 1, !dbg !177
}

; Function Attrs: norecurse nounwind uwtable
define internal void @mydev_uninit(%struct.device_operations* nocapture %devops) #0 !dbg !57 {
  tail call void @llvm.dbg.value(metadata %struct.device_operations* %devops, i64 0, metadata !59, metadata !131), !dbg !178
  %1 = getelementptr inbounds %struct.device_operations, %struct.device_operations* %devops, i64 0, i32 4, !dbg !179
  store i32 0, i32* %1, align 8, !dbg !180, !tbaa !136
  ret void, !dbg !181
}

; Function Attrs: norecurse nounwind readonly uwtable
define internal i32 @readData(%struct.device_operations* nocapture readonly %devops) #4 !dbg !63 {
  tail call void @llvm.dbg.value(metadata %struct.device_operations* %devops, i64 0, metadata !65, metadata !131), !dbg !182
  %1 = getelementptr inbounds %struct.device_operations, %struct.device_operations* %devops, i64 0, i32 4, !dbg !183
  %2 = load i32, i32* %1, align 8, !dbg !183, !tbaa !136
  %3 = icmp sgt i32 %2, 0, !dbg !185
  %. = zext i1 %3 to i32, !dbg !186
  ret i32 %., !dbg !188
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #3

; Function Attrs: norecurse nounwind uwtable
define void @reg_device(%struct.device.0* nocapture %dev, i8* %name) #0 !dbg !71 {
  tail call void @llvm.dbg.value(metadata %struct.device.0* %dev, i64 0, metadata !84, metadata !131), !dbg !189
  tail call void @llvm.dbg.value(metadata i8* %name, i64 0, metadata !85, metadata !131), !dbg !190
  %1 = getelementptr inbounds %struct.device.0, %struct.device.0* %dev, i64 0, i32 0, !dbg !191
  store i8* %name, i8** %1, align 8, !dbg !192, !tbaa !193
  %2 = load i32, i32* getelementptr inbounds (%struct.device_manager, %struct.device_manager* @dm, i64 0, i32 1), align 8, !dbg !194, !tbaa !195
  %3 = sext i32 %2 to i64, !dbg !197
  %4 = getelementptr inbounds %struct.device_manager, %struct.device_manager* @dm, i64 0, i32 0, i64 %3, !dbg !197
  store i8* %name, i8** %4, align 8, !dbg !198, !tbaa !199
  ret void, !dbg !200
}

; Function Attrs: norecurse nounwind uwtable
define i32 @foobar(%struct.device_operations* nocapture %devop) #0 !dbg !86 {
  tail call void @llvm.dbg.value(metadata %struct.device_operations* %devop, i64 0, metadata !111, metadata !131), !dbg !201
  %1 = getelementptr inbounds %struct.device_operations, %struct.device_operations* %devop, i64 0, i32 5, !dbg !202
  store i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.3, i64 0, i64 0), i8** %1, align 8, !dbg !203, !tbaa !166
  ret i32 0, !dbg !204
}

; Function Attrs: nounwind uwtable
define i32 @reg_devops(void (%struct.device_operations*)* nocapture readnone %fp, %struct.device_operations* %devops) #2 !dbg !112 {
  tail call void @llvm.dbg.value(metadata void (%struct.device_operations*)* %fp, i64 0, metadata !116, metadata !131), !dbg !205
  tail call void @llvm.dbg.value(metadata %struct.device_operations* %devops, i64 0, metadata !117, metadata !131), !dbg !206
  tail call void @llvm.dbg.value(metadata i32 (%struct.device_operations*)* @fn_n, i64 0, metadata !118, metadata !131), !dbg !207
  %1 = tail call i32 @fn_n(%struct.device_operations* %devops) #5, !dbg !208
  ret i32 1, !dbg !209
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.dbg.cu = !{!0, !68}
!llvm.ident = !{!128, !128}
!llvm.module.flags = !{!129, !130}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.8.1 (https://github.com/llvm-mirror/clang.git 07a6361e0f32f699d47c124106e7911b584974d4) (https://github.com/llvm-mirror/llvm.git 051e787f26dbfdc26cf61a57bc82ca00dcb812e8)", isOptimized: true, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !3, globals: !66)
!1 = !DIFile(filename: "my_device.c", directory: "/local/devel/DataStructureAnalysis/build")
!2 = !{}
!3 = !{!4, !34, !54, !57, !60, !63}
!4 = distinct !DISubprogram(name: "fn_n", scope: !1, file: !1, line: 44, type: !5, isLocal: false, isDefinition: true, scopeLine: 44, flags: DIFlagPrototyped, isOptimized: true, variables: !32)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64, align: 64)
!9 = !DICompositeType(tag: DW_TAG_structure_type, name: "device_operations", file: !10, line: 18, size: 384, align: 64, elements: !11)
!10 = !DIFile(filename: "./device.h", directory: "/local/devel/DataStructureAnalysis/build")
!11 = !{!12, !23, !27, !28, !30, !31}
!12 = !DIDerivedType(tag: DW_TAG_member, name: "dop_init", scope: !9, file: !10, line: 21, baseType: !13, size: 64, align: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64, align: 64)
!14 = !DISubroutineType(types: !15)
!15 = !{!7, !16}
!16 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64, align: 64)
!17 = !DICompositeType(tag: DW_TAG_structure_type, name: "packet", file: !10, line: 4, size: 128, align: 64, elements: !18)
!18 = !{!19, !20}
!19 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !17, file: !10, line: 5, baseType: !7, size: 32, align: 32)
!20 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !17, file: !10, line: 6, baseType: !21, size: 64, align: 64, offset: 64)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64, align: 64)
!22 = !DIBasicType(name: "char", size: 8, align: 8, encoding: DW_ATE_signed_char)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "dop_uninit", scope: !9, file: !10, line: 24, baseType: !24, size: 64, align: 64, offset: 64)
!24 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !25, size: 64, align: 64)
!25 = !DISubroutineType(types: !26)
!26 = !{null, !8}
!27 = !DIDerivedType(tag: DW_TAG_member, name: "dop_read_write", scope: !9, file: !10, line: 25, baseType: !24, size: 64, align: 64, offset: 128)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "dop_read", scope: !9, file: !10, line: 26, baseType: !29, size: 64, align: 64, offset: 192)
!29 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64, align: 64)
!30 = !DIDerivedType(tag: DW_TAG_member, name: "devop_init_registered", scope: !9, file: !10, line: 28, baseType: !7, size: 32, align: 32, offset: 256)
!31 = !DIDerivedType(tag: DW_TAG_member, name: "device_info", scope: !9, file: !10, line: 29, baseType: !21, size: 64, align: 64, offset: 320)
!32 = !{!33}
!33 = !DILocalVariable(name: "devops", arg: 1, scope: !4, file: !1, line: 44, type: !8)
!34 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 63, type: !35, isLocal: false, isDefinition: true, scopeLine: 63, isOptimized: true, variables: !37)
!35 = !DISubroutineType(types: !36)
!36 = !{!7}
!37 = !{!38}
!38 = !DILocalVariable(name: "my_printer", scope: !34, file: !1, line: 68, type: !39)
!39 = !DICompositeType(tag: DW_TAG_structure_type, name: "device", file: !10, line: 9, size: 320, align: 64, elements: !40)
!40 = !{!41, !42, !46, !51, !53}
!41 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !39, file: !10, line: 10, baseType: !21, size: 64, align: 64)
!42 = !DIDerivedType(tag: DW_TAG_member, name: "status", scope: !39, file: !10, line: 11, baseType: !43, size: 80, align: 8, offset: 64)
!43 = !DICompositeType(tag: DW_TAG_array_type, baseType: !22, size: 80, align: 8, elements: !44)
!44 = !{!45}
!45 = !DISubrange(count: 10)
!46 = !DIDerivedType(tag: DW_TAG_member, name: "set_device", scope: !39, file: !10, line: 13, baseType: !47, size: 64, align: 64, offset: 192)
!47 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !48, size: 64, align: 64)
!48 = !DISubroutineType(types: !49)
!49 = !{null, !50, !21}
!50 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !39, size: 64, align: 64)
!51 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !39, file: !10, line: 14, baseType: !52, size: 32, align: 32, offset: 256)
!52 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!53 = !DIDerivedType(tag: DW_TAG_member, name: "system_id", scope: !39, file: !10, line: 15, baseType: !52, size: 32, align: 32, offset: 288)
!54 = distinct !DISubprogram(name: "mydev_init", scope: !1, file: !1, line: 8, type: !14, isLocal: true, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: true, variables: !55)
!55 = !{!56}
!56 = !DILocalVariable(name: "p", arg: 1, scope: !54, file: !1, line: 8, type: !16)
!57 = distinct !DISubprogram(name: "mydev_uninit", scope: !1, file: !1, line: 15, type: !25, isLocal: true, isDefinition: true, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: true, variables: !58)
!58 = !{!59}
!59 = !DILocalVariable(name: "devops", arg: 1, scope: !57, file: !1, line: 15, type: !8)
!60 = distinct !DISubprogram(name: "readAndWriteData", scope: !1, file: !1, line: 37, type: !25, isLocal: true, isDefinition: true, scopeLine: 37, flags: DIFlagPrototyped, isOptimized: true, variables: !61)
!61 = !{!62}
!62 = !DILocalVariable(name: "devops", arg: 1, scope: !60, file: !1, line: 37, type: !8)
!63 = distinct !DISubprogram(name: "readData", scope: !1, file: !1, line: 20, type: !5, isLocal: true, isDefinition: true, scopeLine: 20, flags: DIFlagPrototyped, isOptimized: true, variables: !64)
!64 = !{!65}
!65 = !DILocalVariable(name: "devops", arg: 1, scope: !63, file: !1, line: 20, type: !8)
!66 = !{!67}
!67 = !DIGlobalVariable(name: "devops", scope: !34, file: !1, line: 84, type: !9, isLocal: true, isDefinition: true, variable: { i32 (%struct.packet*)*, void (%struct.device_operations*)*, void (%struct.device_operations*)*, i32 (%struct.device_operations*)*, i32, i8* }* @main.devops)
!68 = distinct !DICompileUnit(language: DW_LANG_C99, file: !69, producer: "clang version 3.8.1 (https://github.com/llvm-mirror/clang.git 07a6361e0f32f699d47c124106e7911b584974d4) (https://github.com/llvm-mirror/llvm.git 051e787f26dbfdc26cf61a57bc82ca00dcb812e8)", isOptimized: true, runtimeVersion: 0, emissionKind: 1, enums: !2, subprograms: !70, globals: !119)
!69 = !DIFile(filename: "device_manager.c", directory: "/local/devel/DataStructureAnalysis/build")
!70 = !{!71, !86, !112}
!71 = distinct !DISubprogram(name: "reg_device", scope: !69, file: !69, line: 22, type: !72, isLocal: false, isDefinition: true, scopeLine: 22, flags: DIFlagPrototyped, isOptimized: true, variables: !83)
!72 = !DISubroutineType(types: !73)
!73 = !{null, !74, !21}
!74 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !75, size: 64, align: 64)
!75 = !DICompositeType(tag: DW_TAG_structure_type, name: "device", file: !10, line: 9, size: 320, align: 64, elements: !76)
!76 = !{!77, !78, !79, !81, !82}
!77 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !75, file: !10, line: 10, baseType: !21, size: 64, align: 64)
!78 = !DIDerivedType(tag: DW_TAG_member, name: "status", scope: !75, file: !10, line: 11, baseType: !43, size: 80, align: 8, offset: 64)
!79 = !DIDerivedType(tag: DW_TAG_member, name: "set_device", scope: !75, file: !10, line: 13, baseType: !80, size: 64, align: 64, offset: 192)
!80 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !72, size: 64, align: 64)
!81 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !75, file: !10, line: 14, baseType: !52, size: 32, align: 32, offset: 256)
!82 = !DIDerivedType(tag: DW_TAG_member, name: "system_id", scope: !75, file: !10, line: 15, baseType: !52, size: 32, align: 32, offset: 288)
!83 = !{!84, !85}
!84 = !DILocalVariable(name: "dev", arg: 1, scope: !71, file: !69, line: 22, type: !74)
!85 = !DILocalVariable(name: "name", arg: 2, scope: !71, file: !69, line: 22, type: !21)
!86 = distinct !DISubprogram(name: "foobar", scope: !69, file: !69, line: 42, type: !87, isLocal: false, isDefinition: true, scopeLine: 42, flags: DIFlagPrototyped, isOptimized: true, variables: !110)
!87 = !DISubroutineType(types: !88)
!88 = !{!7, !89}
!89 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !90, size: 64, align: 64)
!90 = !DICompositeType(tag: DW_TAG_structure_type, name: "device_operations", file: !10, line: 18, size: 384, align: 64, elements: !91)
!91 = !{!92, !101, !105, !106, !108, !109}
!92 = !DIDerivedType(tag: DW_TAG_member, name: "dop_init", scope: !90, file: !10, line: 21, baseType: !93, size: 64, align: 64)
!93 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !94, size: 64, align: 64)
!94 = !DISubroutineType(types: !95)
!95 = !{!7, !96}
!96 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !97, size: 64, align: 64)
!97 = !DICompositeType(tag: DW_TAG_structure_type, name: "packet", file: !10, line: 4, size: 128, align: 64, elements: !98)
!98 = !{!99, !100}
!99 = !DIDerivedType(tag: DW_TAG_member, name: "size", scope: !97, file: !10, line: 5, baseType: !7, size: 32, align: 32)
!100 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !97, file: !10, line: 6, baseType: !21, size: 64, align: 64, offset: 64)
!101 = !DIDerivedType(tag: DW_TAG_member, name: "dop_uninit", scope: !90, file: !10, line: 24, baseType: !102, size: 64, align: 64, offset: 64)
!102 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !103, size: 64, align: 64)
!103 = !DISubroutineType(types: !104)
!104 = !{null, !89}
!105 = !DIDerivedType(tag: DW_TAG_member, name: "dop_read_write", scope: !90, file: !10, line: 25, baseType: !102, size: 64, align: 64, offset: 128)
!106 = !DIDerivedType(tag: DW_TAG_member, name: "dop_read", scope: !90, file: !10, line: 26, baseType: !107, size: 64, align: 64, offset: 192)
!107 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !87, size: 64, align: 64)
!108 = !DIDerivedType(tag: DW_TAG_member, name: "devop_init_registered", scope: !90, file: !10, line: 28, baseType: !7, size: 32, align: 32, offset: 256)
!109 = !DIDerivedType(tag: DW_TAG_member, name: "device_info", scope: !90, file: !10, line: 29, baseType: !21, size: 64, align: 64, offset: 320)
!110 = !{!111}
!111 = !DILocalVariable(name: "devop", arg: 1, scope: !86, file: !69, line: 42, type: !89)
!112 = distinct !DISubprogram(name: "reg_devops", scope: !69, file: !69, line: 59, type: !113, isLocal: false, isDefinition: true, scopeLine: 59, flags: DIFlagPrototyped, isOptimized: true, variables: !115)
!113 = !DISubroutineType(types: !114)
!114 = !{!7, !102, !89}
!115 = !{!116, !117, !118}
!116 = !DILocalVariable(name: "fp", arg: 1, scope: !112, file: !69, line: 59, type: !102)
!117 = !DILocalVariable(name: "devops", arg: 2, scope: !112, file: !69, line: 59, type: !89)
!118 = !DILocalVariable(name: "new_fp", scope: !112, file: !69, line: 91, type: !107)
!119 = !{!120}
!120 = !DIGlobalVariable(name: "dm", scope: !68, file: !69, line: 15, type: !121, isLocal: false, isDefinition: true, variable: %struct.device_manager* @dm)
!121 = !DIDerivedType(tag: DW_TAG_typedef, name: "device_manager", file: !69, line: 13, baseType: !122)
!122 = !DICompositeType(tag: DW_TAG_structure_type, name: "device_manager", file: !69, line: 9, size: 704, align: 64, elements: !123)
!123 = !{!124, !126, !127}
!124 = !DIDerivedType(tag: DW_TAG_member, name: "log", scope: !122, file: !69, line: 10, baseType: !125, size: 640, align: 64)
!125 = !DICompositeType(tag: DW_TAG_array_type, baseType: !21, size: 640, align: 64, elements: !44)
!126 = !DIDerivedType(tag: DW_TAG_member, name: "dev_req_count", scope: !122, file: !69, line: 11, baseType: !7, size: 32, align: 32, offset: 640)
!127 = !DIDerivedType(tag: DW_TAG_member, name: "dev_reg_count", scope: !122, file: !69, line: 12, baseType: !7, size: 32, align: 32, offset: 672)
!128 = !{!"clang version 3.8.1 (https://github.com/llvm-mirror/clang.git 07a6361e0f32f699d47c124106e7911b584974d4) (https://github.com/llvm-mirror/llvm.git 051e787f26dbfdc26cf61a57bc82ca00dcb812e8)"}
!129 = !{i32 2, !"Dwarf Version", i32 4}
!130 = !{i32 2, !"Debug Info Version", i32 3}
!131 = !DIExpression()
!132 = !DILocation(line: 44, column: 36, scope: !4)
!133 = !DILocation(line: 46, column: 11, scope: !4)
!134 = !DILocation(line: 48, column: 15, scope: !135)
!135 = distinct !DILexicalBlock(scope: !4, file: !1, line: 48, column: 7)
!136 = !{!137, !141, i64 32}
!137 = !{!"device_operations", !138, i64 0, !138, i64 8, !138, i64 16, !138, i64 24, !141, i64 32, !138, i64 40}
!138 = !{!"any pointer", !139, i64 0}
!139 = !{!"omnipotent char", !140, i64 0}
!140 = !{!"Simple C/C++ TBAA"}
!141 = !{!"int", !139, i64 0}
!142 = !DILocation(line: 48, column: 36, scope: !135)
!143 = !DILocation(line: 48, column: 7, scope: !4)
!144 = !DILocation(line: 49, column: 24, scope: !145)
!145 = distinct !DILexicalBlock(scope: !135, file: !1, line: 48, column: 39)
!146 = !DILocation(line: 60, column: 2, scope: !4)
!147 = !DILocation(line: 68, column: 3, scope: !34)
!148 = !DILocation(line: 74, column: 20, scope: !34)
!149 = !DILocation(line: 74, column: 31, scope: !34)
!150 = !{!151, !138, i64 24}
!151 = !{!"device", !138, i64 0, !139, i64 8, !138, i64 24, !152, i64 32, !152, i64 36}
!152 = !{!"float", !139, i64 0}
!153 = !DIExpression(DW_OP_deref)
!154 = !DILocation(line: 68, column: 17, scope: !34)
!155 = !DILocation(line: 79, column: 2, scope: !34)
!156 = !DILocation(line: 108, column: 2, scope: !34)
!157 = !DILocation(line: 114, column: 1, scope: !34)
!158 = !DILocation(line: 37, column: 56, scope: !60)
!159 = !DILocation(line: 39, column: 14, scope: !160)
!160 = distinct !DILexicalBlock(scope: !60, file: !1, line: 39, column: 6)
!161 = !DILocation(line: 39, column: 35, scope: !160)
!162 = !DILocation(line: 39, column: 6, scope: !60)
!163 = !DILocation(line: 40, column: 12, scope: !164)
!164 = distinct !DILexicalBlock(scope: !160, file: !1, line: 39, column: 38)
!165 = !DILocation(line: 40, column: 24, scope: !164)
!166 = !{!137, !138, i64 40}
!167 = !DILocation(line: 41, column: 2, scope: !164)
!168 = !DILocation(line: 42, column: 1, scope: !60)
!169 = !DILocation(line: 8, column: 38, scope: !54)
!170 = !DILocation(line: 9, column: 12, scope: !54)
!171 = !DILocation(line: 9, column: 17, scope: !54)
!172 = !{!173, !141, i64 0}
!173 = !{!"packet", !141, i64 0, !138, i64 8}
!174 = !DILocation(line: 10, column: 12, scope: !54)
!175 = !DILocation(line: 10, column: 17, scope: !54)
!176 = !{!173, !138, i64 8}
!177 = !DILocation(line: 11, column: 2, scope: !54)
!178 = !DILocation(line: 15, column: 52, scope: !57)
!179 = !DILocation(line: 16, column: 10, scope: !57)
!180 = !DILocation(line: 16, column: 32, scope: !57)
!181 = !DILocation(line: 17, column: 1, scope: !57)
!182 = !DILocation(line: 20, column: 47, scope: !63)
!183 = !DILocation(line: 21, column: 14, scope: !184)
!184 = distinct !DILexicalBlock(scope: !63, file: !1, line: 21, column: 6)
!185 = !DILocation(line: 21, column: 35, scope: !184)
!186 = !DILocation(line: 22, column: 3, scope: !187)
!187 = distinct !DILexicalBlock(scope: !184, file: !1, line: 21, column: 38)
!188 = !DILocation(line: 27, column: 1, scope: !63)
!189 = !DILocation(line: 22, column: 33, scope: !71)
!190 = !DILocation(line: 22, column: 44, scope: !71)
!191 = !DILocation(line: 25, column: 7, scope: !71)
!192 = !DILocation(line: 25, column: 12, scope: !71)
!193 = !{!151, !138, i64 0}
!194 = !DILocation(line: 28, column: 12, scope: !71)
!195 = !{!196, !141, i64 80}
!196 = !{!"device_manager", !139, i64 0, !141, i64 80, !141, i64 84}
!197 = !DILocation(line: 28, column: 2, scope: !71)
!198 = !DILocation(line: 28, column: 27, scope: !71)
!199 = !{!138, !138, i64 0}
!200 = !DILocation(line: 40, column: 1, scope: !71)
!201 = !DILocation(line: 42, column: 38, scope: !86)
!202 = !DILocation(line: 43, column: 11, scope: !86)
!203 = !DILocation(line: 43, column: 23, scope: !86)
!204 = !DILocation(line: 44, column: 4, scope: !86)
!205 = !DILocation(line: 59, column: 22, scope: !112)
!206 = !DILocation(line: 59, column: 80, scope: !112)
!207 = !DILocation(line: 91, column: 8, scope: !112)
!208 = !DILocation(line: 107, column: 2, scope: !112)
!209 = !DILocation(line: 110, column: 2, scope: !112)
