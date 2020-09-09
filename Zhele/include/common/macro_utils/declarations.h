#ifndef ZHELE_DECLARATIONS_H
#define ZHELE_DECLARATIONS_H

#if defined (__GNUC__) && defined(__arm__)
#define ZHELE_INTERRUPT(ISR_NAME) __attribute__((interrupt)) void ISR_NAME()
#endif

#endif //!ZHELE_DECLARATIONS_H