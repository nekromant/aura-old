#ifndef __DEBUG_H
#define __DEBUG_H

#if DEBUG == 1

#ifndef COMPONENT
#define COMPONENT "aura"
#endif
#define DBG(fmt, ...) printf(COMPONENT ": " fmt "\n", ##__VA_ARGS__)

#else
#define DBG(fmt, ...) ;;
#endif

#endif
