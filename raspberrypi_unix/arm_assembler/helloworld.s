    .data                       // 데이터 섹션
string: .asciz "Hello World\n"

    .text                       // 코드 섹션
    .global main                // 메인 함수
    .extern printf              // 외부의 printf 함수 

main:
    stp x29, x30, [sp, -16]!
    ldr x0, =string             // 데이터 섹션에 있는 문자열(상수)의 주소 x0으로 로드
    bl  printf
    ldp x29, x30, [sp], 16
    ret
