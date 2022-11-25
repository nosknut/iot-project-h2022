# Configuration

Create the file ```src/aws-config.h``` with the following content:
```
#ifndef aws_config_h
#define aws_config_h

const char *AWS_IOT_ENDPOINT = "";
const char *DEVICE_ID = "";

// Amazon's root CA. This should be the same for everyone.
const char *AWS_CERT_CA = "";

// The private key for your device
const char *AWS_CERT_PRIVATE = "";

// The certificate for your device
const char *AWS_CERT_CRT = "";

#endif
```