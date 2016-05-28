; ModuleID = '../dither/dither-xloops-mem.cc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-n32"
target triple = "pisael-brg"

define void @_ZN6dither17dither_xloops_memEPhS0_ii(i8* nocapture %dest, i8* nocapture %src, i32 %nrows, i32 %ncols) {
entry:
  %add = add nsw i32 %nrows, 1
  %add1 = add i32 %ncols, 2
  %0 = mul nuw i32 %add1, %add
  %vla = alloca i32, i32 %0, align 4
  %vla92 = bitcast i32* %vla to i8*
  %cmp88 = icmp sgt i32 %nrows, 0
  br i1 %cmp88, label %for.cond2.preheader.lr.ph, label %for.end51

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %cmp486 = icmp sgt i32 %add1, 0
  br i1 %cmp486, label %for.cond2.preheader.lr.ph.split.us, label %for.cond13.preheader.lr.ph

for.cond2.preheader.lr.ph.split.us:               ; preds = %for.cond2.preheader.lr.ph
  %1 = shl i32 %ncols, 2
  %2 = add i32 %1, 8
  br label %for.inc7.us

for.inc7.us:                                      ; preds = %for.inc7.us, %for.cond2.preheader.lr.ph.split.us
  %i.089.us = phi i32 [ 0, %for.cond2.preheader.lr.ph.split.us ], [ %inc8.us, %for.inc7.us ]
  %3 = mul i32 %2, %i.089.us
  %uglygep = getelementptr i8* %vla92, i32 %3
  call void @llvm.memset.p0i8.i32(i8* %uglygep, i8 0, i32 %2, i32 4, i1 false)
  %inc8.us = add nsw i32 %i.089.us, 1
  %exitcond93 = icmp eq i32 %inc8.us, %nrows
  br i1 %exitcond93, label %for.cond10.preheader, label %for.inc7.us

for.cond10.preheader:                             ; preds = %for.inc7.us
  %cmp1480 = icmp sgt i32 %ncols, 0
  %or.cond = and i1 %cmp88, %cmp1480
  br i1 %or.cond, label %for.body15.lr.ph.us, label %for.end51

for.cond13.preheader.lr.ph:                       ; preds = %for.cond2.preheader.lr.ph
  %cmp1480.old = icmp sgt i32 %ncols, 0
  br i1 %cmp1480.old, label %for.body15.lr.ph.us, label %for.end51

for.inc49.us:                                     ; preds = %if.end40.us
  %row_idx.bump = add nsw i32 %row_idx.084.us, 1, !dbg !3
  %exitcond90   = icmp eq i32 %row_idx.bump, %nrows
  br i1 %exitcond90, label %for.end51, label %for.body15.lr.ph.us

for.body15.us:                                    ; preds = %if.end40.us, %for.body15.lr.ph.us
  %error.182.us = phi i32 [ %error.085.us, %for.body15.lr.ph.us ], [ %error.2.us, %if.end40.us ]
  %col_idx.081.us = phi i32 [ 0, %for.body15.lr.ph.us ], [ %add41.us, %if.end40.us ]
  %add16.us = add nsw i32 %col_idx.081.us, %mul.us
  %arrayidx17.us = getelementptr inbounds i8* %src, i32 %add16.us
  %4 = load i8* %arrayidx17.us, align 1, !tbaa !0
  %cmp18.us = icmp eq i8 %4, 0
  br i1 %cmp18.us, label %if.end40.us, label %if.then.us

if.then.us:                                       ; preds = %for.body15.us
  %conv.us = zext i8 %4 to i32
  %arrayidx20.sum.us = add i32 %add19.us, %col_idx.081.us
  %arrayidx21.us = getelementptr inbounds i32* %vla, i32 %arrayidx20.sum.us
  %5 = load i32* %arrayidx21.us, align 4, !tbaa !2
  %arrayidx20.sum78.us = add i32 %add23.us, %col_idx.081.us
  %arrayidx25.us = getelementptr inbounds i32* %vla, i32 %arrayidx20.sum78.us
  %6 = load i32* %arrayidx25.us, align 4, !tbaa !2
  %mul26.us = mul nsw i32 %6, 5
  %arrayidx20.sum79.us = add i32 %col_idx.081.us, %9
  %arrayidx29.us = getelementptr inbounds i32* %vla, i32 %arrayidx20.sum79.us
  %7 = load i32* %arrayidx29.us, align 4, !tbaa !2
  %mul30.us = mul nsw i32 %7, 3
  %mul32.us = mul nsw i32 %error.182.us, 7
  %add27.us = add i32 %5, %mul32.us
  %add31.us = add i32 %add27.us, %mul26.us
  %add33.us = add i32 %add31.us, %mul30.us
  %div.us = sdiv i32 %add33.us, 16
  %add36.us = add nsw i32 %div.us, %conv.us
  %cmp37.us = icmp sgt i32 %add36.us, 127
  %..us = sext i1 %cmp37.us to i8
  %conv39.us = zext i8 %..us to i32
  %sub.us = sub nsw i32 %add36.us, %conv39.us
  br label %if.end40.us

if.end40.us:                                      ; preds = %if.then.us, %for.body15.us
  %error.2.us = phi i32 [ %sub.us, %if.then.us ], [ 0, %for.body15.us ]
  %dest_pixel.1.us = phi i8 [ %..us, %if.then.us ], [ 0, %for.body15.us ]
  %add41.us = add nsw i32 %col_idx.081.us, 1
  %arrayidx43.sum.us = add i32 %add41.us, %8
  %arrayidx44.us = getelementptr inbounds i32* %vla, i32 %arrayidx43.sum.us
  store i32 %error.2.us, i32* %arrayidx44.us, align 4, !tbaa !2
  %arrayidx45.us = getelementptr inbounds i8* %dest, i32 %add16.us
  store i8 %dest_pixel.1.us, i8* %arrayidx45.us, align 1, !tbaa !0
  %exitcond = icmp eq i32 %add41.us, %ncols
  br i1 %exitcond, label %for.inc49.us, label %for.body15.us

for.body15.lr.ph.us:                              ; preds = %for.cond10.preheader, %for.inc49.us, %for.cond13.preheader.lr.ph
  %error.085.us = phi i32 [ %error.2.us, %for.inc49.us ], [ 0, %for.cond13.preheader.lr.ph ], [ 0, %for.cond10.preheader ]
  %row_idx.084.us = phi i32 [ %row_idx.bump, %for.inc49.us ], [ 0, %for.cond13.preheader.lr.ph ], [ 0, %for.cond10.preheader ]
  %mul.us = mul nsw i32 %row_idx.084.us, %ncols
  %add42.us = add nsw i32 %row_idx.084.us, 1
  %8 = mul nsw i32 %add42.us, %add1
  %9 = mul nsw i32 %row_idx.084.us, %add1
  %add19.us = add i32 %9, 2
  %add23.us = add i32 %9, 1
  br label %for.body15.us

for.end51:                                        ; preds = %entry, %for.cond13.preheader.lr.ph, %for.inc49.us, %for.cond10.preheader
  ret void
}

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind

!0 = metadata !{metadata !"omnipotent char", metadata !1}
!1 = metadata !{metadata !"Simple C/C++ TBAA"}
!2 = metadata !{metadata !"int", metadata !0}
!3 = metadata !{i32 0, i32 0, metadata !4, null}
!4 = metadata !{metadata !"form"}
