        start:  not r0 r1       ; negate the value of r1 to r0
                and r0 r0 r1    ; and r1 and r0 result in 0 for r0
                not r1 r0       ; negate the value in r0 to r1
                add r1 r1 r1    ; r1 contains 254
                not r1 r1       ; negating 254 result in 1
                and r3 r0 r0    ; r3 = 0
                and r3 r1 r1    ; r3 = 1
        loop:   add r3 r0 r1    ; r3 = r0 + r
                and r0 r1 r1    ; r0 = r1
                and r1 r3 r3    ; r1 = r3
                bnz loop        ; never stop because the zflag is never set