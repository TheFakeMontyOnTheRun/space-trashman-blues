#ifndef FIXP_H
#define FIXP_H

typedef int32_t FixP_t;

#define kIntegerPart 16

#define  fixToInt(fp)  ((FixP_t)((fp) >> kIntegerPart))

#define intToFix(v)  ((int32_t)((v) << kIntegerPart))

#define Mul(v1, v2) ((FixP_t)((((v1) >> 6) * ((v2) >> 6)) >> 4))

#define Div(v1, v2)  ((FixP_t)((((int64_t) (v1)) * (1 << kIntegerPart)) / (v2)))

#define fixToFloat(fp) (fixToInt(Mul((fp), intToFix(16))) / 16.0f)

#endif
