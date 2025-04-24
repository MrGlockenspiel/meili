\ approximate pi using the leibniz series
\ n is iteration count

: leibniz ( n -- pi )
    \ sum
    0.0

    swap \ rearrange to 0.0 n 0 do
    0 do
        \ term = pow(-1.0, (double)i) / (2.0 * (double)i + 1.0)
        -1.0 i d>f f**
        2.0 i d>f f* 1.0 f+
        f/

        \ sum += term
        f+
    loop

    \ pi = 4.0 * sum
    4.0 f*
;
