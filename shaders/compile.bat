set FXC="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe"

if not exist package\Shaders\Loose\VPAO mkdir package\Shaders\Loose\VPAO

%FXC% /T ps_3_0 /E Main /Fo "package\shaders\loose\VPAO\ISLINEARIZEDEPTH.pso" "shaders\LinearizeDepth.hlsl"
%FXC% /T ps_3_0 /E Main /Fo "package\shaders\loose\VPAO\ISDOWNDEPTH.pso" "shaders\DownsampleDepth.hlsl"
%FXC% /T vs_3_0 /E Main /DVS /Fo "package\shaders\loose\VPAO\ISSAO.vso" "shaders\SAO.hlsl"
%FXC% /T ps_3_0 /E Main /DPS /Fo "package\shaders\loose\VPAO\ISSAO.pso" "shaders\SAO.hlsl"
%FXC% /T vs_3_0 /E Main /DVS /Fo "package\shaders\loose\VPAO\ISSAOBlur.vso" "shaders\SAOBlur.hlsl"
%FXC% /T ps_3_0 /E Main /DPS /Fo "package\shaders\loose\VPAO\ISSAOBlur.pso" "shaders\SAOBlur.hlsl"
%FXC% /T vs_3_0 /E Main /DVS /Fo "package\shaders\loose\VPAO\ISSAOApply.vso" "shaders\SAOApply.hlsl"
%FXC% /T ps_3_0 /E Main /DPS /Fo "package\shaders\loose\VPAO\ISSAOApply.pso" "shaders\SAOApply.hlsl"
