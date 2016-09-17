/*
 * nanojpegHelper.h
 *
 *  Created on: Sep 14, 2016
 *      Author: bennyboy
 */

#ifndef LIB_JPEG_NANOJPEGHELPER_H_
#define LIB_JPEG_NANOJPEGHELPER_H_

// nj_result_t: Result codes for njDecode().
typedef enum _nj_result {
    NJ_OK = 0,        // no error, decoding successful
    NJ_NO_JPEG,       // not a JPEG file
    NJ_UNSUPPORTED,   // unsupported format
    NJ_OUT_OF_MEM,    // out of memory
    NJ_INTERNAL_ERR,  // internal error
    NJ_SYNTAX_ERROR,  // syntax error
    __NJ_FINISHED,    // used internally, will never be reported
} nj_result_t;

extern void njInit(void);
extern nj_result_t njDecode(const void* jpeg, const int size);
extern int njGetHeight(void);
extern int njGetDepth(void);
extern int njIsColor(void);
extern unsigned char* njGetImage(void);
extern int njGetImageSize(void);
extern void njDone(void);
int njGetWidth(void);

#endif /* LIB_JPEG_NANOJPEGHELPER_H_ */
