release note 1.5.2
Features :
* improved AI denoisers
* FP16 models for upscaler and denoiser = > for now under variable RIF_AI_FP16_ENABLED=1
* using RML 0.9.6
Bug  Fixes
* Return card name instead of asics name for AMD
* Fix unchartedtonemapper for navi products
* no more link against OpenCL, we do delay load
