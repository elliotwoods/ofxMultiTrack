//@author: vux
//@help: standard constant shader
//@tags: color
//@credits: 

Texture2D RGBTexture <string uiname="RGB";>;
Texture2D<float> DepthTexture <string uiname="Depth";>;

SamplerState g_samLinear <string uiname="Sampler State";>
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

 
cbuffer cbPerDraw : register( b0 )
{
	float4x4 tVP : VIEWPROJECTION;
};


cbuffer cbPerObj : register( b1 )
{
	float4x4 tW : WORLD;
};

struct VS_IN
{
	float4 PosO : POSITION;
	float4 TexCd : TEXCOORD0;

};

struct vs2ps
{
    float4 PosWVP: SV_POSITION;
    float4 TexCd: TEXCOORD0;
};

vs2ps VS(VS_IN input)
{
    vs2ps Out = (vs2ps)0;
    
	float depth = DepthTexture.Load(int3(input.TexCd.xy * float2(512, 424),0)) * float((1 << 16) - 1) / 1000.0f;
	
	input.PosO.z = depth;
	
	Out.PosWVP  = mul(input.PosO,mul(tW,tVP));
    Out.TexCd = input.TexCd;
	
	if(depth == 0) {
		Out.PosWVP.w = 0;
	}
	
    return Out;
}




float4 PS(vs2ps In): SV_Target
{
    float4 col = RGBTexture.Sample(g_samLinear,In.TexCd.xy);
    return col;
}





technique10 Constant
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
	}
}




