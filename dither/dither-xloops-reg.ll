; ModuleID = '../dither/dither-xloops-reg.cc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-n32"
target triple = "pisael-brg"

define void @_ZN6dither17dither_xloops_regEPhS0_ii(i8* nocapture %dest, i8* nocapture %src, i32 %nrows, i32 %ncols) {
entry:
  %add = add i32 %ncols, 2
  %vla = alloca i32, i32 %add, align 4
  %vla2 = alloca i32, i32 %add, align 4
  %cmp78 = icmp sgt i32 %add, 0
  br i1 %cmp78, label %for.body.lr.ph, label %for.cond7.preheader

for.body.lr.ph:                                   ; preds = %entry
  %vla281 = bitcast i32* %vla2 to i8*
  %vla80 = bitcast i32* %vla to i8*
  %0 = shl i32 %ncols, 2
  %1 = add i32 %0, 8
  call void @llvm.memset.p0i8.i32(i8* %vla80, i8 0, i32 %1, i32 4, i1 false)
  call void @llvm.memset.p0i8.i32(i8* %vla281, i8 0, i32 %1, i32 4, i1 false)
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.body.lr.ph, %entry
  %cmp873 = icmp sgt i32 %nrows, 0
  %cmp1170 = icmp sgt i32 %ncols, 0
  %or.cond = and i1 %cmp873, %cmp1170
  br i1 %or.cond, label %for.body12.lr.ph.us, label %for.end43

for.end40.us:                                     ; preds = %if.end34.us
  %inc42.us = add nsw i32 %row_idx.076.us, 1
  %exitcond82 = icmp eq i32 %inc42.us, %nrows
  br i1 %exitcond82, label %for.end43, label %for.body12.lr.ph.us

for.body12.us:                                    ; preds = %if.end34.us, %for.body12.lr.ph.us
  %error.172.us = phi i32 [ %error.077.us, %for.body12.lr.ph.us ], [ %error.2.us, %if.end34.us ]
  %col_idx.071.us = phi i32 [ 0, %for.body12.lr.ph.us ], [ %col_idx.bump, %if.end34.us ]
  %add13.us = add nsw i32 %col_idx.071.us, %mul.us
  %arrayidx14.us = getelementptr inbounds i8* %src, i32 %add13.us
  %2 = load i8* %arrayidx14.us, align 1, !tbaa !0
  %cmp15.us = icmp eq i8 %2, 0
  br i1 %cmp15.us, label %for.body12.us.if.end34.us_crit_edge, label %if.then.us

for.body12.us.if.end34.us_crit_edge:              ; preds = %for.body12.us
  br label %if.end34.us

if.then.us:                                       ; preds = %for.body12.us
  %conv.us = zext i8 %2 to i32
  %add16.us = add nsw i32 %col_idx.071.us, 2
  %arrayidx17.us = getelementptr inbounds i32* %vla6974.us, i32 %add16.us
  %3 = load i32* %arrayidx17.us, align 4, !tbaa !4
  %add19.us = add nsw i32 %col_idx.071.us, 1
  %arrayidx20.us = getelementptr inbounds i32* %vla6974.us, i32 %add19.us
  %4 = load i32* %arrayidx20.us, align 4, !tbaa !4
  %mul21.us = mul nsw i32 %4, 5
  %arrayidx23.us = getelementptr inbounds i32* %vla6974.us, i32 %col_idx.071.us
  %5 = load i32* %arrayidx23.us, align 4, !tbaa !4
  %mul24.us = mul nsw i32 %5, 3
  %mul26.us = mul nsw i32 %error.172.us, 7
  %add22.us = add i32 %3, %mul26.us
  %add25.us = add i32 %add22.us, %mul21.us
  %add27.us = add i32 %add25.us, %mul24.us
  %div.us = sdiv i32 %add27.us, 16
  %add30.us = add nsw i32 %div.us, %conv.us
  %cmp31.us = icmp sgt i32 %add30.us, 127
  %..us = sext i1 %cmp31.us to i8
  %conv33.us = zext i8 %..us to i32
  %sub.us = sub nsw i32 %add30.us, %conv33.us
  br label %if.end34.us

if.end34.us:                                      ; preds = %for.body12.us.if.end34.us_crit_edge, %if.then.us
  %dest_pixel.1.us = phi i8 [ 0, %for.body12.us.if.end34.us_crit_edge ], [ %..us, %if.then.us ]
  %error.2.us = phi i32 [ 0, %for.body12.us.if.end34.us_crit_edge ], [ %sub.us, %if.then.us ]
  ; shreesha - merging the iv bump instruction
  %col_idx.bump = add nsw i32 %col_idx.071.us, 1, !dbg !2
  %arrayidx36.us = getelementptr inbounds i32* %vla26875.us, i32 %col_idx.bump
  store i32 %error.2.us, i32* %arrayidx36.us, align 4, !tbaa !4
  %arrayidx37.us = getelementptr inbounds i8* %dest, i32 %add13.us
  store i8 %dest_pixel.1.us, i8* %arrayidx37.us, align 1, !tbaa !0
  %exitcond = icmp eq i32 %col_idx.bump, %ncols
  br i1 %exitcond, label %for.end40.us, label %for.body12.us

for.body12.lr.ph.us:                              ; preds = %for.cond7.preheader, %for.end40.us
  %error.077.us = phi i32 [ %error.2.us, %for.end40.us ], [ 0, %for.cond7.preheader ]
  %row_idx.076.us = phi i32 [ %inc42.us, %for.end40.us ], [ 0, %for.cond7.preheader ]
  %vla26875.us = phi i32* [ %vla6974.us, %for.end40.us ], [ %vla2, %for.cond7.preheader ]
  %vla6974.us = phi i32* [ %vla26875.us, %for.end40.us ], [ %vla, %for.cond7.preheader ]
  %mul.us = mul nsw i32 %row_idx.076.us, %ncols
  br label %for.body12.us

for.end43:                                        ; preds = %for.end40.us, %for.cond7.preheader
  ret void
}

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind

!0 = metadata !{metadata !"omnipotent char", metadata !1}
!1 = metadata !{metadata !"Simple C/C++ TBAA"}
!2 = metadata !{i32 0, i32 0, metadata !3, null}
!3 = metadata !{metadata !"forr"}
!4 = metadata !{metadata !"int", metadata !0}
!5 = metadata !{i32 1838}
