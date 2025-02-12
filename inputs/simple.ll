define ptr @f(ptr %p, i64 %idx) {
    %i1 = ptrtoint ptr %p to i64
    %i2 = add i64 %i1, %idx
    %r = inttoptr i64 %i2 to ptr
    ret ptr %r
}

define ptr @g(ptr %p, i64 %idx1, i64 %idx2) {
    %i1 = ptrtoint ptr %p to i64
    %i2 = add i64 %i1, %idx1
    %p1 = inttoptr i64 %i2 to ptr

    %i3 = ptrtoint ptr %p to i64
    %i4 = add i64 %i3, %idx2
    %p2 = inttoptr i64 %i4 to ptr

    %cmp = icmp sgt ptr %p1, %p2            ; set if p1 > p2
    %max = select i1 %cmp, ptr %p1, ptr %p2 ; cmp ? p1 : p2
    ret ptr %max
}

define ptr @b(ptr %p, i64 %idx) {
    %i1 = ptrtoint ptr %p to i64
    %i2 = add i64 %i1, %idx
    %r = inttoptr i64 %i2 to ptr

    %i3 = add i64 %i2, 1

    ret ptr %r
}