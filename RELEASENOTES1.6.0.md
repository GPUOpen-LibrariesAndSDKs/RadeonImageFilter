# release note 1.6.0.
Features :
* remove deprecated API
* new API rifGetDeviceInfo
* add a decay parameter for bloom
* Tracing improvement for debug
* border processing (repeat, mirror, clamp) for gaussian based algorithm
* update to RML 0.9.8
* backward/forward compatibility added (major and minor version need to match to have compatibility)

Bugs :
* RML 0.9.8 workaround limitation on nvidia for OpenCL sub buffer bigger than 256MB
* check if a device supports DX12 features
* fixing hexadecimal output for logging and tracing

