#ifndef SRC_INCLUDE_BASE64_H_
#define SRC_INCLUDE_BASE64_H_

#if __cplusplus
extern "C"{
#endif
    
    int base64_encode(const char *indata, int inlen, char *outdata, int *outlen);
    int base64_decode(const char *indata, int inlen, char *outdata, int *outlen);
            
#if __cplusplus
}
#endif

#endif /* SRC_INCLUDE_BASE64_H_ */
