\ not very forthlike, dont care

variable c1
variable c2

variable width
variable height

variable iter
variable max-iter

variable real-min
variable real-max

variable imag-min
variable imag-max

variable real-step
variable imag-step

variable zreal
variable zimag

variable creal
variable cimag

variable zreal^2
variable zimag^2

variable temp-real
variable temp-imag

: mandel ( n -- )
    max-iter !

    80 width !
    24 height !

    32 c1 ! \ ' '
    35 c2 ! \ '#'

    -2.0 real-min !
    1.0 real-max !

    -1.0 imag-min !
    1.0 imag-max !

    real-max @ real-min @ f-
    width @ d>f f/
    real-step !

    imag-max @ imag-min @ f-
    height @ d>f f/
    imag-step !

    height @ 0 do
        width @ 0 do
            0.0 zreal !
            0.0 zimag !

            real-step @ i d>f f*
            real-min @ f+
            creal !

            imag-step @ j d>f f*
            imag-min @ f+
            cimag !

            0 iter !
            max-iter @ 0 do
                iter @ 1+ iter !

                zreal @ dup f* zreal^2 !
                zimag @ dup f* zimag^2 !

                zreal^2 @ zimag^2 @ f+

                4.0 
                f> if
                    leave
                then

                zreal^2 @
                zimag^2 @ f-
                creal @ f+
                temp-real !

                2.0
                zreal @ f*
                zimag @ f*
                cimag @ f+
                temp-imag !

                temp-real @ zreal !
                temp-imag @ zimag !
                
            loop
            
            iter @
            max-iter @
            = if 
                c1 @ emit
            else
                c2 @ emit
            then
        loop
        10 emit
    loop
;