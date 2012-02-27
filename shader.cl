__kernel void adder(__read_only image2d_t srcImg,__write_only image2d_t dstImg,sampler_t sampler, int width, int height)
{
float3 gMonoMult = (float3) (0.299, 0.587, 0.114);

float4 f4 = read_imagef(srcImg, sampler, (int2)(get_global_id(0), get_global_id(1)));

float4 Mono = (float4)(0.0, 0.0, 0.0, 0.0);

Mono.xyz = dot(f4.xyz,gMonoMult);

write_imagef(dstImg, (int2)(get_global_id(0), get_global_id(1)), Mono);

}